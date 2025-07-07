# Range Query Operations

*Efficient Spatial Search for Polygon Overlap Detection*

## Overview

Range query operations find all polygons that overlap with a given rectangular region. This is a fundamental operation in EDA tools for collision detection, area selection, and proximity analysis. The efficiency of range queries directly impacts interactive performance.

## Problem Definition

### Query Types

- **Point Query**: Find polygons containing a specific point
- **Rectangle Query**: Find polygons overlapping a rectangular region  
- **Circle Query**: Find polygons within circular radius
- **Polygon Query**: Find polygons overlapping with arbitrary polygon

### Mathematical Foundation

For polygon P and query rectangle Q:
```
Overlap(P, Q) = true if:
  BoundingBox(P) ∩ BoundingBox(Q) ≠ ∅ AND
  DetailedIntersection(P, Q) = true
```

Two-stage filtering:
1. **Coarse Filter**: Bounding box intersection test (fast)
2. **Fine Filter**: Detailed geometric intersection (precise)

---

## Data Structure Comparison

### 1. QuadTree Implementation

```cpp
class QuadTree {
private:
    struct Node {
        Rectangle bounds;
        std::vector<int> polygon_ids;
        std::array<std::unique_ptr<Node>, 4> children;
        bool is_leaf = true;
        
        static constexpr int MAX_POLYGONS = 10;
        static constexpr int MAX_DEPTH = 12;
    };
    
    std::unique_ptr<Node> root;
    std::vector<Polygon> polygons;

public:
    QuadTree(const Rectangle& world_bounds) {
        root = std::make_unique<Node>();
        root->bounds = world_bounds;
    }
    
    void insert(const Polygon& polygon) {
        int poly_id = polygons.size();
        polygons.push_back(polygon);
        insert_recursive(root.get(), poly_id, 0);
    }
    
    std::vector<int> range_query(const Rectangle& query_rect) const {
        std::vector<int> results;
        range_query_recursive(root.get(), query_rect, results);
        
        // Fine filtering: detailed intersection test
        std::vector<int> filtered_results;
        for (int poly_id : results) {
            if (polygons[poly_id].intersects(query_rect)) {
                filtered_results.push_back(poly_id);
            }
        }
        
        return filtered_results;
    }

private:
    void insert_recursive(Node* node, int poly_id, int depth) {
        if (node->is_leaf) {
            node->polygon_ids.push_back(poly_id);
            
            // Split if necessary
            if (node->polygon_ids.size() > Node::MAX_POLYGONS && 
                depth < Node::MAX_DEPTH) {
                split_node(node, depth);
            }
        } else {
            // Insert into appropriate children
            Rectangle poly_bbox = polygons[poly_id].bounding_box();
            
            for (int i = 0; i < 4; ++i) {
                if (node->children[i]->bounds.intersects(poly_bbox)) {
                    insert_recursive(node->children[i].get(), poly_id, depth + 1);
                }
            }
        }
    }
    
    void range_query_recursive(const Node* node, const Rectangle& query_rect,
                              std::vector<int>& results) const {
        if (!node->bounds.intersects(query_rect)) {
            return;  // No intersection with node bounds
        }
        
        if (node->is_leaf) {
            // Add all polygons in this leaf (coarse filtering)
            results.insert(results.end(), 
                          node->polygon_ids.begin(), 
                          node->polygon_ids.end());
        } else {
            // Recursively query children
            for (const auto& child : node->children) {
                if (child) {
                    range_query_recursive(child.get(), query_rect, results);
                }
            }
        }
    }
    
    void split_node(Node* node, int depth) {
        node->is_leaf = false;
        
        // Create quadrants
        double mid_x = (node->bounds.min_x + node->bounds.max_x) / 2.0;
        double mid_y = (node->bounds.min_y + node->bounds.max_y) / 2.0;
        
        node->children[0] = std::make_unique<Node>();  // NW
        node->children[0]->bounds = Rectangle(
            node->bounds.min_x, mid_y, mid_x, node->bounds.max_y);
            
        node->children[1] = std::make_unique<Node>();  // NE
        node->children[1]->bounds = Rectangle(
            mid_x, mid_y, node->bounds.max_x, node->bounds.max_y);
            
        node->children[2] = std::make_unique<Node>();  // SW
        node->children[2]->bounds = Rectangle(
            node->bounds.min_x, node->bounds.min_y, mid_x, mid_y);
            
        node->children[3] = std::make_unique<Node>();  // SE
        node->children[3]->bounds = Rectangle(
            mid_x, node->bounds.min_y, node->bounds.max_x, mid_y);
        
        // Redistribute polygons
        for (int poly_id : node->polygon_ids) {
            for (int i = 0; i < 4; ++i) {
                Rectangle poly_bbox = polygons[poly_id].bounding_box();
                if (node->children[i]->bounds.intersects(poly_bbox)) {
                    insert_recursive(node->children[i].get(), poly_id, depth + 1);
                }
            }
        }
        
        node->polygon_ids.clear();
    }
};
```

