# Edge Intersection Detection Algorithm

*Geometry Validation for Layout Integrity*

## Overview

Edge intersection detection identifies where polygon edges cross each other, which is critical for detecting layout errors, self-intersecting polygons, and overlapping components. This algorithm forms the foundation of geometric validation in EDA tools.

## Problem Definition

### Types of Intersections

- **Self-intersection**: Edges within the same polygon cross each other
- **Polygon-to-polygon**: Edges from different polygons intersect
- **Degenerate cases**: Coincident edges, touching endpoints
- **Numerical precision**: Near-intersections due to floating-point errors

### Mathematical Foundation

For two line segments P₁P₂ and P₃P₄:
```
Intersection exists if:
- Line segments are not parallel
- Intersection point lies within both segments
- Parametric solution: P = P₁ + t(P₂ - P₁) = P₃ + s(P₄ - P₃)
```

---

## Core Algorithms

### Algorithm 1: Brute Force O(n²)

```cpp
class EdgeIntersectionDetector {
public:
    struct Intersection {
        Point location;
        int edge1_polygon_id, edge1_index;
        int edge2_polygon_id, edge2_index;
        bool is_proper;  // True if segments cross, false if just touching
    };
    
    std::vector<Intersection> detectIntersections(
        const std::vector<Polygon>& polygons
    ) {
        std::vector<Intersection> intersections;
        
        // Check all edge pairs between all polygons
        for (size_t poly1_id = 0; poly1_id < polygons.size(); ++poly1_id) {
            for (size_t poly2_id = poly1_id; poly2_id < polygons.size(); ++poly2_id) {
                auto poly_intersections = checkPolygonPair(
                    polygons[poly1_id], polygons[poly2_id], 
                    poly1_id, poly2_id
                );
                intersections.insert(intersections.end(),
                                   poly_intersections.begin(),
                                   poly_intersections.end());
            }
        }
        
        return intersections;
    }

private:
    std::vector<Intersection> checkPolygonPair(
        const Polygon& poly1, const Polygon& poly2,
        int poly1_id, int poly2_id
    ) {
        std::vector<Intersection> intersections;
        auto edges1 = poly1.edges();
        auto edges2 = poly2.edges();
        
        for (size_t i = 0; i < edges1.size(); ++i) {
            for (size_t j = 0; j < edges2.size(); ++j) {
                // Skip adjacent edges within same polygon
                if (poly1_id == poly2_id && areAdjacent(i, j, edges1.size())) {
                    continue;
                }
                
                auto intersection = segmentIntersection(
                    edges1[i].first, edges1[i].second,
                    edges2[j].first, edges2[j].second
                );
                
                if (intersection.exists) {
                    intersections.push_back({
                        intersection.point,
                        poly1_id, static_cast<int>(i),
                        poly2_id, static_cast<int>(j),
                        intersection.is_proper
                    });
                }
            }
        }
        
        return intersections;
    }
    
    struct IntersectionResult {
        bool exists;
        Point point;
        bool is_proper;
    };
    
    IntersectionResult segmentIntersection(
        const Point& p1, const Point& p2,
        const Point& p3, const Point& p4
    ) {
        // Vector representations
        double dx1 = p2.x - p1.x, dy1 = p2.y - p1.y;
        double dx2 = p4.x - p3.x, dy2 = p4.y - p3.y;
        double dx3 = p1.x - p3.x, dy3 = p1.y - p3.y;
        
        // Cross product for determinant
        double denominator = dx1 * dy2 - dy1 * dx2;
        
        // Check if lines are parallel
        if (std::abs(denominator) < 1e-10) {
            return {false, Point(), false};
        }
        
        // Calculate parameters
        double t = (dx2 * dy3 - dy2 * dx3) / denominator;
        double s = (dx1 * dy3 - dy1 * dx3) / denominator;
        
        // Check if intersection is within both segments
        bool within_seg1 = (t >= 0.0 && t <= 1.0);
        bool within_seg2 = (s >= 0.0 && s <= 1.0);
        
        if (within_seg1 && within_seg2) {
            Point intersection_point(
                p1.x + t * dx1,
                p1.y + t * dy1
            );
            
            // Check if it's a proper intersection (not just touching)
            bool is_proper = (t > 1e-10 && t < 1.0 - 1e-10) &&
                            (s > 1e-10 && s < 1.0 - 1e-10);
            
            return {true, intersection_point, is_proper};
        }
        
        return {false, Point(), false};
    }
};
```

