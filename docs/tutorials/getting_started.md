# Getting Started with ZLayout

Welcome to ZLayout! This guide will help you get up and running with the Advanced Electronic Design Automation Layout Library.

## Prerequisites

Before installing ZLayout, ensure you have:

- **Python 3.7+** (for Python bindings)
- **C++17 compatible compiler** (for C++ development)
  - Windows: Visual Studio 2019+ or MSVC 2012+
  - Linux: GCC 7+ or Clang 6+
  - macOS: Xcode 10+
- **CMake 3.15+** or **XMake** (for building from source)

## Installation

### Option 1: Python Package (Recommended)

```bash
# Install from PyPI
pip install zlayout

# Install with visualization support
pip install zlayout[viz]

# Install development version
pip install git+https://github.com/your-org/zlayout.git
```

### Option 2: Build from Source

```bash
# Clone the repository
git clone https://github.com/your-org/zlayout.git
cd zlayout

# Using XMake (recommended)
xmake build
xmake install

# Or using CMake
mkdir build && cd build
cmake ..
cmake --build .
```

### Option 3: Conda Package

```bash
# Install from conda-forge
conda install -c conda-forge zlayout
```

## Verify Installation

```python
import zlayout
print(f"ZLayout version: {zlayout.__version__}")
print(f"Features: {zlayout.get_features()}")
```

Expected output:
```
ZLayout version: 1.0.0
Features: ['geometry', 'spatial', 'optimization', 'visualization']
```

## Basic Concepts

### 1. Geometry Primitives

ZLayout provides fundamental geometric primitives for EDA layouts:

```python
import zlayout

# Create points
point1 = zlayout.Point(10.0, 20.0)
point2 = zlayout.Point(30.0, 40.0)

# Create rectangles
rect = zlayout.Rectangle(0, 0, 100, 50)  # x, y, width, height
print(f"Rectangle area: {rect.area()}")

# Create polygons
vertices = [
    zlayout.Point(0, 0),
    zlayout.Point(100, 0),
    zlayout.Point(100, 50),
    zlayout.Point(0, 50)
]
polygon = zlayout.Polygon(vertices)
print(f"Polygon area: {polygon.area()}")
```

### 2. Spatial Indexing

Efficient spatial queries using QuadTree:

```python
# Create a world bounds
world_bounds = zlayout.Rectangle(0, 0, 1000, 1000)

# Create QuadTree for spatial indexing
quadtree = zlayout.QuadTree(world_bounds)

# Insert components
components = [
    zlayout.Rectangle(10, 10, 50, 30),
    zlayout.Rectangle(100, 100, 40, 40),
    zlayout.Rectangle(200, 200, 60, 25)
]

for component in components:
    quadtree.insert(component)

# Query for intersecting components
query_area = zlayout.Rectangle(0, 0, 80, 80)
results = quadtree.query_range(query_area)
print(f"Found {len(results)} components in query area")
```

### 3. Layout Analysis

Analyze layouts for manufacturing constraints:

```python
# Create geometry processor
processor = zlayout.GeometryProcessor(world_bounds)

# Add components to processor
for component in components:
    processor.add_component(component)

# Run analysis
analysis = processor.analyze_layout(
    sharp_angle_threshold=30.0,    # degrees
    narrow_distance_threshold=0.15  # microns
)

print(f"Sharp angles: {analysis['sharp_angles']['count']}")
print(f"Narrow distances: {analysis['narrow_distances']['count']}")
print(f"Intersections: {analysis['intersections']['polygon_pairs']}")
```

## Your First ZLayout Program

Let's create a complete example that demonstrates key ZLayout features:

```python
#!/usr/bin/env python3
"""
ZLayout Getting Started Example
Demonstrates basic geometry, spatial indexing, and analysis
"""

import zlayout
import matplotlib.pyplot as plt

def create_sample_layout():
    """Create a sample EDA layout."""
    
    # Define world bounds
    world_bounds = zlayout.Rectangle(0, 0, 200, 150)
    
    # Create components representing circuit elements
    components = [
        # Main chip
        zlayout.Rectangle(50, 50, 80, 60),
        
        # Memory blocks
        zlayout.Rectangle(10, 10, 30, 20),
        zlayout.Rectangle(10, 120, 30, 20),
        
        # I/O pads
        zlayout.Rectangle(160, 20, 15, 10),
        zlayout.Rectangle(160, 40, 15, 10),
        zlayout.Rectangle(160, 60, 15, 10),
        
        # Power rails
        zlayout.Rectangle(0, 70, 200, 5),
        zlayout.Rectangle(0, 80, 200, 5),
    ]
    
    return world_bounds, components

def demonstrate_spatial_indexing(world_bounds, components):
    """Demonstrate spatial indexing capabilities."""
    
    print("=== Spatial Indexing Demo ===")
    
    # Create QuadTree
    quadtree = zlayout.QuadTree(world_bounds)
    
    # Insert all components
    for i, component in enumerate(components):
        quadtree.insert(component, data=f"Component_{i}")
    
    # Perform spatial queries
    query_regions = [
        zlayout.Rectangle(0, 0, 100, 100),    # Left half
        zlayout.Rectangle(100, 0, 100, 150),  # Right half
        zlayout.Rectangle(40, 40, 100, 80),   # Center region
    ]
    
    for i, query in enumerate(query_regions):
        results = quadtree.query_range(query)
        print(f"Query {i+1}: Found {len(results)} components")
    
    return quadtree

def demonstrate_layout_analysis(world_bounds, components):
    """Demonstrate layout analysis features."""
    
    print("\n=== Layout Analysis Demo ===")
    
    # Create processor
    processor = zlayout.GeometryProcessor(world_bounds)
    
    # Add components
    for component in components:
        processor.add_component(component)
    
    # Run comprehensive analysis
    analysis = processor.analyze_layout(
        sharp_angle_threshold=45.0,
        narrow_distance_threshold=5.0
    )
    
    print(f"Total components: {len(components)}")
    print(f"Sharp angles detected: {analysis['sharp_angles']['count']}")
    print(f"Narrow distances: {analysis['narrow_distances']['count']}")
    print(f"Component intersections: {analysis['intersections']['polygon_pairs']}")
    
    # Check manufacturing feasibility
    if analysis['sharp_angles']['count'] == 0 and analysis['narrow_distances']['count'] == 0:
        print("✅ Layout passes basic manufacturing checks!")
    else:
        print("⚠️  Layout has potential manufacturing issues")
    
    return analysis

def visualize_layout(world_bounds, components, analysis):
    """Visualize the layout and analysis results."""
    
    print("\n=== Visualization Demo ===")
    
    try:
        # Create visualizer
        visualizer = zlayout.LayoutVisualizer(figsize=(12, 8))
        
        # Convert rectangles to polygons for visualization
        polygons = [rect.to_polygon() for rect in components]
        
        # Create layout plot
        fig = visualizer.plot_layout(
            polygons, 
            title="Sample EDA Layout",
            show_grid=True,
            component_labels=True
        )
        
        # Add analysis overlay
        visualizer.add_analysis_overlay(analysis)
        
        plt.tight_layout()
        plt.show()
        
        return fig
        
    except ImportError:
        print("Matplotlib not available. Install with: pip install matplotlib")
        return None

def main():
    """Main demonstration function."""
    
    print("🚀 ZLayout Getting Started Demo")
    print("=" * 40)
    
    # Initialize library
    if not zlayout.initialize():
        print("❌ Failed to initialize ZLayout")
        return
    
    try:
        # Create sample layout
        world_bounds, components = create_sample_layout()
        print(f"Created layout with {len(components)} components")
        
        # Demonstrate features
        quadtree = demonstrate_spatial_indexing(world_bounds, components)
        analysis = demonstrate_layout_analysis(world_bounds, components)
        
        # Visualize results
        fig = visualize_layout(world_bounds, components, analysis)
        
        print("\n✅ Demo completed successfully!")
        print("\nNext steps:")
        print("1. Explore the tutorials in docs/tutorials/")
        print("2. Check out advanced examples in docs/examples/")
        print("3. Read the API documentation")
        
    except Exception as e:
        print(f"❌ Error during demo: {e}")
        
    finally:
        # Cleanup
        zlayout.cleanup()

if __name__ == "__main__":
    main()
```

## Key Concepts Explained

### Coordinate System
- ZLayout uses a standard 2D coordinate system
- Origin (0,0) is at the bottom-left
- X increases to the right, Y increases upward
- All coordinates are in floating-point units

### Tolerance and Precision
- Geometric operations use configurable tolerance (default: 1e-10)
- Suitable for both micro-scale (chip design) and macro-scale (PCB design)
- Precision can be adjusted based on application requirements

### Memory Management
- Efficient memory pools for large-scale layouts
- Automatic cleanup of temporary objects
- RAII principles for resource management

### Thread Safety
- Core algorithms are thread-safe
- OpenMP support for parallel processing
- Concurrent spatial queries supported

## Common Patterns

### 1. Batch Processing
```python
# Process multiple layouts efficiently
layouts = [create_layout(i) for i in range(10)]
results = zlayout.batch_process(layouts, analysis_config)
```

### 2. Hierarchical Design
```python
# Build complex circuits from sub-components
cpu_block = zlayout.create_component_block("CPU", cpu_components)
memory_block = zlayout.create_component_block("Memory", memory_components)
top_level = zlayout.create_component_block("SoC", [cpu_block, memory_block])
```

### 3. Design Rule Checking
```python
# Define and check manufacturing rules
rules = zlayout.DesignRules(
    min_spacing=0.15,
    min_width=0.10,
    max_aspect_ratio=10.0
)
violations = rules.check_layout(components)
```

## Troubleshooting

### Common Issues

1. **Import Error**: Ensure ZLayout is installed correctly
   ```bash
   pip install --upgrade zlayout
   ```

2. **Performance Issues**: Enable OpenMP for parallel processing
   ```python
   zlayout.initialize(enable_openmp=True)
   ```

3. **Memory Issues**: Use batch processing for large layouts
   ```python
   zlayout.set_memory_limit(8192)  # 8GB limit
   ```

### Getting Help

- 📚 [Documentation](https://your-org.github.io/zlayout/)
- 💬 [Discussion Forum](https://github.com/your-org/zlayout/discussions)
- 🐛 [Issue Tracker](https://github.com/your-org/zlayout/issues)
- 📧 [Email Support](mailto:support@zlayout.org)

## Next Steps

Now that you've completed the getting started guide, explore these advanced topics:

1. **[Geometry Operations](geometry_operations.md)** - Deep dive into geometric algorithms
2. **[Spatial Indexing](spatial_indexing.md)** - Advanced spatial data structures
3. **[Layout Analysis](layout_analysis.md)** - Comprehensive layout verification
4. **[Component System](component_system.md)** - Building hierarchical designs
5. **[Layout Optimization](layout_optimization.md)** - Performance optimization techniques

---

*Happy coding with ZLayout! 🚀* 