### 2. R-tree Implementation

```cpp
class RTree {
private:
    struct Node {
        Rectangle bounding_box;
        std::vector<std::pair<Rectangle, int>> entries;  // (bbox, polygon_id)
        std::vector<std::unique_ptr<Node>> children;
        bool is_leaf = true;
        
        static constexpr int MIN_ENTRIES = 2;
        static constexpr int MAX_ENTRIES = 8;
    };
    
    std::unique_ptr<Node> root;
    std::vector<Polygon> polygons;

public:
    RTree() {
        root = std::make_unique<Node>();
    }
    
    void insert(const Polygon& polygon) {
        int poly_id = polygons.size();
        polygons.push_back(polygon);
        
        Rectangle poly_bbox = polygon.bounding_box();
        insert_recursive(root.get(), poly_bbox, poly_id);
    }
    
    std::vector<int> range_query(const Rectangle& query_rect) const {
        std::vector<int> results;
        range_query_recursive(root.get(), query_rect, results);
        return results;
    }

private:
    void insert_recursive(Node* node, const Rectangle& bbox, int poly_id) {
        if (node->is_leaf) {
            node->entries.emplace_back(bbox, poly_id);
            node->bounding_box = node->bounding_box.union_with(bbox);
            
            if (node->entries.size() > Node::MAX_ENTRIES) {
                split_node(node);
            }
        } else {
            // Choose subtree with minimum area enlargement
            int best_child = choose_subtree(node, bbox);
            insert_recursive(node->children[best_child].get(), bbox, poly_id);
            
            // Update bounding box
            node->bounding_box = node->bounding_box.union_with(bbox);
        }
    }
    
    void range_query_recursive(const Node* node, const Rectangle& query_rect,
                              std::vector<int>& results) const {
        if (!node->bounding_box.intersects(query_rect)) {
            return;
        }
        
        if (node->is_leaf) {
            for (const auto& [bbox, poly_id] : node->entries) {
                if (bbox.intersects(query_rect)) {
                    // Detailed intersection test
                    if (polygons[poly_id].intersects(query_rect)) {
                        results.push_back(poly_id);
                    }
                }
            }
        } else {
            for (const auto& child : node->children) {
                range_query_recursive(child.get(), query_rect, results);
            }
        }
    }
    
    int choose_subtree(Node* node, const Rectangle& bbox) {
        int best_child = 0;
        double min_enlargement = std::numeric_limits<double>::max();
        
        for (size_t i = 0; i < node->children.size(); ++i) {
            Rectangle union_bbox = node->children[i]->bounding_box.union_with(bbox);
            double enlargement = union_bbox.area() - 
                               node->children[i]->bounding_box.area();
            
            if (enlargement < min_enlargement) {
                min_enlargement = enlargement;
                best_child = static_cast<int>(i);
            }
        }
        
        return best_child;
    }
};
```

