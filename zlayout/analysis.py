"""
Advanced geometric analysis algorithms for layout optimization.
"""

import math
from typing import List, Tuple, Set, Dict, Optional, Union
from .geometry import Point, Rectangle, Polygon
from .spatial import QuadTree, SpatialIndex


class EdgeIntersectionResult:
    """Result of edge intersection analysis."""
    
    def __init__(self):
        self.intersecting_pairs: List[Tuple[int, int]] = []  # (polygon_id1, polygon_id2)
        self.intersection_points: List[Point] = []
        self.total_intersections = 0


class NarrowDistanceResult:
    """Result of narrow distance analysis."""
    
    def __init__(self):
        self.narrow_regions: List[Tuple[Point, Point, float]] = []  # (point1, point2, distance)
        self.min_distance = float('inf')
        self.max_distance = 0.0
        self.average_distance = 0.0


class SharpAngleResult:
    """Result of sharp angle analysis."""
    
    def __init__(self):
        self.sharp_angles: List[Tuple[int, int, float]] = []  # (polygon_id, vertex_index, angle_degrees)
        self.sharpest_angle = 180.0
        self.average_angle = 90.0


def line_intersection(p1: Point, p2: Point, p3: Point, p4: Point) -> Optional[Point]:
    """
    Find intersection point of two line segments.
    Returns None if lines don't intersect.
    """
    # Line 1: p1 to p2
    # Line 2: p3 to p4
    
    denom = (p1.x - p2.x) * (p3.y - p4.y) - (p1.y - p2.y) * (p3.x - p4.x)
    
    if abs(denom) < 1e-10:  # Lines are parallel
        return None
    
    t = ((p1.x - p3.x) * (p3.y - p4.y) - (p1.y - p3.y) * (p3.x - p4.x)) / denom
    u = -((p1.x - p2.x) * (p1.y - p3.y) - (p1.y - p2.y) * (p1.x - p3.x)) / denom
    
    # Check if intersection is within both line segments
    if 0 <= t <= 1 and 0 <= u <= 1:
        x = p1.x + t * (p2.x - p1.x)
        y = p1.y + t * (p2.y - p1.y)
        return Point(x, y)
    
    return None


def segments_intersect(p1: Point, p2: Point, p3: Point, p4: Point) -> bool:
    """
    Fast check if two line segments intersect using orientation method.
    This is more efficient than computing actual intersection point.
    """
    def orientation(p: Point, q: Point, r: Point) -> int:
        """Find orientation of ordered triplet (p, q, r).
        Returns:
        0 -> p, q and r are colinear
        1 -> Clockwise
        2 -> Counterclockwise
        """
        val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y)
        if abs(val) < 1e-10:
            return 0
        return 1 if val > 0 else 2
    
    def on_segment(p: Point, q: Point, r: Point) -> bool:
        """Check if point q lies on segment pr."""
        return (q.x <= max(p.x, r.x) and q.x >= min(p.x, r.x) and
                q.y <= max(p.y, r.y) and q.y >= min(p.y, r.y))
    
    o1 = orientation(p1, p2, p3)
    o2 = orientation(p1, p2, p4)
    o3 = orientation(p3, p4, p1)
    o4 = orientation(p3, p4, p2)
    
    # General case
    if o1 != o2 and o3 != o4:
        return True
    
    # Special cases for collinear points
    if (o1 == 0 and on_segment(p1, p3, p2) or
        o2 == 0 and on_segment(p1, p4, p2) or
        o3 == 0 and on_segment(p3, p1, p4) or
        o4 == 0 and on_segment(p3, p2, p4)):
        return True
    
    return False


