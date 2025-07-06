#!/usr/bin/env python3
"""
Basic usage example of ZLayout library.

This example demonstrates:
1. Creating geometric shapes (rectangles and polygons)
2. Setting up spatial indexing with quadtree
3. Analyzing layout for sharp angles, narrow distances, and intersections
4. Visualizing results
"""

import sys
import os

# Add parent directory to path to import zlayout
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

import zlayout
import matplotlib.pyplot as plt


def create_sample_layout():
    """Create a sample EDA layout with various components."""
    
    # Define world bounds for the layout
    world_bounds = zlayout.Rectangle(0, 0, 100, 100)
    
    # Create geometry processor
    processor = zlayout.GeometryProcessor(world_bounds)
    
    # Add some rectangular components (representing chips, resistors, etc.)
    rectangles = [
        zlayout.Rectangle(10, 10, 15, 8),   # Component 1
        zlayout.Rectangle(30, 15, 12, 6),   # Component 2
        zlayout.Rectangle(50, 25, 20, 10),  # Component 3
        zlayout.Rectangle(75, 40, 8, 15),   # Component 4
    ]
    
    for rect in rectangles:
        processor.add_component(rect)
    
    # Add some polygonal components (representing custom shapes)
    polygons = [
        # Triangle with sharp angle
        zlayout.Polygon([
            zlayout.Point(20, 50),
            zlayout.Point(35, 55),
            zlayout.Point(22, 65)
        ]),
        
        # L-shaped component
        zlayout.Polygon([
            zlayout.Point(45, 60),
            zlayout.Point(60, 60),
            zlayout.Point(60, 70),
            zlayout.Point(55, 70),
            zlayout.Point(55, 75),
            zlayout.Point(45, 75)
        ]),
        
        # Narrow component that might cause spacing issues
        zlayout.Polygon([
            zlayout.Point(70, 10),
            zlayout.Point(90, 12),
            zlayout.Point(88, 18),
            zlayout.Point(68, 16)
        ]),
        
        # Component with potential intersection
        zlayout.Polygon([
            zlayout.Point(25, 20),
            zlayout.Point(40, 25),
            zlayout.Point(35, 35),
            zlayout.Point(20, 30)
        ])
    ]
    
    for polygon in polygons:
        processor.add_component(polygon)
    
    return processor, rectangles, polygons


def demonstrate_analysis(processor):
    """Demonstrate layout analysis capabilities."""
    
    print("=== ZLayout Analysis Demo ===\n")
    
    # Perform comprehensive analysis
    print("Analyzing layout...")
    optimization_results = processor.optimize_layout()
    
    analysis = optimization_results['analysis']
    
    # Print results
    print(f"Optimization Score: {optimization_results['optimization_score']:.1f}/100\n")
    
    print("Sharp Angles Analysis:")
    print(f"  - Found {analysis['sharp_angles']['count']} sharp angles")
    if analysis['sharp_angles']['count'] > 0:
        print(f"  - Sharpest angle: {analysis['sharp_angles']['sharpest']:.1f}°")
        print(f"  - Average angle: {analysis['sharp_angles']['average']:.1f}°")
    print()
    
    print("Narrow Distances Analysis:")
    print(f"  - Found {analysis['narrow_distances']['count']} narrow regions")
    if analysis['narrow_distances']['count'] > 0:
        print(f"  - Minimum distance: {analysis['narrow_distances']['minimum']:.3f}")
        print(f"  - Average distance: {analysis['narrow_distances']['average']:.3f}")
    print()
    
    print("Edge Intersections Analysis:")
    print(f"  - Found {analysis['intersections']['polygon_pairs']} intersecting polygon pairs")
    print(f"  - Total intersection points: {analysis['intersections']['total_points']}")
    print()
    
    print("Suggestions:")
    for suggestion in optimization_results['suggestions']:
        print(f"  • {suggestion}")
    
    return optimization_results


def demonstrate_visualization(processor, rectangles, polygons, optimization_results):
    """Demonstrate visualization capabilities."""
    
    print("\n=== Generating Visualizations ===")
    
    visualizer = zlayout.LayoutVisualizer()
    
    # Create layout overview
    print("Creating layout overview...")
    fig1 = visualizer.plot_layout(polygons, rectangles, "Sample EDA Layout")
    
    # Create analysis visualization
    print("Creating analysis visualization...")
    fig2 = visualizer.plot_analysis_results(polygons, optimization_results['analysis'])
    
    # Create optimization summary
    print("Creating optimization summary...")
    fig3 = visualizer.plot_optimization_summary(optimization_results)
    
    # Show plots
    plt.show()
    
    return [fig1, fig2, fig3]


def demonstrate_quadtree():
    """Demonstrate quadtree spatial indexing."""
    
    print("\n=== QuadTree Spatial Indexing Demo ===")
    
    # Create a quadtree
    world_bounds = zlayout.Rectangle(0, 0, 100, 100)
    quadtree = zlayout.QuadTree(world_bounds, capacity=3, max_depth=4)
    
    # Add some objects
    objects = [
        zlayout.Rectangle(10, 10, 5, 5),
        zlayout.Rectangle(15, 20, 8, 6),
        zlayout.Rectangle(50, 50, 12, 8),
        zlayout.Rectangle(75, 25, 6, 10),
        zlayout.Rectangle(30, 70, 15, 5),
    ]
    
    print(f"Inserting {len(objects)} objects into quadtree...")
    for i, obj in enumerate(objects):
        success = quadtree.insert(obj)
        print(f"  Object {i+1}: {'✓' if success else '✗'}")
    
    # Query examples
    print(f"\nQuadTree contains {quadtree.size()} objects")
    
    # Range query
    query_region = zlayout.Rectangle(0, 0, 50, 50)
    objects_in_region = quadtree.query_range(query_region)
    print(f"Objects in region (0,0,50,50): {len(objects_in_region)}")
    
    # Point query
    query_point = zlayout.Point(12, 12)
    objects_at_point = quadtree.query_point(query_point)
    print(f"Objects containing point (12,12): {len(objects_at_point)}")
    
    # Find potential intersections
    intersections = quadtree.find_intersections()
    print(f"Potential intersecting pairs: {len(intersections)}")


def main():
    """Main demonstration function."""
    
    print("Welcome to ZLayout - Advanced EDA Layout Library!")
    print("=" * 50)
    
    # Create sample layout
    processor, rectangles, polygons = create_sample_layout()
    
    # Demonstrate analysis
    optimization_results = demonstrate_analysis(processor)
    
    # Demonstrate quadtree
    demonstrate_quadtree()
    
    # Demonstrate visualization
    try:
        figures = demonstrate_visualization(processor, rectangles, polygons, optimization_results)
        
        # Optionally save plots
        save_plots = input("\nSave plots to files? (y/n): ").lower().strip()
        if save_plots == 'y':
            visualizer = zlayout.LayoutVisualizer()
            visualizer.save_plots(figures, "zlayout_demo")
            print("Plots saved!")
    
    except ImportError:
        print("\nNote: Matplotlib not available. Skipping visualization demo.")
        print("Install matplotlib to see visual results: pip install matplotlib")
    
    print("\nDemo completed!")


if __name__ == "__main__":
    main()