### 3. Z-order Spatial Hashing

```cpp
class ZOrderIndex {
private:
    struct ZOrderEntry {
        uint64_t z_value;
        int polygon_id;
        Rectangle bbox;
        
        bool operator<(const ZOrderEntry& other) const {
            return z_value < other.z_value;
        }
    };
    
    std::vector<ZOrderEntry> entries;
    std::vector<Polygon> polygons;
    Rectangle world_bounds;
    bool is_sorted = false;

public:
    ZOrderIndex(const Rectangle& bounds) : world_bounds(bounds) {}
    
    void insert(const Polygon& polygon) {
        int poly_id = polygons.size();
        polygons.push_back(polygon);
        
        Rectangle bbox = polygon.bounding_box();
        uint64_t z_value = compute_z_order(bbox.center());
        
        entries.push_back({z_value, poly_id, bbox});
        is_sorted = false;
    }
    
    std::vector<int> range_query(const Rectangle& query_rect) const {
        if (!is_sorted) {
            std::sort(const_cast<std::vector<ZOrderEntry>&>(entries).begin(),
                     const_cast<std::vector<ZOrderEntry>&>(entries).end());
            const_cast<ZOrderIndex*>(this)->is_sorted = true;
        }
        
        std::vector<int> results;
        
        // Calculate Z-order range for query rectangle
        auto [z_min, z_max] = compute_z_range(query_rect);
        
        // Binary search for range
        auto lower = std::lower_bound(entries.begin(), entries.end(),
                                    ZOrderEntry{z_min, -1, Rectangle()});
        auto upper = std::upper_bound(entries.begin(), entries.end(),
                                    ZOrderEntry{z_max, -1, Rectangle()});
        
        // Check candidates in Z-order range
        for (auto it = lower; it != upper; ++it) {
            if (it->bbox.intersects(query_rect)) {
                if (polygons[it->polygon_id].intersects(query_rect)) {
                    results.push_back(it->polygon_id);
                }
            }
        }
        
        return results;
    }

private:
    uint64_t compute_z_order(const Point& point) const {
        // Normalize coordinates to [0, 1]
        double norm_x = (point.x - world_bounds.min_x) / world_bounds.width();
        double norm_y = (point.y - world_bounds.min_y) / world_bounds.height();
        
        // Convert to integer coordinates
        uint32_t int_x = static_cast<uint32_t>(norm_x * UINT32_MAX);
        uint32_t int_y = static_cast<uint32_t>(norm_y * UINT32_MAX);
        
        // Interleave bits to create Z-order value
        return interleave_bits(int_x, int_y);
    }
    
    uint64_t interleave_bits(uint32_t x, uint32_t y) const {
        uint64_t result = 0;
        for (int i = 0; i < 32; ++i) {
            result |= ((x & (1ULL << i)) << i) | ((y & (1ULL << i)) << (i + 1));
        }
        return result;
    }
    
    std::pair<uint64_t, uint64_t> compute_z_range(const Rectangle& rect) const {
        // Simplified: use min and max corners
        uint64_t z_min = compute_z_order(Point(rect.min_x, rect.min_y));
        uint64_t z_max = compute_z_order(Point(rect.max_x, rect.max_y));
        
        return {std::min(z_min, z_max), std::max(z_min, z_max)};
    }
};
```

---

## Performance Analysis

### Complexity Comparison

| Data Structure | Build Time | Query Time | Space | Best Use Case |
|----------------|------------|------------|--------|---------------|
| QuadTree | O(n log n) | O(log n + k) | O(n) | Uniform distribution |
| R-tree | O(n log n) | O(log n + k) | O(n) | Clustered rectangles |
| Z-order | O(n log n) | O(log n + k) | O(n) | High-dimension data |
| Hybrid | O(n log n) | O(log n + k) | O(n) | Mixed workloads |

