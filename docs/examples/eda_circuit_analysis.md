# EDA Circuit Analysis Example

This example demonstrates advanced EDA circuit layout analysis using ZLayout. We'll create a complex circuit layout with various components and analyze it for manufacturing feasibility.

## Overview

Electronic Design Automation (EDA) tools must verify that circuit layouts meet manufacturing constraints. This example shows how to use ZLayout to:

1. **Create complex circuit layouts** with multiple component types
2. **Analyze manufacturing constraints** like spacing, angles, and intersections
3. **Visualize analysis results** for design verification
4. **Generate comprehensive reports** for manufacturing teams

## Complete Example Code

```python
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
    """Create a microcontroller component with realistic dimensions."""
    # Main body (QFP-64 package: 14mm x 14mm)
    body = zlayout.Rectangle(x, y, 14, 14)
    
    # Convert to polygon for more complex analysis
    geometry = body.to_polygon()
    
    mcu = CircuitComponent("MCU_STM32F4", geometry, "microcontroller")
    
    # Add pins around the perimeter (0.65mm pitch)
    pin_spacing = 0.65
    pin_count_per_side = 16
    
    # Bottom pins (pins 1-16)
    for i in range(pin_count_per_side):
        pin_x = x + 1.5 + i * pin_spacing
        mcu.add_pin(zlayout.Point(pin_x, y), f"P{i+1}")
    
    # Right pins (pins 17-32)
    for i in range(pin_count_per_side):
        pin_y = y + 1.5 + i * pin_spacing
        mcu.add_pin(zlayout.Point(x + 14, pin_y), f"P{i+17}")
    
    # Top pins (pins 33-48)
    for i in range(pin_count_per_side):
        pin_x = x + 12.5 - i * pin_spacing
        mcu.add_pin(zlayout.Point(pin_x, y + 14), f"P{i+33}")
    
    # Left pins (pins 49-64)
    for i in range(pin_count_per_side):
        pin_y = y + 12.5 - i * pin_spacing
        mcu.add_pin(zlayout.Point(x, pin_y), f"P{i+49}")
    
    return mcu


def create_resistor(x: float, y: float, rotation: float = 0, 
                   value: str = "1k", package: str = "0805") -> CircuitComponent:
    """Create a resistor component with specified package size."""
    
    # Package dimensions (mm)
    packages = {
        "0402": (1.0, 0.5),
        "0603": (1.6, 0.8),
        "0805": (2.0, 1.25),
        "1206": (3.2, 1.6)
    }
    
    length, width = packages.get(package, (2.0, 1.25))
    
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
    resistor = CircuitComponent(f"R_{value}_{package}", geometry, "resistor")
    
    # Add connection pads
    pad_offset = length/2 * 0.8
    resistor.add_pin(zlayout.Point(x - pad_offset, y), "1")
    resistor.add_pin(zlayout.Point(x + pad_offset, y), "2")
    
    return resistor


def create_capacitor(x: float, y: float, value: str = "10uF", 
                    package: str = "0805") -> CircuitComponent:
    """Create a capacitor component."""
    
    # Package dimensions
    packages = {
        "0402": (1.0, 0.5),
        "0603": (1.6, 0.8),
        "0805": (2.0, 1.25),
        "1206": (3.2, 1.6),
        "1210": (3.2, 2.5)  # Larger for higher capacity
    }
    
    length, width = packages.get(package, (2.0, 1.25))
    
    # Capacitor body
    geometry = zlayout.Rectangle(x-length/2, y-width/2, length, width).to_polygon()
    
    cap = CircuitComponent(f"C_{value}_{package}", geometry, "capacitor")
    cap.add_pin(zlayout.Point(x-length/2 + 0.2, y), "1")
    cap.add_pin(zlayout.Point(x+length/2 - 0.2, y), "2")
    
    return cap


def create_trace(start: zlayout.Point, end: zlayout.Point, 
                width: float = 0.2, layer: str = "top") -> CircuitComponent:
    """Create a PCB trace (connection line with width)."""
    
    # Calculate trace geometry
    dx = end.x - start.x
    dy = end.y - start.y
    length = math.sqrt(dx*dx + dy*dy)
    
    if length < 1e-10:
        # Degenerate trace - create small rectangle
        geometry = zlayout.Rectangle(start.x, start.y, 0.1, 0.1).to_polygon()
    else:
        # Normalized perpendicular vector for trace width
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
    
    trace_name = f"Trace_{layer}_{width}mm"
    return CircuitComponent(trace_name, geometry, "trace")


def create_realistic_circuit_layout():
    """Create a realistic circuit layout representing a microcontroller board."""
    
    # Define PCB bounds (50mm x 40mm)
    pcb_bounds = zlayout.Rectangle(0, 0, 50, 40)
    processor = zlayout.GeometryProcessor(pcb_bounds)
    
    components = []
    
    # Add main microcontroller (center of board)
    mcu = create_microcontroller(18, 13)
    components.append(mcu)
    processor.add_component(mcu.geometry)
    
    # Add power regulation circuit
    power_components = [
        # Buck converter IC
        create_resistor(8, 30, 0, "3.3V_REG", "1206"),
        
        # Input/output capacitors
        create_capacitor(5, 30, "22uF", "1210"),
        create_capacitor(12, 30, "100nF", "0603"),
        
        # Inductor (represented as elongated rectangle)
        create_resistor(8, 27, 0, "10uH", "1210"),
        
        # Feedback resistors
        create_resistor(8, 24, 0, "10k", "0603"),
        create_resistor(11, 24, 0, "3.3k", "0603"),
    ]
    
    for comp in power_components:
        components.append(comp)
        processor.add_component(comp.geometry)
    
    # Add bypass capacitors near MCU
    bypass_caps = [
        create_capacitor(15, 10, "100nF", "0603"),
        create_capacitor(35, 10, "100nF", "0603"),
        create_capacitor(15, 30, "100nF", "0603"),
        create_capacitor(35, 30, "100nF", "0603"),
    ]
    
    for cap in bypass_caps:
        components.append(cap)
        processor.add_component(cap.geometry)
    
    # Add crystal oscillator circuit
    crystal_components = [
        # 8MHz crystal
        create_resistor(40, 20, math.pi/2, "8MHz", "HC49"),
        
        # Load capacitors
        create_capacitor(37, 18, "22pF", "0603"),
        create_capacitor(37, 22, "22pF", "0603"),
    ]
    
    for comp in crystal_components:
        components.append(comp)
        processor.add_component(comp.geometry)
    
    # Add LED indicators
    led_components = [
        create_resistor(45, 35, 0, "LED_R", "0603"),
        create_resistor(45, 32, 0, "LED_G", "0603"),
        create_resistor(45, 29, 0, "LED_B", "0603"),
        
        # Current limiting resistors
        create_resistor(42, 35, 0, "330R", "0603"),
        create_resistor(42, 32, 0, "330R", "0603"),
        create_resistor(42, 29, 0, "330R", "0603"),
    ]
    
    for comp in led_components:
        components.append(comp)
        processor.add_component(comp.geometry)
    
    # Add connecting traces
    traces = [
        # Power traces (wider for current carrying)
        create_trace(zlayout.Point(5, 30), zlayout.Point(15, 30), 0.5, "power"),
        create_trace(zlayout.Point(32, 25), zlayout.Point(40, 25), 0.3, "power"),
        
        # Signal traces
        create_trace(zlayout.Point(32, 20), zlayout.Point(37, 20), 0.15, "signal"),
        create_trace(zlayout.Point(32, 18), zlayout.Point(37, 18), 0.15, "signal"),
        
        # LED control traces
        create_trace(zlayout.Point(32, 15), zlayout.Point(42, 35), 0.1, "signal"),
        create_trace(zlayout.Point(32, 16), zlayout.Point(42, 32), 0.1, "signal"),
        create_trace(zlayout.Point(32, 17), zlayout.Point(42, 29), 0.1, "signal"),
        
        # Some traces that might cause spacing issues (for testing)
        create_trace(zlayout.Point(20, 5), zlayout.Point(20, 8), 0.08, "signal"),
        create_trace(zlayout.Point(20.12, 5), zlayout.Point(20.12, 8), 0.08, "signal"),
    ]
    
    for trace in traces:
        components.append(trace)
        processor.add_component(trace.geometry)
    
    # Add potentially problematic geometries for testing
    
    # Sharp angled connector (simulating a non-standard connector)
    connector_vertices = [
        zlayout.Point(2, 15),
        zlayout.Point(8, 16),
        zlayout.Point(4, 20),  # Sharp angle here
        zlayout.Point(1, 18)
    ]
    sharp_connector = zlayout.Polygon(connector_vertices)
    components.append(CircuitComponent("Sharp_Connector", sharp_connector, "connector"))
    processor.add_component(sharp_connector)
    
    # Overlapping test components (manufacturing error simulation)
    overlap_rect1 = zlayout.Rectangle(45, 5, 3, 2).to_polygon()
    overlap_rect2 = zlayout.Rectangle(46, 6, 3, 2).to_polygon()
    
    components.extend([
        CircuitComponent("Test_Overlap1", overlap_rect1, "test"),
        CircuitComponent("Test_Overlap2", overlap_rect2, "test")
    ])
    processor.add_component(overlap_rect1)
    processor.add_component(overlap_rect2)
    
    return processor, components


def analyze_manufacturing_feasibility(processor):
    """Comprehensive manufacturing feasibility analysis."""
    
    print("=== Manufacturing Feasibility Analysis ===\n")
    
    # Define manufacturing process constraints
    process_constraints = {
        "prototype": {
            "min_trace_width": 0.1,
            "min_spacing": 0.1,
            "sharp_angle_limit": 20,
            "description": "Prototype PCB (loose tolerances)"
        },
        "standard": {
            "min_trace_width": 0.15,
            "min_spacing": 0.15,
            "sharp_angle_limit": 30,
            "description": "Standard PCB manufacturing"
        },
        "high_density": {
            "min_trace_width": 0.05,
            "min_spacing": 0.05,
            "sharp_angle_limit": 15,
            "description": "High-density interconnect (HDI)"
        }
    }
    
    analysis_results = {}
    
    for process_name, constraints in process_constraints.items():
        print(f"--- {process_name.upper()} PROCESS ---")
        print(f"Description: {constraints['description']}")
        
        # Run layout analysis
        analysis = processor.analyze_layout(
            sharp_angle_threshold=constraints["sharp_angle_limit"],
            narrow_distance_threshold=constraints["min_spacing"]
        )
        
        analysis_results[process_name] = analysis
        
        # Count violations
        violations = 0
        warnings = []
        
        if analysis['sharp_angles']['count'] > 0:
            violations += analysis['sharp_angles']['count']
            warnings.append(f"Sharp angles: {analysis['sharp_angles']['count']} violations")
            # Show specific angles
            for angle_info in analysis['sharp_angles']['details'][:3]:  # Show first 3
                warnings.append(f"  - {angle_info['angle']:.1f}° at {angle_info['location']}")
        
        if analysis['narrow_distances']['count'] > 0:
            violations += analysis['narrow_distances']['count']
            warnings.append(f"Spacing violations: {analysis['narrow_distances']['count']}")
            warnings.append(f"  - Minimum distance: {analysis['narrow_distances']['minimum']:.3f}mm")
        
        if analysis['intersections']['polygon_pairs'] > 0:
            violations += analysis['intersections']['polygon_pairs']
            warnings.append(f"Component intersections: {analysis['intersections']['polygon_pairs']} pairs")
        
        # Manufacturing assessment
        if violations == 0:
            print("✅ MANUFACTURABLE")
            print("  All constraints satisfied")
        else:
            print(f"❌ {violations} VIOLATIONS")
            for warning in warnings:
                print(f"  {warning}")
        
        # Calculate yield estimate
        yield_estimate = max(0, 100 - violations * 5)
        print(f"  Estimated yield: {yield_estimate}%")
        
        print()
    
    return analysis_results


def generate_component_report(components):
    """Generate detailed component analysis report."""
    
    print("=== Component Analysis Report ===\n")
    
    # Group components by type
    component_stats = {}
    total_area = 0
    
    for comp in components:
        comp_type = comp.component_type
        if comp_type not in component_stats:
            component_stats[comp_type] = {
                'count': 0,
                'total_area': 0,
                'components': []
            }
        
        component_stats[comp_type]['count'] += 1
        component_stats[comp_type]['components'].append(comp)
        
        # Calculate component area
        area = comp.geometry.area()
        component_stats[comp_type]['total_area'] += area
        total_area += area
    
    # Print summary
    print("Component Summary:")
    print(f"{'Type':<15} {'Count':<8} {'Area (mm²)':<12} {'% of Total':<10}")
    print("-" * 50)
    
    for comp_type, stats in sorted(component_stats.items()):
        percentage = (stats['total_area'] / total_area * 100) if total_area > 0 else 0
        print(f"{comp_type:<15} {stats['count']:<8} {stats['total_area']:<12.2f} {percentage:<10.1f}%")
    
    print(f"\nTotal components: {len(components)}")
    print(f"Total area: {total_area:.2f} mm²")
    
    # Component density analysis
    pcb_area = 50 * 40  # PCB dimensions
    utilization = (total_area / pcb_area) * 100
    print(f"PCB utilization: {utilization:.1f}%")
    
    if utilization > 80:
        print("⚠️  High component density - consider larger PCB or double-sided layout")
    elif utilization < 30:
        print("ℹ️  Low component density - PCB size could be optimized")
    else:
        print("✅ Good component density")
    
    return component_stats


def create_visualization(processor, components, analysis_results):
    """Create comprehensive visualization of the circuit and analysis."""
    
    print("\n=== Creating Visualizations ===")
    
    try:
        # Create visualizer with larger figure
        visualizer = zlayout.LayoutVisualizer(figsize=(16, 12))
        
        # Extract geometries for visualization
        polygons = [comp.geometry for comp in components if hasattr(comp.geometry, 'vertices')]
        
        # Create main layout plot
        print("Creating circuit layout visualization...")
        fig1 = visualizer.plot_layout(
            polygons, 
            title="Microcontroller Board Layout",
            show_grid=True,
            grid_spacing=5.0,
            component_labels=False  # Too many components for labels
        )
        
        # Add component type coloring
        colors = {
            'microcontroller': 'red',
            'resistor': 'blue',
            'capacitor': 'green',
            'trace': 'gray',
            'connector': 'orange',
            'test': 'purple'
        }
        
        for comp in components:
            if hasattr(comp.geometry, 'vertices'):
                color = colors.get(comp.component_type, 'black')
                visualizer.add_component_highlight(comp.geometry, color=color, alpha=0.6)
        
        # Create analysis results visualization
        print("Creating analysis visualization...")
        fig2 = visualizer.plot_analysis_results(
            polygons, 
            analysis_results['standard'],
            title="Design Rule Check Results (Standard Process)"
        )
        
        # Create process comparison chart
        print("Creating process comparison...")
        fig3 = visualizer.create_process_comparison_chart(analysis_results)
        
        plt.tight_layout()
        plt.show()
        
        return [fig1, fig2, fig3]
        
    except ImportError:
        print("Matplotlib not available. Install with: pip install matplotlib")
        return []


def main():
    """Main function demonstrating comprehensive EDA circuit analysis."""
    
    print("🔧 ZLayout - Advanced EDA Circuit Analysis")
    print("=" * 60)
    
    # Initialize ZLayout
    if not zlayout.initialize():
        print("❌ Failed to initialize ZLayout")
        return
    
    try:
        # Create realistic circuit layout
        print("Creating realistic microcontroller board layout...")
        processor, components = create_realistic_circuit_layout()
        print(f"✅ Created layout with {len(components)} components")
        
        # Analyze manufacturing feasibility
        analysis_results = analyze_manufacturing_feasibility(processor)
        
        # Generate component report
        component_stats = generate_component_report(components)
        
        # Create visualizations
        figures = create_visualization(processor, components, analysis_results)
        
        # Generate final report
        print("\n=== Final Assessment ===")
        
        # Find best manufacturing process
        best_process = None
        min_violations = float('inf')
        
        for process_name, analysis in analysis_results.items():
            violations = (analysis['sharp_angles']['count'] + 
                         analysis['narrow_distances']['count'] + 
                         analysis['intersections']['polygon_pairs'])
            
            if violations < min_violations:
                min_violations = violations
                best_process = process_name
        
        print(f"Recommended process: {best_process.upper()}")
        print(f"Total violations: {min_violations}")
        
        if min_violations == 0:
            print("✅ Design is ready for manufacturing!")
        else:
            print("⚠️  Design needs revision before manufacturing")
            print("\nRecommended actions:")
            print("1. Increase spacing between narrow traces")
            print("2. Smooth sharp angles in connector geometries")
            print("3. Check for component overlaps")
        
        # Offer to save results
        save_results = input("\nSave analysis results and visualizations? (y/n): ").lower().strip()
        if save_results == 'y':
            # Save visualizations
            if figures:
                visualizer = zlayout.LayoutVisualizer()
                visualizer.save_plots(figures, "eda_circuit_analysis")
            
            # Save detailed report
            with open("eda_analysis_report.txt", "w") as f:
                f.write("ZLayout EDA Circuit Analysis Report\n")
                f.write("=" * 50 + "\n\n")
                
                f.write("COMPONENT SUMMARY\n")
                f.write("-" * 20 + "\n")
                for comp_type, stats in component_stats.items():
                    f.write(f"{comp_type}: {stats['count']} components, "
                           f"{stats['total_area']:.2f} mm²\n")
                
                f.write("\nMANUFACTURING ANALYSIS\n")
                f.write("-" * 25 + "\n")
                for process_name, analysis in analysis_results.items():
                    violations = (analysis['sharp_angles']['count'] + 
                                analysis['narrow_distances']['count'] + 
                                analysis['intersections']['polygon_pairs'])
                    f.write(f"{process_name}: {violations} violations\n")
                
                f.write(f"\nRECOMMENDED PROCESS: {best_process.upper()}\n")
            
            print("✅ Results saved to files!")
        
    except Exception as e:
        print(f"❌ Error during analysis: {e}")
        import traceback
        traceback.print_exc()
        
    finally:
        # Clean up
        zlayout.cleanup()
    
    print("\n🎉 EDA circuit analysis completed!")


if __name__ == "__main__":
    main()
```

