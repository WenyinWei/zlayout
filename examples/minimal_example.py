#!/usr/bin/env python3
"""
Minimal ZLayout example without external dependencies.

This example demonstrates core functionality without requiring
numpy or matplotlib.
"""

import sys
import os

# Add parent directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

import zlayout


def test_basic_geometry():
    """Test basic geometry operations."""
    
    print("=== Basic Geometry Tests ===")
    
    # Create points
    p1 = zlayout.Point(0, 0)
    p2 = zlayout.Point(3, 4)
    p3 = zlayout.Point(1, 1)
    
    print(f"Point 1: {p1}")
    print(f"Point 2: {p2}")
    print(f"Distance p1 to p2: {p1.distance_to(p2):.2f}")
    
    # Create rectangles
    rect1 = zlayout.Rectangle(0, 0, 10, 5)
    rect2 = zlayout.Rectangle(5, 2, 8, 6)
    
    print(f"\nRectangle 1: {rect1}")
    print(f"Rectangle 2: {rect2}")
    print(f"Rectangles intersect: {rect1.intersects(rect2)}")
    print(f"Point {p3} in rect1: {rect1.contains_point(p3)}")
    
    # Create polygon
    triangle = zlayout.Polygon([
        zlayout.Point(0, 0),
        zlayout.Point(4, 0), 
        zlayout.Point(2, 3)
    ])
    
    print(f"\nTriangle: {triangle}")
    print(f"Triangle area: {triangle.area():.2f}")
    print(f"Triangle is convex: {triangle.is_convex()}")
    
    return rect1, rect2, triangle


def test_spatial_indexing():
    """Test quadtree spatial indexing."""
    
    print("\n=== Spatial Indexing Tests ===")
    
    # Create quadtree
    world_bounds = zlayout.Rectangle(0, 0, 100, 100)
    quadtree = zlayout.QuadTree(world_bounds, capacity=2)
    
    # Add objects
    objects = [
        zlayout.Rectangle(10, 10, 5, 5),
        zlayout.Rectangle(20, 20, 8, 6),
        zlayout.Rectangle(50, 50, 12, 8),
        zlayout.Rectangle(75, 25, 6, 10),
    ]
    
    print(f"Adding {len(objects)} objects to quadtree...")
    for i, obj in enumerate(objects):
        success = quadtree.insert(obj)
        print(f"  Object {i+1}: {'✓' if success else '✗'}")
    
    print(f"QuadTree size: {quadtree.size()}")
    
    # Test queries
    query_region = zlayout.Rectangle(0, 0, 30, 30)
    found_objects = quadtree.query_range(query_region)
    print(f"Objects in region (0,0,30,30): {len(found_objects)}")
    
    query_point = zlayout.Point(12, 12)
    containing_objects = quadtree.query_point(query_point)
    print(f"Objects containing point (12,12): {len(containing_objects)}")
    
    return quadtree


def test_layout_analysis():
    """Test layout analysis without visualization."""
    
    print("\n=== Layout Analysis Tests ===")
    
    # Create processor
    world_bounds = zlayout.Rectangle(0, 0, 100, 100)
    processor = zlayout.GeometryProcessor(world_bounds)
    
    # Add components
    components = [
        # Regular rectangles
        zlayout.Rectangle(10, 10, 15, 8),
        zlayout.Rectangle(30, 15, 12, 6),
        zlayout.Rectangle(50, 25, 20, 10),
        
        # Triangles with potential sharp angles
        zlayout.Polygon([
            zlayout.Point(70, 10),
            zlayout.Point(85, 12),
            zlayout.Point(72, 20)  # Sharp angle
        ]),
        
        # Close components (narrow spacing)
        zlayout.Rectangle(15, 35, 5, 3),
        zlayout.Rectangle(21, 35, 5, 3),  # Only 1 unit apart
        
        # Potentially overlapping
        zlayout.Polygon([
            zlayout.Point(40, 50),
            zlayout.Point(55, 52),
            zlayout.Point(53, 65),
            zlayout.Point(38, 63)
        ]),
        zlayout.Polygon([
            zlayout.Point(50, 55),
            zlayout.Point(65, 57),
            zlayout.Point(63, 70),
            zlayout.Point(48, 68)
        ])
    ]
    
    for comp in components:
        processor.add_component(comp)
    
    # Run analysis
    print("Running layout analysis...")
    results = processor.analyze_layout(
        sharp_angle_threshold=45.0,
        narrow_distance_threshold=2.0
    )
    
    # Print results
    print(f"Results structure: {results.keys()}")
    analysis = results
    
    print(f"\nAnalysis Results:")
    print(f"  Sharp angles found: {analysis['sharp_angles']['count']}")
    if analysis['sharp_angles']['count'] > 0:
        print(f"    Sharpest: {analysis['sharp_angles']['sharpest']:.1f}°")
    
    print(f"  Narrow regions found: {analysis['narrow_distances']['count']}")
    if analysis['narrow_distances']['count'] > 0:
        print(f"    Minimum distance: {analysis['narrow_distances']['minimum']:.3f}")
    
    print(f"  Intersecting polygon pairs: {analysis['intersections']['polygon_pairs']}")
    
    # Get optimization suggestions
    optimization = processor.optimize_layout()
    print(f"\nOptimization Score: {optimization['optimization_score']:.1f}/100")
    
    if optimization['suggestions']:
        print("Suggestions:")
        for suggestion in optimization['suggestions']:
            print(f"  • {suggestion}")
    else:
        print("✅ No issues found!")
    
    return processor, optimization