class PolygonAnalyzer:
    """Analyzer for polygon geometric properties and relationships."""
    
    def __init__(self, spatial_index: Optional[SpatialIndex] = None):
        self.spatial_index = spatial_index
        self.polygons: List[Polygon] = []
        self.polygon_ids: List[int] = []
    
    def add_polygon(self, polygon: Polygon) -> int:
        """Add a polygon for analysis."""
        if self.spatial_index:
            poly_id = self.spatial_index.add_polygon(polygon)
        else:
            poly_id = len(self.polygons)
            self.polygons.append(polygon)
            self.polygon_ids.append(poly_id)
        return poly_id
    
    def find_sharp_angles(self, threshold_degrees: float = 30.0) -> SharpAngleResult:
        """Find all sharp angles in all polygons."""
        result = SharpAngleResult()
        
        polygons_to_analyze = []
        if self.spatial_index:
            for obj_id, obj in self.spatial_index.objects.items():
                if isinstance(obj, Polygon):
                    polygons_to_analyze.append((obj_id, obj))
        else:
            for i, polygon in enumerate(self.polygons):
                polygons_to_analyze.append((i, polygon))
        
        total_angles = 0
        angle_sum = 0.0
        
        for poly_id, polygon in polygons_to_analyze:
            sharp_vertices = polygon.get_sharp_angles(threshold_degrees)
            
            # Calculate actual angles for sharp vertices
            for vertex_idx in sharp_vertices:
                n = len(polygon.vertices)
                prev_vertex = polygon.vertices[(vertex_idx - 1) % n]
                curr_vertex = polygon.vertices[vertex_idx]
                next_vertex = polygon.vertices[(vertex_idx + 1) % n]
                
                # Calculate angle
                v1 = Point(prev_vertex.x - curr_vertex.x, prev_vertex.y - curr_vertex.y)
                v2 = Point(next_vertex.x - curr_vertex.x, next_vertex.y - curr_vertex.y)
                
                dot_product = v1.x * v2.x + v1.y * v2.y
                mag1 = math.sqrt(v1.x**2 + v1.y**2)
                mag2 = math.sqrt(v2.x**2 + v2.y**2)
                
                if mag1 > 1e-10 and mag2 > 1e-10:
                    cos_angle = dot_product / (mag1 * mag2)
                    cos_angle = max(-1.0, min(1.0, cos_angle))
                    angle_degrees = math.degrees(math.acos(cos_angle))
                    
                    result.sharp_angles.append((poly_id, vertex_idx, angle_degrees))
                    result.sharpest_angle = min(result.sharpest_angle, angle_degrees)
                    angle_sum += angle_degrees
                    total_angles += 1
        
        if total_angles > 0:
            result.average_angle = angle_sum / total_angles
        
        return result
    
    def find_narrow_distances(self, threshold_distance: float = 1.0) -> NarrowDistanceResult:
        """Find regions where polygon edges are too close together."""
        result = NarrowDistanceResult()
        
        polygons_to_analyze = []
        if self.spatial_index:
            for obj_id, obj in self.spatial_index.objects.items():
                if isinstance(obj, Polygon):
                    polygons_to_analyze.append((obj_id, obj))
        else:
            for i, polygon in enumerate(self.polygons):
                polygons_to_analyze.append((i, polygon))
        
        distances = []
        
        # Check distances between edges of different polygons
        for i, (id1, poly1) in enumerate(polygons_to_analyze):
            for j, (id2, poly2) in enumerate(polygons_to_analyze[i+1:], i+1):
                # Check all edge pairs between the two polygons
                for edge1 in poly1.edges:
                    for edge2 in poly2.edges:
                        dist = self._edge_to_edge_distance(edge1[0], edge1[1], edge2[0], edge2[1])
                        distances.append(dist)
                        
                        if dist < threshold_distance:
                            # Find closest points on the edges
                            closest_points = self._closest_points_on_edges(
                                edge1[0], edge1[1], edge2[0], edge2[1]
                            )
                            result.narrow_regions.append((closest_points[0], closest_points[1], dist))
        
        # Also check within same polygon (self-intersection prevention)
        for poly_id, polygon in polygons_to_analyze:
            edges = polygon.edges
            for i, edge1 in enumerate(edges):
                for j, edge2 in enumerate(edges[i+2:], i+2):  # Skip adjacent edges
                    if j == len(edges) - 1 and i == 0:  # Skip last-first edge pair
                        continue
                        
                    dist = self._edge_to_edge_distance(edge1[0], edge1[1], edge2[0], edge2[1])
                    distances.append(dist)
                    
                    if dist < threshold_distance:
                        closest_points = self._closest_points_on_edges(
                            edge1[0], edge1[1], edge2[0], edge2[1]
                        )
                        result.narrow_regions.append((closest_points[0], closest_points[1], dist))
        
        if distances:
            result.min_distance = min(distances)
            result.max_distance = max(distances)
            result.average_distance = sum(distances) / len(distances)
        
        return result
    
    def find_edge_intersections(self) -> EdgeIntersectionResult:
        """Find all edge intersections using spatial indexing for efficiency."""
        result = EdgeIntersectionResult()
        
        if self.spatial_index:
            # Use spatial indexing for efficiency
            intersecting_pairs = self.spatial_index.find_intersecting_edges()
            
            for id1, id2 in intersecting_pairs:
                poly1 = self.spatial_index.objects[id1]
                poly2 = self.spatial_index.objects[id2]
                
                if isinstance(poly1, Polygon) and isinstance(poly2, Polygon):
                    intersections = self._find_polygon_edge_intersections(poly1, poly2)
                    if intersections:
                        result.intersecting_pairs.append((id1, id2))
                        result.intersection_points.extend(intersections)
                        result.total_intersections += len(intersections)
        else:
            # Brute force approach for small datasets
            for i, poly1 in enumerate(self.polygons):
                for j, poly2 in enumerate(self.polygons[i+1:], i+1):
                    intersections = self._find_polygon_edge_intersections(poly1, poly2)
                    if intersections:
                        result.intersecting_pairs.append((i, j))
                        result.intersection_points.extend(intersections)
                        result.total_intersections += len(intersections)
        
        return result
    
    def _find_polygon_edge_intersections(self, poly1: Polygon, poly2: Polygon) -> List[Point]:
        """Find all intersection points between edges of two polygons."""
        intersections = []
        
        for edge1 in poly1.edges:
            for edge2 in poly2.edges:
                intersection = line_intersection(edge1[0], edge1[1], edge2[0], edge2[1])
                if intersection:
                    intersections.append(intersection)
        
        return intersections
    
    def _edge_to_edge_distance(self, p1: Point, p2: Point, p3: Point, p4: Point) -> float:
        """Calculate minimum distance between two line segments."""
        # Try all point-to-line distances
        distances = [
            p1.distance_to_line(p3, p4),
            p2.distance_to_line(p3, p4),
            p3.distance_to_line(p1, p2),
            p4.distance_to_line(p1, p2)
        ]
        
        # Also check endpoint-to-endpoint distances
        distances.extend([
            p1.distance_to(p3), p1.distance_to(p4),
            p2.distance_to(p3), p2.distance_to(p4)
        ])
        
        return min(distances)
    
    def _closest_points_on_edges(self, p1: Point, p2: Point, p3: Point, p4: Point) -> Tuple[Point, Point]:
        """Find the closest points on two line segments."""
        # For simplicity, return the endpoints that give minimum distance
        min_dist = float('inf')
        closest_pair = (p1, p3)
        
        candidates = [
            (p1, p3), (p1, p4), (p2, p3), (p2, p4)
        ]
        
        for pt1, pt2 in candidates:
            dist = pt1.distance_to(pt2)
            if dist < min_dist:
                min_dist = dist
                closest_pair = (pt1, pt2)
        
        return closest_pair