### Algorithm 2: Sweep Line O((n+k) log n)

```cpp
class SweepLineIntersectionDetector {
private:
    struct Event {
        double x;
        enum Type { START, END, INTERSECTION } type;
        int edge_id;
        Point point;
        
        bool operator<(const Event& other) const {
            if (std::abs(x - other.x) > 1e-10) return x < other.x;
            return type < other.type;  // Process START before END
        }
    };
    
    struct Edge {
        Point start, end;
        int polygon_id, edge_index;
        
        double y_at_x(double x) const {
            if (std::abs(end.x - start.x) < 1e-10) return start.y;
            double t = (x - start.x) / (end.x - start.x);
            return start.y + t * (end.y - start.y);
        }
    };

public:
    std::vector<Intersection> detectIntersectionsSweepLine(
        const std::vector<Polygon>& polygons
    ) {
        std::vector<Event> events;
        std::vector<Edge> edges;
        
        // Create events for all edges
        int edge_id = 0;
        for (size_t poly_id = 0; poly_id < polygons.size(); ++poly_id) {
            auto poly_edges = polygons[poly_id].edges();
            
            for (size_t edge_idx = 0; edge_idx < poly_edges.size(); ++edge_idx) {
                const auto& edge = poly_edges[edge_idx];
                
                // Ensure left-to-right ordering
                Point left = edge.first, right = edge.second;
                if (left.x > right.x) std::swap(left, right);
                
                edges.push_back({left, right, 
                               static_cast<int>(poly_id), 
                               static_cast<int>(edge_idx)});
                
                events.push_back({left.x, Event::START, edge_id, left});
                events.push_back({right.x, Event::END, edge_id, right});
                
                edge_id++;
            }
        }
        
        // Sort events by x-coordinate
        std::sort(events.begin(), events.end());
        
        // Sweep line status structure (ordered set by y-coordinate)
        std::set<int, decltype([this](int a, int b) {
            return this->compareEdgesByY(a, b);
        })> active_edges(
            [this](int a, int b) { return compareEdgesByY(a, b); }
        );
        
        std::vector<Intersection> intersections;
        
        // Process events
        for (const auto& event : events) {
            if (event.type == Event::START) {
                // Add edge to active set
                auto [iter, inserted] = active_edges.insert(event.edge_id);
                
                // Check intersections with neighbors
                if (iter != active_edges.begin()) {
                    auto prev = std::prev(iter);
                    checkAndAddIntersection(*prev, event.edge_id, 
                                          intersections, edges);
                }
                
                auto next = std::next(iter);
                if (next != active_edges.end()) {
                    checkAndAddIntersection(event.edge_id, *next,
                                          intersections, edges);
                }
                
            } else if (event.type == Event::END) {
                // Remove edge from active set
                auto iter = active_edges.find(event.edge_id);
                if (iter != active_edges.end()) {
                    // Check if neighbors now intersect
                    auto prev = (iter != active_edges.begin()) ? 
                               std::prev(iter) : active_edges.end();
                    auto next = std::next(iter);
                    
                    active_edges.erase(iter);
                    
                    if (prev != active_edges.end() && next != active_edges.end()) {
                        checkAndAddIntersection(*prev, *next,
                                              intersections, edges);
                    }
                }
            }
        }
        
        return intersections;
    }

private:
    std::vector<Edge> edges_storage;
    double current_sweep_x;
    
    bool compareEdgesByY(int edge_a, int edge_b) {
        double y_a = edges_storage[edge_a].y_at_x(current_sweep_x);
        double y_b = edges_storage[edge_b].y_at_x(current_sweep_x);
        
        if (std::abs(y_a - y_b) > 1e-10) return y_a < y_b;
        return edge_a < edge_b;  // Tie-breaking
    }
    
    void checkAndAddIntersection(int edge1_id, int edge2_id,
                                std::vector<Intersection>& intersections,
                                const std::vector<Edge>& edges) {
        const auto& edge1 = edges[edge1_id];
        const auto& edge2 = edges[edge2_id];
        
        auto intersection = segmentIntersection(
            edge1.start, edge1.end,
            edge2.start, edge2.end
        );
        
        if (intersection.exists) {
            intersections.push_back({
                intersection.point,
                edge1.polygon_id, edge1.edge_index,
                edge2.polygon_id, edge2.edge_index,
                intersection.is_proper
            });
        }
    }
};
```