Where:
- `n` = number of polygons
- `k` = number of results returned

### Python Benchmarking Framework

```python
class RangeQueryBenchmark:
    """Comprehensive benchmarking for range query algorithms."""
    
    def __init__(self, world_bounds):
        self.world_bounds = world_bounds
        self.algorithms = {
            'quadtree': QuadTreeIndex(world_bounds),
            'rtree': RTreeIndex(world_bounds),
            'zorder': ZOrderIndex(world_bounds),
            'hybrid': HybridIndex(world_bounds)
        }
    
    def benchmark_range_queries(self, polygons, query_rects, num_trials=10):
        """Benchmark different range query algorithms."""
        
        results = {}
        
        for algo_name, index in self.algorithms.items():
            print(f"\nBenchmarking {algo_name}...")
            
            # Build index
            build_start = time.perf_counter()
            for polygon in polygons:
                index.insert(polygon)
            build_time = time.perf_counter() - build_start
            
            # Query performance
            query_times = []
            result_counts = []
            
            for trial in range(num_trials):
                for query_rect in query_rects:
                    query_start = time.perf_counter()
                    query_results = index.range_query(query_rect)
                    query_time = time.perf_counter() - query_start
                    
                    query_times.append(query_time)
                    result_counts.append(len(query_results))
            
            results[algo_name] = {
                'build_time': build_time,
                'avg_query_time': np.mean(query_times),
                'avg_results': np.mean(result_counts),
                'memory_usage': index.memory_usage()
            }
        
        return results
    
    def generate_test_scenarios(self, n_polygons):
        """Generate different test scenarios for comprehensive evaluation."""
        
        scenarios = {}
        
        # Uniform random distribution
        scenarios['uniform'] = self.generate_uniform_polygons(n_polygons)
        
        # Clustered distribution
        scenarios['clustered'] = self.generate_clustered_polygons(n_polygons, n_clusters=10)
        
        # Real EDA layout pattern
        scenarios['eda_pattern'] = self.generate_eda_layout(n_polygons)
        
        return scenarios
    
    def generate_query_patterns(self):
        """Generate different query patterns for testing."""
        
        queries = {}
        
        # Small area queries (1% of world area)
        small_area = self.world_bounds.area() * 0.01
        side_length = math.sqrt(small_area)
        queries['small'] = [
            Rectangle(x, y, side_length, side_length)
            for x, y in self.random_points(100)
        ]
        
        # Medium area queries (5% of world area)
        medium_area = self.world_bounds.area() * 0.05
        side_length = math.sqrt(medium_area)
        queries['medium'] = [
            Rectangle(x, y, side_length, side_length)
            for x, y in self.random_points(50)
        ]
        
        # Large area queries (20% of world area)
        large_area = self.world_bounds.area() * 0.20
        side_length = math.sqrt(large_area)
        queries['large'] = [
            Rectangle(x, y, side_length, side_length)
            for x, y in self.random_points(20)
        ]
        
        return queries

# Run comprehensive benchmark
def run_comprehensive_benchmark():
    world_bounds = Rectangle(0, 0, 10000, 10000)
    benchmark = RangeQueryBenchmark(world_bounds)
    
    polygon_counts = [1000, 5000, 10000, 50000]
    
    for n_polys in polygon_counts:
        print(f"\n=== Testing with {n_polys} polygons ===")
        
        scenarios = benchmark.generate_test_scenarios(n_polys)
        query_patterns = benchmark.generate_query_patterns()
        
        for scenario_name, polygons in scenarios.items():
            print(f"\nScenario: {scenario_name}")
            
            for query_name, queries in query_patterns.items():
                print(f"Query pattern: {query_name}")
                
                results = benchmark.benchmark_range_queries(polygons, queries)
                
                # Print results table
                print(f"{'Algorithm':<12} {'Build(ms)':<12} {'Query(μs)':<12} {'Results':<10} {'Memory(MB)':<12}")
                print("-" * 60)
                
                for algo, metrics in results.items():
                    print(f"{algo:<12} {metrics['build_time']*1000:<12.1f} "
                          f"{metrics['avg_query_time']*1e6:<12.1f} "
                          f"{metrics['avg_results']:<10.1f} "
                          f"{metrics['memory_usage']/1e6:<12.1f}")
```

