#!/usr/bin/env python3
"""
Advanced EDA Circuit Layout Example

This example simulates a more realistic electronic circuit layout with:
- Various component types (chips, resistors, capacitors, traces)
- Different geometric constraints
- Comprehensive analysis for manufacturing feasibility
"""

import sys
import os
import math

# Add parent directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

import zlayout
import matplotlib.pyplot as plt


class CircuitComponent:
    """Represents a circuit component with properties."""
    
    def __init__(self, name: str, geometry, component_type: str = "generic"):
        self.name = name
        self.geometry = geometry
        self.component_type = component_type
        self.pins = []  # List of pin locations
    
    def add_pin(self, point: zlayout.Point, pin_name: str = ""):
        """Add a pin to the component."""
        self.pins.append((point, pin_name))


def create_microcontroller(x: float, y: float) -> CircuitComponent:
    """Create a microcontroller component."""
    # Main body (rectangle)
    body = zlayout.Rectangle(x, y, 15, 15)
    
    # Convert to polygon for more complex analysis
    geometry = body.to_polygon()
    
    mcu = CircuitComponent("MCU_1", geometry, "microcontroller")
    
    # Add pins around the perimeter
    pin_spacing = 1.0
    # Bottom pins
    for i in range(8):
        pin_x = x + 2 + i * pin_spacing
        mcu.add_pin(zlayout.Point(pin_x, y), f"P{i}")
    
    # Top pins  
    for i in range(8):
        pin_x = x + 2 + i * pin_spacing
        mcu.add_pin(zlayout.Point(pin_x, y + 15), f"P{i+8}")
    
    return mcu


def create_resistor(x: float, y: float, rotation: float = 0) -> CircuitComponent:
    """Create a resistor component."""
    # Basic resistor shape
    length, width = 6, 2
    
    # Create resistor body
    vertices = [
        zlayout.Point(-length/2, -width/2),
        zlayout.Point(length/2, -width/2),
        zlayout.Point(length/2, width/2),
        zlayout.Point(-length/2, width/2)
    ]
    
    # Apply rotation and translation
    cos_r, sin_r = math.cos(rotation), math.sin(rotation)
    rotated_vertices = []
    for v in vertices:
        new_x = v.x * cos_r - v.y * sin_r + x
        new_y = v.x * sin_r + v.y * cos_r + y
        rotated_vertices.append(zlayout.Point(new_x, new_y))
    
    geometry = zlayout.Polygon(rotated_vertices)
    resistor = CircuitComponent("R1", geometry, "resistor")
    
    # Add connection points
    resistor.add_pin(zlayout.Point(x - length/2, y), "1")
    resistor.add_pin(zlayout.Point(x + length/2, y), "2")
    
    return resistor


def create_capacitor(x: float, y: float) -> CircuitComponent:
    """Create a capacitor component."""
    # Capacitor body (smaller rectangle)
    geometry = zlayout.Rectangle(x-2, y-1.5, 4, 3).to_polygon()
    
    cap = CircuitComponent("C1", geometry, "capacitor")
    cap.add_pin(zlayout.Point(x-2, y), "1")
    cap.add_pin(zlayout.Point(x+2, y), "2")
    
    return cap


def create_trace(start: zlayout.Point, end: zlayout.Point, width: float = 0.3) -> CircuitComponent:
    """Create a PCB trace (connection line with width)."""
    # Calculate perpendicular vector for width
    dx = end.x - start.x
    dy = end.y - start.y
    length = math.sqrt(dx*dx + dy*dy)
    
    if length < 1e-10:
        # Degenerate trace
        geometry = zlayout.Rectangle(start.x, start.y, 0.1, 0.1).to_polygon()
    else:
        # Normalized perpendicular vector
        perp_x = -dy * width / (2 * length)
        perp_y = dx * width / (2 * length)
        
        # Create trace as a polygon
        vertices = [
            zlayout.Point(start.x + perp_x, start.y + perp_y),
            zlayout.Point(end.x + perp_x, end.y + perp_y),
            zlayout.Point(end.x - perp_x, end.y - perp_y),
            zlayout.Point(start.x - perp_x, start.y - perp_y)
        ]
        geometry = zlayout.Polygon(vertices)
    
    return CircuitComponent("Trace", geometry, "trace")