## Key Learning Points

### 1. Realistic Component Modeling

This example shows how to model real electronic components with accurate dimensions:

- **Microcontroller**: QFP-64 package with 0.65mm pin pitch
- **Resistors/Capacitors**: Standard SMD packages (0402, 0603, 0805, 1206)
- **Traces**: Proper width calculation for current carrying capacity
- **Connectors**: Complex geometries that may have sharp angles

### 2. Manufacturing Constraints

Different PCB manufacturing processes have different capabilities:

| Process | Min Trace Width | Min Spacing | Sharp Angle Limit |
|---------|----------------|-------------|-------------------|
| Prototype | 0.1mm | 0.1mm | 20° |
| Standard | 0.15mm | 0.15mm | 30° |
| HDI | 0.05mm | 0.05mm | 15° |

### 3. Design Rule Checking (DRC)

The analysis checks for common manufacturing issues:

- **Sharp angles**: Can cause etching problems
- **Narrow spacing**: Risk of shorts or manufacturing defects
- **Component overlaps**: Physical conflicts in assembly

### 4. Component Density Analysis

Optimal PCB utilization depends on:
- Component count and size
- Thermal considerations
- Assembly accessibility
- Cost optimization

### 5. Visualization for Design Review

Effective visualization helps identify:
- Component placement issues
- Routing congestion
- Manufacturing constraint violations
- Design optimization opportunities

## Running the Example

```bash
# Run the complete analysis
python eda_circuit_analysis.py

# Expected output:
# - Component count by type
# - Manufacturing feasibility for each process
# - Visual layout with analysis overlay
# - Detailed report with recommendations
```

## Educational Insights

### Why This Example Matters

1. **Real-world applicability**: Uses actual component packages and constraints
2. **Manufacturing awareness**: Teaches the importance of DRC early in design
3. **Process optimization**: Shows how to choose the right manufacturing process
4. **Quality metrics**: Demonstrates yield estimation and risk assessment

### Extensions and Variations

Try modifying the example to explore:

- **Different component types**: Add inductors, crystals, connectors
- **Multi-layer boards**: Separate power and signal layers
- **Thermal analysis**: Add temperature constraints
- **Signal integrity**: Consider high-speed design rules
- **Assembly constraints**: Add pick-and-place accessibility checks

This example provides a solid foundation for understanding how ZLayout can be used in real EDA workflows, from initial design through manufacturing verification.

---

*This example demonstrates the power of ZLayout for professional EDA applications. The combination of geometric analysis, manufacturing constraints, and visualization capabilities makes it suitable for production use in electronic design workflows.* 