---

## Python Implementation

```python
class EdgeIntersectionDetector:
    """Optimized edge intersection detection for EDA layouts."""
    
    @staticmethod
    def detect_intersections(polygons, include_touching=False):
        """
        Detect all edge intersections in polygon collection.
        
        Args:
            polygons: List of Polygon objects
            include_touching: Include endpoint touching as intersections
            
        Returns:
            List of intersection dictionaries
        """
        intersections = []
        
        # Use spatial indexing for efficiency
        spatial_index = SpatialIndex()
        edge_data = []
        
        # Index all edges
        for poly_id, polygon in enumerate(polygons):
            edges = polygon.edges
            for edge_id, (start, end) in enumerate(edges):
                bbox = BoundingBox.from_points([start, end])
                edge_info = {
                    'polygon_id': poly_id,
                    'edge_id': edge_id,
                    'start': start,
                    'end': end,
                    'bbox': bbox
                }
                spatial_index.insert(bbox, len(edge_data))
                edge_data.append(edge_info)
        
        # Check intersections
        checked_pairs = set()
        
        for i, edge1 in enumerate(edge_data):
            # Query nearby edges
            candidates = spatial_index.query(edge1['bbox'])
            
            for j in candidates:
                if i >= j:  # Avoid duplicate checks
                    continue
                    
                edge2 = edge_data[j]
                
                # Skip adjacent edges in same polygon
                if (edge1['polygon_id'] == edge2['polygon_id'] and 
                    abs(edge1['edge_id'] - edge2['edge_id']) <= 1):
                    continue
                
                pair = (i, j)
                if pair in checked_pairs:
                    continue
                checked_pairs.add(pair)
                
                # Check for intersection
                intersection = EdgeIntersectionDetector._segment_intersection(
                    edge1['start'], edge1['end'],
                    edge2['start'], edge2['end']
                )
                
                if intersection['exists']:
                    if include_touching or intersection['is_proper']:
                        intersections.append({
                            'point': intersection['point'],
                            'polygon1_id': edge1['polygon_id'],
                            'edge1_id': edge1['edge_id'],
                            'polygon2_id': edge2['polygon_id'], 
                            'edge2_id': edge2['edge_id'],
                            'is_proper': intersection['is_proper']
                        })
        
        return intersections
    
    @staticmethod
    def _segment_intersection(p1, p2, p3, p4):
        """Calculate intersection between two line segments."""
        # Direction vectors
        d1 = (p2[0] - p1[0], p2[1] - p1[1])
        d2 = (p4[0] - p3[0], p4[1] - p3[1])
        d3 = (p1[0] - p3[0], p1[1] - p3[1])
        
        # Cross products for determinant
        denominator = d1[0] * d2[1] - d1[1] * d2[0]
        
        # Check for parallel lines
        if abs(denominator) < 1e-10:
            return {'exists': False, 'point': None, 'is_proper': False}
        
        # Calculate parameters
        t = (d2[0] * d3[1] - d2[1] * d3[0]) / denominator
        s = (d1[0] * d3[1] - d1[1] * d3[0]) / denominator
        
        # Check if intersection is within both segments
        if 0.0 <= t <= 1.0 and 0.0 <= s <= 1.0:
            intersection_point = (
                p1[0] + t * d1[0],
                p1[1] + t * d1[1]
            )
            
            # Determine if it's a proper intersection
            epsilon = 1e-10
            is_proper = (epsilon < t < 1.0 - epsilon and 
                        epsilon < s < 1.0 - epsilon)
            
            return {
                'exists': True,
                'point': intersection_point,
                'is_proper': is_proper
            }
        
        return {'exists': False, 'point': None, 'is_proper': False}
```

