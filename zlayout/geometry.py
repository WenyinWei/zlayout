"""
Basic geometric data structures for layout processing.
"""

import math
from typing import List, Tuple, Optional, Union

# Optional numpy import - fallback to built-in types if not available
try:
    import numpy as np
    HAS_NUMPY = True
except ImportError:
    HAS_NUMPY = False


class Point:
    """2D point with coordinates and utility methods."""
    
    def __init__(self, x: float, y: float):
        self.x = float(x)
        self.y = float(y)
    
    def __repr__(self) -> str:
        return f"Point({self.x}, {self.y})"
    
    def __eq__(self, other) -> bool:
        if not isinstance(other, Point):
            return False
        return abs(self.x - other.x) < 1e-10 and abs(self.y - other.y) < 1e-10
    
    def __hash__(self) -> int:
        return hash((round(self.x, 10), round(self.y, 10)))
    
    def distance_to(self, other: 'Point') -> float:
        """Calculate Euclidean distance to another point."""
        return math.sqrt((self.x - other.x)**2 + (self.y - other.y)**2)
    
    def distance_to_line(self, line_start: 'Point', line_end: 'Point') -> float:
        """Calculate distance from this point to a line segment."""
        # Vector from line_start to line_end
        line_vec = Point(line_end.x - line_start.x, line_end.y - line_start.y)
        line_length_sq = line_vec.x**2 + line_vec.y**2
        
        if line_length_sq < 1e-10:  # Degenerate line
            return self.distance_to(line_start)
        
        # Vector from line_start to this point
        point_vec = Point(self.x - line_start.x, self.y - line_start.y)
        
        # Project point onto line
        t = max(0, min(1, (point_vec.x * line_vec.x + point_vec.y * line_vec.y) / line_length_sq))
        
        # Find closest point on line segment
        closest = Point(line_start.x + t * line_vec.x, line_start.y + t * line_vec.y)
        
        return self.distance_to(closest)
    
    def to_numpy(self):
        """Convert to numpy array (if numpy is available)."""
        if HAS_NUMPY:
            return np.array([self.x, self.y])
        else:
            return [self.x, self.y]


class Rectangle:
    """Axis-aligned rectangle for bounding boxes and simple components."""
    
    def __init__(self, x: float, y: float, width: float, height: float):
        self.x = float(x)
        self.y = float(y)
        self.width = float(width)
        self.height = float(height)
    
    def __repr__(self) -> str:
        return f"Rectangle({self.x}, {self.y}, {self.width}, {self.height})"
    
    @property
    def left(self) -> float:
        return self.x
    
    @property
    def right(self) -> float:
        return self.x + self.width
    
    @property
    def top(self) -> float:
        return self.y + self.height
    
    @property
    def bottom(self) -> float:
        return self.y
    
    @property
    def center(self) -> Point:
        return Point(self.x + self.width / 2, self.y + self.height / 2)
    
    def contains_point(self, point: Point) -> bool:
        """Check if point is inside rectangle."""
        return (self.left <= point.x <= self.right and 
                self.bottom <= point.y <= self.top)
    
    def intersects(self, other: 'Rectangle') -> bool:
        """Check if this rectangle intersects with another."""
        return not (self.right < other.left or other.right < self.left or
                   self.top < other.bottom or other.top < self.bottom)
    
    def to_polygon(self) -> 'Polygon':
        """Convert rectangle to polygon."""
        vertices = [
            Point(self.left, self.bottom),
            Point(self.right, self.bottom),
            Point(self.right, self.top),
            Point(self.left, self.top)
        ]
        return Polygon(vertices)