def create_complex_circuit_layout():
    """Create a complex circuit layout with multiple components."""
    
    # Define larger world bounds for complex circuit
    world_bounds = zlayout.Rectangle(0, 0, 150, 100)
    processor = zlayout.GeometryProcessor(world_bounds)
    
    components = []
    
    # Add microcontroller
    mcu = create_microcontroller(30, 40)
    components.append(mcu)
    processor.add_component(mcu.geometry)
    
    # Add resistors in various orientations
    resistors = [
        create_resistor(60, 50, 0),           # Horizontal
        create_resistor(75, 35, math.pi/2),   # Vertical  
        create_resistor(90, 55, math.pi/4),   # 45 degrees
        create_resistor(20, 25, 0),           # Near MCU
    ]
    
    for resistor in resistors:
        components.append(resistor)
        processor.add_component(resistor.geometry)
    
    # Add capacitors
    capacitors = [
        create_capacitor(50, 25),
        create_capacitor(70, 65),
        create_capacitor(100, 40),
    ]
    
    for cap in capacitors:
        components.append(cap)
        processor.add_component(cap.geometry)
    
    # Add traces connecting components
    traces = [
        # Connect MCU to resistors
        create_trace(zlayout.Point(45, 47), zlayout.Point(54, 50), 0.2),
        create_trace(zlayout.Point(37, 55), zlayout.Point(37, 65), 0.2),
        
        # Connect resistors to capacitors  
        create_trace(zlayout.Point(66, 50), zlayout.Point(68, 65), 0.2),
        create_trace(zlayout.Point(75, 45), zlayout.Point(98, 40), 0.2),
        
        # Some traces that might cause narrow spacing issues
        create_trace(zlayout.Point(85, 50), zlayout.Point(85, 60), 0.15),
        create_trace(zlayout.Point(87, 50), zlayout.Point(87, 60), 0.15),
    ]
    
    for trace in traces:
        components.append(trace)
        processor.add_component(trace.geometry)
    
    # Add some problematic geometries to test analysis
    
    # Sharp angled component (connector)
    sharp_connector = zlayout.Polygon([
        zlayout.Point(110, 20),
        zlayout.Point(125, 22),
        zlayout.Point(112, 28),  # This creates a sharp angle
        zlayout.Point(108, 24)
    ])
    components.append(CircuitComponent("Connector", sharp_connector, "connector"))
    processor.add_component(sharp_connector)
    
    # Potentially intersecting components
    overlap_poly1 = zlayout.Polygon([
        zlayout.Point(15, 70),
        zlayout.Point(25, 72),
        zlayout.Point(23, 82),
        zlayout.Point(13, 80)
    ])
    
    overlap_poly2 = zlayout.Polygon([
        zlayout.Point(20, 75),
        zlayout.Point(30, 77),
        zlayout.Point(28, 87),
        zlayout.Point(18, 85)
    ])
    
    components.extend([
        CircuitComponent("Test1", overlap_poly1, "test"),
        CircuitComponent("Test2", overlap_poly2, "test")
    ])
    processor.add_component(overlap_poly1)
    processor.add_component(overlap_poly2)
    
    return processor, components


def analyze_manufacturing_constraints(processor):
    """Analyze the layout for manufacturing constraints."""
    
    print("=== Manufacturing Constraint Analysis ===\n")
    
    # Different constraint levels for different processes
    constraints = {
        "prototype": {"min_spacing": 0.1, "min_trace_width": 0.1, "sharp_angle_limit": 20},
        "standard": {"min_spacing": 0.15, "min_trace_width": 0.15, "sharp_angle_limit": 30},
        "high_density": {"min_spacing": 0.05, "min_trace_width": 0.05, "sharp_angle_limit": 15}
    }
    
    for process_name, limits in constraints.items():
        print(f"--- {process_name.upper()} PROCESS ---")
        
        analysis = processor.analyze_layout(
            sharp_angle_threshold=limits["sharp_angle_limit"],
            narrow_distance_threshold=limits["min_spacing"]
        )
        
        # Check manufacturability
        violations = 0
        
        if analysis['sharp_angles']['count'] > 0:
            violations += analysis['sharp_angles']['count']
            print(f"  ⚠️  Sharp angles: {analysis['sharp_angles']['count']} violations")
        
        if analysis['narrow_distances']['count'] > 0:
            violations += analysis['narrow_distances']['count'] 
            print(f"  ⚠️  Spacing violations: {analysis['narrow_distances']['count']}")
            print(f"     Minimum distance: {analysis['narrow_distances']['minimum']:.3f}")
        
        if analysis['intersections']['polygon_pairs'] > 0:
            violations += analysis['intersections']['polygon_pairs']
            print(f"  ❌ Intersections: {analysis['intersections']['polygon_pairs']} pairs")
        
        if violations == 0:
            print(f"  ✅ Manufacturable with {process_name} process")
        else:
            print(f"  ❌ {violations} total violations for {process_name} process")
        
        print()
    
    return processor.optimize_layout()