---

## Complexity Analysis

### Time Complexity Comparison

| Algorithm | Preprocessing | Detection | Total | Space |
|-----------|--------------|-----------|--------|--------|
| Brute Force | None | O(n²m²) | O(n²m²) | O(1) |
| Sweep Line | O(nm log(nm)) | O((nm+k)log(nm)) | O((nm+k)log(nm)) | O(nm) |
| Spatial Index | O(nm log(nm)) | O(nm log(nm) + k) | O(nm log(nm) + k) | O(nm) |

Where:
- `n` = number of polygons
- `m` = average edges per polygon
- `k` = number of intersections found

### Performance Benchmarks

```python
def benchmark_intersection_algorithms():
    """Compare performance of different intersection algorithms."""
    
    polygon_counts = [10, 50, 100, 500]
    edge_counts = [4, 8, 16, 32]  # Average edges per polygon
    
    results = {}
    
    for n_polys in polygon_counts:
        for avg_edges in edge_counts:
            test_polygons = generate_complex_layout(n_polys, avg_edges)
            
            # Brute force method
            start = time.perf_counter()
            bf_detector = BruteForceDetector()
            bf_results = bf_detector.detect_intersections(test_polygons)
            bf_time = time.perf_counter() - start
            
            # Sweep line method
            start = time.perf_counter()
            sl_detector = SweepLineDetector()
            sl_results = sl_detector.detect_intersections(test_polygons)
            sl_time = time.perf_counter() - start
            
            # Spatial index method
            start = time.perf_counter()
            si_detector = EdgeIntersectionDetector()
            si_results = si_detector.detect_intersections(test_polygons)
            si_time = time.perf_counter() - start
            
            key = f"{n_polys}p_{avg_edges}e"
            results[key] = {
                'brute_force': bf_time,
                'sweep_line': sl_time,
                'spatial_index': si_time,
                'intersections_found': len(si_results)
            }
            
            print(f"{key}: BF={bf_time:.3f}s, SL={sl_time:.3f}s, SI={si_time:.3f}s")
    
    return results
```

---

## Interactive Tutorial

### Tutorial 1: Basic Intersection Detection

```python
import zlayout
import matplotlib.pyplot as plt

# Create test case with known intersections
polygons = [
    # Self-intersecting bowtie polygon
    zlayout.Polygon([
        (0, 0), (4, 4), (4, 0), (0, 4)  # Creates X-shape intersection
    ]),
    
    # Two overlapping rectangles
    zlayout.Rectangle(5, 1, 3, 2),  # Rectangle 1
    zlayout.Rectangle(6, 0, 3, 3),  # Rectangle 2 - overlaps with Rectangle 1
    
    # Triangle intersecting rectangle
    zlayout.Polygon([
        (10, 0), (13, 0), (11.5, 3)   # Triangle
    ]),
    zlayout.Rectangle(11, 1, 2, 1),   # Rectangle intersecting triangle
]

detector = zlayout.EdgeIntersectionDetector()
intersections = detector.detect_intersections(polygons)

print(f"Found {len(intersections)} edge intersections:")
for i, intersection in enumerate(intersections):
    print(f"Intersection {i+1}:")
    print(f"  Location: ({intersection['point'][0]:.2f}, {intersection['point'][1]:.2f})")
    print(f"  Between: Polygon {intersection['polygon1_id']} edge {intersection['edge1_id']}")
    print(f"           Polygon {intersection['polygon2_id']} edge {intersection['edge2_id']}")
    print(f"  Type: {'Proper crossing' if intersection['is_proper'] else 'Endpoint touching'}")
```

### Tutorial 2: Self-Intersection Validation