---

## Interactive Tutorial

### Tutorial 1: Basic Range Queries

```python
import zlayout
import matplotlib.pyplot as plt
import numpy as np

# Create test layout with various polygon types
world_bounds = zlayout.Rectangle(0, 0, 1000, 1000)
index = zlayout.QuadTree(world_bounds)

# Add different types of components
components = [
    # Regular components
    zlayout.Rectangle(100, 100, 50, 30),   # Small rectangle
    zlayout.Rectangle(200, 150, 80, 60),   # Medium rectangle
    zlayout.Rectangle(400, 300, 120, 90),  # Large rectangle
    
    # Triangular components
    zlayout.Polygon([(500, 100), (600, 100), (550, 180)]),
    zlayout.Polygon([(700, 200), (800, 220), (750, 300)]),
    
    # Complex polygons
    zlayout.Polygon([(50, 500), (150, 480), (180, 550), (120, 600), (30, 580)]),
]

# Build spatial index
for component in components:
    index.insert(component)

# Define query regions
query_regions = [
    zlayout.Rectangle(80, 80, 100, 100),    # Overlaps with small rectangle
    zlayout.Rectangle(450, 250, 200, 200),  # Overlaps with large rectangle
    zlayout.Rectangle(0, 450, 300, 200),    # Overlaps with complex polygon
]

# Perform range queries
for i, query_rect in enumerate(query_regions):
    print(f"\n=== Query {i+1}: {query_rect} ===")
    
    # Find overlapping components
    overlapping_ids = index.range_query(query_rect)
    
    print(f"Found {len(overlapping_ids)} overlapping components:")
    for comp_id in overlapping_ids:
        component = components[comp_id]
        overlap_area = component.intersection_area(query_rect)
        print(f"  Component {comp_id}: overlap area = {overlap_area:.2f}")
```

### Tutorial 2: Performance Comparison

```python
def compare_index_performance():
    """Compare performance of different spatial indexing methods."""
    
    # Generate test data
    world_bounds = zlayout.Rectangle(0, 0, 5000, 5000)
    n_polygons = 10000
    
    # Create different index types
    indexes = {
        'QuadTree': zlayout.QuadTree(world_bounds),
        'R-tree': zlayout.RTree(world_bounds),
        'Z-order': zlayout.ZOrderIndex(world_bounds)
    }
    
    # Generate test polygons
    polygons = generate_random_layout(n_polygons, world_bounds)
    
    # Build indexes and measure time
    build_times = {}
    for name, index in indexes.items():
        start_time = time.perf_counter()
        
        for polygon in polygons:
            index.insert(polygon)
            
        build_times[name] = time.perf_counter() - start_time
        print(f"{name} build time: {build_times[name]:.3f}s")
    
    # Generate query rectangles
    query_rects = [
        zlayout.Rectangle(x, y, 100, 100)
        for x, y in zip(
            np.random.uniform(0, 4900, 1000),
            np.random.uniform(0, 4900, 1000)
        )
    ]
    
    # Measure query performance
    query_times = {}
    for name, index in indexes.items():
        times = []
        
        for query_rect in query_rects:
            start_time = time.perf_counter()
            results = index.range_query(query_rect)
            end_time = time.perf_counter()
            
            times.append(end_time - start_time)
        
        query_times[name] = {
            'avg': np.mean(times),
            'median': np.median(times),
            'p95': np.percentile(times, 95)
        }
    
    # Print comparison table
    print(f"\n{'Index Type':<12} {'Build(s)':<10} {'Avg Query(μs)':<15} {'P95 Query(μs)':<15}")
    print("-" * 55)
    
    for name in indexes.keys():
        print(f"{name:<12} {build_times[name]:<10.3f} "
              f"{query_times[name]['avg']*1e6:<15.1f} "
              f"{query_times[name]['p95']*1e6:<15.1f}")

# Run performance comparison
compare_index_performance()
```