class Polygon:
    """Polygon class supporting both convex and concave polygons."""
    
    def __init__(self, vertices: List[Point]):
        if len(vertices) < 3:
            raise ValueError("Polygon must have at least 3 vertices")
        self.vertices = vertices.copy()
    
    def __repr__(self) -> str:
        return f"Polygon({len(self.vertices)} vertices)"
    
    @property
    def edges(self) -> List[Tuple[Point, Point]]:
        """Get all edges as (start, end) point pairs."""
        edges = []
        n = len(self.vertices)
        for i in range(n):
            edges.append((self.vertices[i], self.vertices[(i + 1) % n]))
        return edges
    
    def bounding_box(self) -> Rectangle:
        """Calculate axis-aligned bounding box."""
        if not self.vertices:
            raise ValueError("Cannot calculate bounding box of empty polygon")
        
        min_x = min(v.x for v in self.vertices)
        max_x = max(v.x for v in self.vertices)
        min_y = min(v.y for v in self.vertices)
        max_y = max(v.y for v in self.vertices)
        
        return Rectangle(min_x, min_y, max_x - min_x, max_y - min_y)
    
    def area(self) -> float:
        """Calculate polygon area using shoelace formula."""
        if len(self.vertices) < 3:
            return 0.0
        
        area = 0.0
        n = len(self.vertices)
        for i in range(n):
            j = (i + 1) % n
            area += self.vertices[i].x * self.vertices[j].y
            area -= self.vertices[j].x * self.vertices[i].y
        return abs(area) / 2.0
    
    def is_convex(self) -> bool:
        """Check if polygon is convex."""
        if len(self.vertices) < 3:
            return True
        
        def cross_product(o: Point, a: Point, b: Point) -> float:
            return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x)
        
        n = len(self.vertices)
        sign = None
        
        for i in range(n):
            cp = cross_product(
                self.vertices[i],
                self.vertices[(i + 1) % n],
                self.vertices[(i + 2) % n]
            )
            
            if abs(cp) > 1e-10:  # Not collinear
                if sign is None:
                    sign = cp > 0
                elif (cp > 0) != sign:
                    return False
        
        return True
    
    def contains_point(self, point: Point) -> bool:
        """Check if point is inside polygon using ray casting."""
        x, y = point.x, point.y
        n = len(self.vertices)
        inside = False
        
        p1x, p1y = self.vertices[0].x, self.vertices[0].y
        for i in range(1, n + 1):
            p2x, p2y = self.vertices[i % n].x, self.vertices[i % n].y
            if y > min(p1y, p2y):
                if y <= max(p1y, p2y):
                    if x <= max(p1x, p2x):
                        if p1y != p2y:
                            xinters = (y - p1y) * (p2x - p1x) / (p2y - p1y) + p1x
                        if p1x == p2x or x <= xinters:
                            inside = not inside
            p1x, p1y = p2x, p2y
        
        return inside
    
    def get_sharp_angles(self, threshold_degrees: float = 30.0) -> List[int]:
        """Find vertices with sharp angles (less than threshold)."""
        def angle_between_vectors(v1: Point, v2: Point) -> float:
            """Calculate angle between two vectors in degrees."""
            dot_product = v1.x * v2.x + v1.y * v2.y
            mag1 = math.sqrt(v1.x**2 + v1.y**2)
            mag2 = math.sqrt(v2.x**2 + v2.y**2)
            
            if mag1 < 1e-10 or mag2 < 1e-10:
                return 0.0
            
            cos_angle = dot_product / (mag1 * mag2)
            cos_angle = max(-1.0, min(1.0, cos_angle))  # Clamp to [-1, 1]
            return math.degrees(math.acos(cos_angle))
        
        sharp_angles = []
        n = len(self.vertices)
        
        for i in range(n):
            prev_vertex = self.vertices[(i - 1) % n]
            curr_vertex = self.vertices[i]
            next_vertex = self.vertices[(i + 1) % n]
            
            # Vectors from current vertex to adjacent vertices
            v1 = Point(prev_vertex.x - curr_vertex.x, prev_vertex.y - curr_vertex.y)
            v2 = Point(next_vertex.x - curr_vertex.x, next_vertex.y - curr_vertex.y)
            
            angle = angle_between_vectors(v1, v2)
            
            # For interior angle, we need to consider polygon orientation
            if angle < threshold_degrees or angle > (180 - threshold_degrees):
                sharp_angles.append(i)
        
        return sharp_angles