```python
def validate_polygon_integrity(polygon):
    """Check if a polygon has self-intersections."""
    
    # Create single-polygon list for intersection detection
    result = detector.detect_intersections([polygon])
    
    # Filter for self-intersections only
    self_intersections = [
        inter for inter in result 
        if inter['polygon1_id'] == inter['polygon2_id']
    ]
    
    if self_intersections:
        print(f"Polygon has {len(self_intersections)} self-intersections:")
        for inter in self_intersections:
            print(f"  Edge {inter['edge1_id']} intersects edge {inter['edge2_id']}")
            print(f"  At point: ({inter['point'][0]:.3f}, {inter['point'][1]:.3f})")
        return False
    else:
        print("Polygon is valid (no self-intersections)")
        return True

# Test various polygon types
test_cases = [
    # Valid simple polygon
    zlayout.Polygon([(0, 0), (4, 0), (4, 3), (0, 3)]),
    
    # Self-intersecting bowtie
    zlayout.Polygon([(0, 0), (2, 2), (2, 0), (0, 2)]),
    
    # Complex valid polygon
    zlayout.Polygon([(0, 0), (3, 0), (4, 1), (3, 3), (1, 4), (0, 2)]),
    
    # Figure-8 shape (self-intersecting)
    zlayout.Polygon([(0, 1), (1, 2), (2, 1), (3, 2), (4, 1), (3, 0), (2, 1), (1, 0)])
]

for i, polygon in enumerate(test_cases):
    print(f"\n=== Test Case {i+1} ===")
    is_valid = validate_polygon_integrity(polygon)
```

---

## Real-world Applications

### 1. Layout Verification

```python
class LayoutVerifier:
    def __init__(self):
        self.detector = EdgeIntersectionDetector()
    
    def comprehensive_intersection_check(self, layout):
        """Perform comprehensive intersection analysis."""
        
        report = {
            'self_intersections': [],
            'component_overlaps': [],
            'critical_intersections': [],
            'total_intersections': 0
        }
        
        # Get all intersections
        all_intersections = self.detector.detect_intersections(
            layout.components, include_touching=True
        )
        
        report['total_intersections'] = len(all_intersections)
        
        for intersection in all_intersections:
            # Categorize intersection type
            if intersection['polygon1_id'] == intersection['polygon2_id']:
                # Self-intersection
                report['self_intersections'].append(intersection)
            else:
                # Component overlap
                component1 = layout.components[intersection['polygon1_id']]
                component2 = layout.components[intersection['polygon2_id']]
                
                overlap_info = {
                    'intersection': intersection,
                    'component1_name': component1.name,
                    'component2_name': component2.name,
                    'component1_layer': component1.layer,
                    'component2_layer': component2.layer
                }
                
                # Check if it's a critical intersection
                if (component1.layer == component2.layer and 
                    intersection['is_proper']):
                    report['critical_intersections'].append(overlap_info)
                else:
                    report['component_overlaps'].append(overlap_info)
        
        return report
    
    def generate_fix_suggestions(self, intersection_report):
        """Generate suggestions for fixing intersections."""
        suggestions = []
        
        # Self-intersection fixes
        for self_inter in intersection_report['self_intersections']:
            suggestions.append({
                'type': 'self_intersection',
                'severity': 'critical',
                'message': f"Polygon has self-intersecting edges. Consider simplifying geometry.",
                'location': self_inter['point']
            })
        
        # Critical overlap fixes
        for critical in intersection_report['critical_intersections']:
            suggestions.append({
                'type': 'component_overlap',
                'severity': 'high', 
                'message': f"Components '{critical['component1_name']}' and "
                          f"'{critical['component2_name']}' overlap on same layer",
                'suggestion': "Adjust component positions or modify routing"
            })
        
        return suggestions
```

### 2. Manufacturing Constraint Checking