---

## Real-world Applications

### 1. Interactive Selection Tool

```python
class InteractiveSelector:
    """Interactive polygon selection for EDA GUI."""
    
    def __init__(self, spatial_index):
        self.index = spatial_index
        self.selection_cache = {}
    
    def select_in_region(self, selection_rect, selection_mode='replace'):
        """Select polygons in rectangular region."""
        
        # Use cache for repeated queries
        cache_key = (selection_rect.to_tuple(), selection_mode)
        if cache_key in self.selection_cache:
            return self.selection_cache[cache_key]
        
        # Perform range query
        candidates = self.index.range_query(selection_rect)
        
        selected_polygons = []
        for poly_id in candidates:
            polygon = self.index.get_polygon(poly_id)
            
            # Different selection criteria
            if selection_mode == 'intersect':
                if polygon.intersects(selection_rect):
                    selected_polygons.append(poly_id)
            elif selection_mode == 'contain':
                if selection_rect.contains(polygon):
                    selected_polygons.append(poly_id)
            elif selection_mode == 'center':
                if selection_rect.contains(polygon.center()):
                    selected_polygons.append(poly_id)
        
        # Cache result
        self.selection_cache[cache_key] = selected_polygons
        return selected_polygons
    
    def incremental_select(self, current_selection, new_rect, operation='add'):
        """Incrementally update selection based on operation."""
        
        new_selection = self.select_in_region(new_rect)
        
        if operation == 'add':
            return list(set(current_selection) | set(new_selection))
        elif operation == 'subtract':
            return list(set(current_selection) - set(new_selection))
        elif operation == 'intersect':
            return list(set(current_selection) & set(new_selection))
        else:  # replace
            return new_selection
```

### 2. Collision Detection System

```python
class CollisionDetector:
    """Real-time collision detection for component placement."""
    
    def __init__(self, spatial_index):
        self.index = spatial_index
        self.collision_threshold = 0.1  # Minimum clearance
    
    def check_placement(self, new_component, position):
        """Check if component can be placed at position without collision."""
        
        # Transform component to new position
        positioned_component = new_component.translate(position)
        component_bbox = positioned_component.bounding_box()
        
        # Expand bounding box by collision threshold
        expanded_bbox = component_bbox.expand(self.collision_threshold)
        
        # Find nearby components
        nearby_components = self.index.range_query(expanded_bbox)
        
        collisions = []
        for comp_id in nearby_components:
            existing_component = self.index.get_polygon(comp_id)
            
            # Check actual collision
            distance = positioned_component.distance_to(existing_component)
            if distance < self.collision_threshold:
                collisions.append({
                    'component_id': comp_id,
                    'distance': distance,
                    'overlap_area': positioned_component.intersection_area(existing_component)
                })
        
        return {
            'can_place': len(collisions) == 0,
            'collisions': collisions,
            'collision_count': len(collisions)
        }
    
    def find_valid_positions(self, component, search_region, grid_spacing=10):
        """Find all valid positions for component within search region."""
        
        valid_positions = []
        
        x_positions = np.arange(search_region.min_x, search_region.max_x, grid_spacing)
        y_positions = np.arange(search_region.min_y, search_region.max_y, grid_spacing)
        
        for x in x_positions:
            for y in y_positions:
                position = Point(x, y)
                result = self.check_placement(component, position)
                
                if result['can_place']:
                    valid_positions.append(position)
        
        return valid_positions
```

---