def demonstrate_design_rule_checking():
    """Demonstrate design rule checking capabilities."""
    
    print("=== Design Rule Checking (DRC) ===\n")
    
    # Create processor with sample layout
    processor, components = create_complex_circuit_layout()
    
    # Run manufacturing analysis
    optimization_results = analyze_manufacturing_constraints(processor)
    
    # Component-specific analysis
    print("--- COMPONENT ANALYSIS ---")
    
    component_types = {}
    for comp in components:
        comp_type = comp.component_type
        if comp_type not in component_types:
            component_types[comp_type] = 0
        component_types[comp_type] += 1
    
    print("Component count by type:")
    for comp_type, count in component_types.items():
        print(f"  {comp_type}: {count}")
    
    print(f"\nTotal components: {len(components)}")
    
    # Generate comprehensive report
    print("\n--- DRC SUMMARY REPORT ---")
    score = optimization_results['optimization_score']
    
    if score >= 90:
        status = "EXCELLENT"
        emoji = "🟢"
    elif score >= 70:
        status = "GOOD"  
        emoji = "🟡"
    elif score >= 50:
        status = "ACCEPTABLE"
        emoji = "🟠"
    else:
        status = "NEEDS WORK"
        emoji = "🔴"
    
    print(f"Overall Layout Quality: {emoji} {status} ({score:.1f}/100)")
    
    print("\nRecommendations:")
    for suggestion in optimization_results['suggestions']:
        print(f"  • {suggestion}")
    
    if not optimization_results['suggestions']:
        print("  ✅ No issues found! Layout meets all design rules.")
    
    return processor, components, optimization_results


def main():
    """Main function demonstrating advanced EDA layout analysis."""
    
    print("🔧 ZLayout - Advanced EDA Circuit Analysis")
    print("=" * 60)
    
    # Run design rule checking
    processor, components, optimization_results = demonstrate_design_rule_checking()
    
    # Generate visualizations
    print("\n=== Visualization Generation ===")
    
    try:
        visualizer = zlayout.LayoutVisualizer(figsize=(16, 12))
        
        # Extract just the geometries for visualization
        polygons = [comp.geometry for comp in components if isinstance(comp.geometry, zlayout.Polygon)]
        
        print("Creating circuit layout visualization...")
        fig1 = visualizer.plot_layout(polygons, title="Complex Circuit Layout")
        
        print("Creating DRC analysis visualization...")
        fig2 = visualizer.plot_analysis_results(polygons, optimization_results['analysis'])
        
        print("Creating optimization summary...")
        fig3 = visualizer.plot_optimization_summary(optimization_results)
        
        plt.show()
        
        # Optionally save the results
        save_option = input("\nSave analysis results and plots? (y/n): ").lower().strip()
        if save_option == 'y':
            # Save plots
            visualizer.save_plots([fig1, fig2, fig3], "circuit_analysis")
            
            # Save analysis to text file
            with open("circuit_analysis_report.txt", "w") as f:
                f.write("ZLayout Circuit Analysis Report\n")
                f.write("=" * 40 + "\n\n")
                f.write(f"Layout Quality Score: {optimization_results['optimization_score']:.1f}/100\n\n")
                
                analysis = optimization_results['analysis']
                f.write(f"Sharp Angles: {analysis['sharp_angles']['count']}\n")
                f.write(f"Narrow Distances: {analysis['narrow_distances']['count']}\n") 
                f.write(f"Intersections: {analysis['intersections']['polygon_pairs']}\n\n")
                
                f.write("Recommendations:\n")
                for suggestion in optimization_results['suggestions']:
                    f.write(f"  • {suggestion}\n")
            
            print("Results saved to files!")
    
    except ImportError:
        print("Matplotlib not available. Install with: pip install matplotlib")
    
    print("\n✅ Advanced EDA analysis completed!")


if __name__ == "__main__":
    main()