class GeometryProcessor:
    """High-level geometry processing and optimization."""
    
    def __init__(self, world_bounds: Rectangle):
        self.spatial_index = SpatialIndex(world_bounds)
        self.analyzer = PolygonAnalyzer(self.spatial_index)
    
    def add_component(self, geometry: Union[Rectangle, Polygon]) -> int:
        """Add a geometric component to the processor."""
        if isinstance(geometry, Rectangle):
            return self.spatial_index.add_rectangle(geometry)
        elif isinstance(geometry, Polygon):
            return self.analyzer.add_polygon(geometry)
        else:
            raise ValueError("Unsupported geometry type")
    
    def analyze_layout(self, sharp_angle_threshold: float = 30.0, 
                      narrow_distance_threshold: float = 1.0) -> Dict:
        """Perform comprehensive layout analysis."""
        results = {}
        
        # Analyze sharp angles
        sharp_angles = self.analyzer.find_sharp_angles(sharp_angle_threshold)
        results['sharp_angles'] = {
            'count': len(sharp_angles.sharp_angles),
            'sharpest': sharp_angles.sharpest_angle,
            'average': sharp_angles.average_angle,
            'details': sharp_angles.sharp_angles
        }
        
        # Analyze narrow distances
        narrow_distances = self.analyzer.find_narrow_distances(narrow_distance_threshold)
        results['narrow_distances'] = {
            'count': len(narrow_distances.narrow_regions),
            'minimum': narrow_distances.min_distance,
            'maximum': narrow_distances.max_distance,
            'average': narrow_distances.average_distance,
            'details': narrow_distances.narrow_regions
        }
        
        # Analyze edge intersections
        intersections = self.analyzer.find_edge_intersections()
        results['intersections'] = {
            'polygon_pairs': len(intersections.intersecting_pairs),
            'total_points': intersections.total_intersections,
            'pairs': intersections.intersecting_pairs,
            'points': intersections.intersection_points
        }
        
        return results
    
    def optimize_layout(self) -> Dict:
        """Suggest layout optimizations based on analysis."""
        analysis = self.analyze_layout()
        suggestions = []
        
        if analysis['sharp_angles']['count'] > 0:
            suggestions.append(f"Found {analysis['sharp_angles']['count']} sharp angles. "
                             f"Consider rounding corners or adjusting geometry.")
        
        if analysis['narrow_distances']['count'] > 0:
            suggestions.append(f"Found {analysis['narrow_distances']['count']} narrow regions. "
                             f"Minimum distance: {analysis['narrow_distances']['minimum']:.3f}")
        
        if analysis['intersections']['polygon_pairs'] > 0:
            suggestions.append(f"Found {analysis['intersections']['polygon_pairs']} intersecting polygon pairs. "
                             f"Total intersection points: {analysis['intersections']['total_points']}")
        
        return {
            'analysis': analysis,
            'suggestions': suggestions,
            'optimization_score': self._calculate_optimization_score(analysis)
        }
    
    def _calculate_optimization_score(self, analysis: Dict) -> float:
        """Calculate a score representing layout quality (0-100, higher is better)."""
        score = 100.0
        
        # Penalty for sharp angles
        sharp_penalty = min(analysis['sharp_angles']['count'] * 5, 30)
        score -= sharp_penalty
        
        # Penalty for narrow distances
        narrow_penalty = min(analysis['narrow_distances']['count'] * 10, 40)
        score -= narrow_penalty
        
        # Heavy penalty for intersections
        intersection_penalty = min(analysis['intersections']['polygon_pairs'] * 20, 50)
        score -= intersection_penalty
        
        return max(0.0, score)