```python
def check_manufacturing_constraints(layout, process_rules):
    """Check for intersections that violate manufacturing rules."""
    
    violations = []
    intersections = detector.detect_intersections(layout.components)
    
    for intersection in intersections:
        comp1 = layout.components[intersection['polygon1_id']]
        comp2 = layout.components[intersection['polygon2_id']]
        
        # Check same-layer intersections
        if comp1.layer == comp2.layer:
            layer_rules = process_rules.get(comp1.layer, {})
            
            if intersection['is_proper']:
                # Proper intersection on same layer is always violation
                violations.append({
                    'type': 'same_layer_overlap',
                    'severity': 'error',
                    'layer': comp1.layer,
                    'location': intersection['point'],
                    'components': [comp1.name, comp2.name]
                })
            
        # Check inter-layer violations
        else:
            # Some layer combinations are not allowed to intersect
            forbidden_combinations = process_rules.get('forbidden_intersections', [])
            layer_pair = tuple(sorted([comp1.layer, comp2.layer]))
            
            if layer_pair in forbidden_combinations:
                violations.append({
                    'type': 'forbidden_layer_intersection',
                    'severity': 'warning',
                    'layers': layer_pair,
                    'location': intersection['point']
                })
    
    return violations
```

---

## Advanced Optimizations

### Bentley-Ottmann Algorithm

```python
class BentleyOttmannDetector:
    """Optimal O((n+k) log n) intersection detection."""
    
    def detect_intersections_optimal(self, polygons):
        """Bentley-Ottmann sweep line algorithm."""
        
        events = []
        edges = []
        
        # Create initial events
        for poly_id, polygon in enumerate(polygons):
            poly_edges = polygon.edges
            for edge_id, (start, end) in enumerate(poly_edges):
                # Ensure left-to-right ordering
                if start[0] > end[0] or (start[0] == end[0] and start[1] > end[1]):
                    start, end = end, start
                
                edge_data = {
                    'id': len(edges),
                    'polygon_id': poly_id,
                    'edge_id': edge_id,
                    'start': start,
                    'end': end
                }
                edges.append(edge_data)
                
                events.append(Event(start[0], 'start', len(edges) - 1))
                events.append(Event(end[0], 'end', len(edges) - 1))
        
        # Sort events
        events.sort(key=lambda e: (e.x, e.type == 'end'))
        
        # Sweep line state
        active_edges = BalancedBST()  # Y-ordered active edges
        intersections = []
        
        for event in events:
            if event.type == 'start':
                edge = edges[event.edge_id]
                
                # Insert edge and find neighbors
                node = active_edges.insert(edge)
                
                # Check intersections with neighbors
                if node.predecessor:
                    self._check_intersection(
                        edges[node.predecessor.edge_id], edge, intersections
                    )
                if node.successor:
                    self._check_intersection(
                        edge, edges[node.successor.edge_id], intersections
                    )
                    
            elif event.type == 'end':
                edge = edges[event.edge_id]
                node = active_edges.find(edge)
                
                # Check if predecessor and successor now intersect
                if node.predecessor and node.successor:
                    self._check_intersection(
                        edges[node.predecessor.edge_id],
                        edges[node.successor.edge_id], 
                        intersections
                    )
                
                active_edges.delete(node)
        
        return intersections
```

---

## Performance Summary

Edge intersection detection characteristics:

**Algorithm Selection:**
- **Brute Force**: Use for <100 edges total
- **Spatial Index**: Best general-purpose algorithm
- **Sweep Line**: Optimal for sparse intersections
- **Bentley-Ottmann**: Best theoretical complexity

**Performance Scaling:**
- Small datasets (<1K edges): All algorithms acceptable
- Medium datasets (1K-10K edges): Spatial indexing recommended
- Large datasets (>10K edges): Sweep line or Bentley-Ottmann

**Memory Requirements:**
- Brute force: O(1) working memory
- Spatial index: O(n) for index structure
- Sweep line: O(n) for event queue and active set

**Practical Considerations:**
1. Spatial indexing has best average-case performance
2. Sweep line excels when intersection density is low  
3. Numerical precision critical for robust results
4. Early termination possible for validation use cases

The algorithm is fundamental to geometric validation, enabling reliable detection of layout errors and ensuring geometric integrity in EDA workflows. 