## Optimization Techniques

### 1. Query Result Caching

```python
class CachedRangeQuery:
    """Range query with intelligent caching."""
    
    def __init__(self, spatial_index, cache_size=1000):
        self.index = spatial_index
        self.cache = {}
        self.cache_hits = 0
        self.cache_misses = 0
        self.max_cache_size = cache_size
    
    def range_query(self, query_rect, cache_enabled=True):
        """Range query with optional caching."""
        
        if not cache_enabled:
            return self.index.range_query(query_rect)
        
        # Check cache
        cache_key = self._compute_cache_key(query_rect)
        
        if cache_key in self.cache:
            self.cache_hits += 1
            return self.cache[cache_key]
        
        # Cache miss - perform actual query
        self.cache_misses += 1
        results = self.index.range_query(query_rect)
        
        # Add to cache (with eviction if necessary)
        if len(self.cache) >= self.max_cache_size:
            # Simple LRU eviction
            oldest_key = next(iter(self.cache))
            del self.cache[oldest_key]
        
        self.cache[cache_key] = results
        return results
    
    def _compute_cache_key(self, query_rect):
        """Compute cache key for query rectangle."""
        # Quantize coordinates to reduce cache misses for similar queries
        quantum = 1.0
        return (
            round(query_rect.min_x / quantum) * quantum,
            round(query_rect.min_y / quantum) * quantum,
            round(query_rect.width() / quantum) * quantum,
            round(query_rect.height() / quantum) * quantum
        )
    
    def get_cache_stats(self):
        """Get cache performance statistics."""
        total_queries = self.cache_hits + self.cache_misses
        hit_rate = self.cache_hits / total_queries if total_queries > 0 else 0
        
        return {
            'cache_hits': self.cache_hits,
            'cache_misses': self.cache_misses,
            'hit_rate': hit_rate,
            'cache_size': len(self.cache)
        }
```

### 2. Multi-threaded Range Queries

```python
class ParallelRangeQuery:
    """Parallel range query processing for large query sets."""
    
    def __init__(self, spatial_index, num_threads=None):
        self.index = spatial_index
        self.num_threads = num_threads or multiprocessing.cpu_count()
    
    def batch_range_query(self, query_rects):
        """Process multiple range queries in parallel."""
        
        # Split queries into batches
        batch_size = max(1, len(query_rects) // self.num_threads)
        query_batches = [
            query_rects[i:i + batch_size]
            for i in range(0, len(query_rects), batch_size)
        ]
        
        # Process batches in parallel
        with multiprocessing.Pool(self.num_threads) as pool:
            batch_results = pool.map(self._process_query_batch, query_batches)
        
        # Combine results
        all_results = []
        for batch_result in batch_results:
            all_results.extend(batch_result)
        
        return all_results
    
    def _process_query_batch(self, query_batch):
        """Process a batch of queries in single thread."""
        batch_results = []
        
        for query_rect in query_batch:
            results = self.index.range_query(query_rect)
            batch_results.append(results)
        
        return batch_results
```

---

## Summary

Range query operations are fundamental to spatial data processing in EDA tools:

**Algorithm Selection Guidelines:**
- **QuadTree**: Best for uniformly distributed data and interactive applications
- **R-tree**: Optimal for clustered rectangular data and bulk operations  
- **Z-order**: Excellent for high-dimensional data and parallel processing
- **Hybrid**: Combines benefits of multiple approaches for diverse workloads

**Performance Characteristics:**
- All algorithms achieve O(log n + k) query complexity
- Build time is O(n log n) for balanced structures
- Memory usage is linear O(n) in number of objects

**Optimization Strategies:**
1. Choose appropriate data structure based on data distribution
2. Implement query result caching for repeated patterns
3. Use parallel processing for batch queries
4. Apply two-stage filtering for complex geometry

Range queries enable efficient spatial analysis essential for interactive EDA tools and automated layout verification systems. 