def _print_design_rule_results(process_name, violations, analysis):
    """Helper function to print design rule checking results without conditionals in main test."""
    # Pass case
    if violations == 0:
        print(f"  ✅ Passes {process_name} constraints")
        return
    
    # Violation case
    print(f"  ❌ {violations} violations for {process_name}")
    print(f"     Sharp angles: {analysis['sharp_angles']['count']}")
    print(f"     Spacing issues: {analysis['narrow_distances']['count']}")


def _test_prototype_process(processor):
    """Test prototype manufacturing process constraints."""
    print("\n--- Prototype Process ---")
    analysis = processor.analyze_layout(sharp_angle_threshold=20, narrow_distance_threshold=0.5)
    violations = (analysis['sharp_angles']['count'] + analysis['narrow_distances']['count'] + 
                 analysis['intersections']['polygon_pairs'])
    _print_design_rule_results("Prototype", violations, analysis)


def _test_standard_process(processor):
    """Test standard manufacturing process constraints."""
    print("\n--- Standard Process ---")
    analysis = processor.analyze_layout(sharp_angle_threshold=30, narrow_distance_threshold=1.5)
    violations = (analysis['sharp_angles']['count'] + analysis['narrow_distances']['count'] + 
                 analysis['intersections']['polygon_pairs'])
    _print_design_rule_results("Standard", violations, analysis)


def _test_high_precision_process(processor):
    """Test high-precision manufacturing process constraints."""
    print("\n--- High-precision Process ---")
    analysis = processor.analyze_layout(sharp_angle_threshold=45, narrow_distance_threshold=2.5)
    violations = (analysis['sharp_angles']['count'] + analysis['narrow_distances']['count'] + 
                 analysis['intersections']['polygon_pairs'])
    _print_design_rule_results("High-precision", violations, analysis)


def test_design_rules():
    """Test design rule checking for different manufacturing processes."""
    
    print("\n=== Design Rule Checking ===")
    
    # Create test layout
    world_bounds = zlayout.Rectangle(0, 0, 50, 50)
    processor = zlayout.GeometryProcessor(world_bounds)
    
    # Add components with known issues
    processor.add_component(zlayout.Rectangle(5, 5, 10, 8))    # Normal component
    processor.add_component(zlayout.Rectangle(16, 5, 10, 8))   # Close spacing (1 unit)
    
    # Sharp angle component
    sharp_poly = zlayout.Polygon([
        zlayout.Point(10, 20),
        zlayout.Point(25, 21),
        zlayout.Point(12, 25)  # Very sharp angle
    ])
    processor.add_component(sharp_poly)
    
    # Test different manufacturing constraints
    _test_prototype_process(processor)
    _test_standard_process(processor)
    _test_high_precision_process(processor)


def main():
    """Run all minimal tests."""
    
    print("ZLayout - Minimal Functionality Test")
    print("=" * 40)
    
    try:
        # Test basic geometry
        rect1, rect2, triangle = test_basic_geometry()
        
        # Test spatial indexing
        quadtree = test_spatial_indexing()
        
        # Test layout analysis
        processor, optimization = test_layout_analysis()
        
        # Test design rules
        test_design_rules()
        
        print("\n" + "=" * 40)
        print("✅ All tests completed successfully!")
        print("\nNote: For visualization features, install matplotlib:")
        print("  pip install matplotlib")
        
    except Exception as e:
        print(f"\n❌ Test failed with error: {e}")
        import traceback
        traceback.print_exc()
        return 1
    
    return 0


if __name__ == "__main__":
    sys.exit(main())