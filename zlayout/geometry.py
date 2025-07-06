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
        # Show integers as integers, floats as floats
        x_str = str(int(self.x)) if self.x == int(self.x) else str(self.x)
        y_str = str(int(self.y)) if self.y == int(self.y) else str(self.y)
        return f"Point({x_str}, {y_str})"
    
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
        
        # First check if point is on an edge
        for i in range(n):
            v1 = self.vertices[i]
            v2 = self.vertices[(i + 1) % n]
            
            # Check if point is on the edge
            if self._point_on_edge(point, v1, v2):
                return True
        
        # Ray casting algorithm
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
    
    def _point_on_edge(self, point: Point, edge_start: Point, edge_end: Point) -> bool:
        """Check if a point lies on an edge."""
        # Use distance to line segment
        distance = point.distance_to_line(edge_start, edge_end)
        return distance < 1e-10
    
    def get_sharp_angles(self, threshold_degrees: float = 30.0) -> List[int]:
        """Find vertices with sharp angles (less than threshold)."""
        def calculate_interior_angle(prev_pt: Point, curr_pt: Point, next_pt: Point) -> float:
            """Calculate the interior angle at curr_pt."""
            # Vectors from current point to adjacent points  
            v1 = Point(prev_pt.x - curr_pt.x, prev_pt.y - curr_pt.y)
            v2 = Point(next_pt.x - curr_pt.x, next_pt.y - curr_pt.y)
            
            # Calculate angle using atan2 for better numerical stability
            angle1 = math.atan2(v1.y, v1.x)
            angle2 = math.atan2(v2.y, v2.x)
            
            # Calculate interior angle
            angle_diff = angle2 - angle1
            
            # Normalize to [0, 2π]
            while angle_diff < 0:
                angle_diff += 2 * math.pi
            while angle_diff > 2 * math.pi:
                angle_diff -= 2 * math.pi
                
            # Convert to degrees
            interior_angle = math.degrees(angle_diff)
            
            # For convex polygons in counter-clockwise order, interior angles should be < 180°
            # For clockwise order, we need to take the complement
            if interior_angle > 180:
                interior_angle = 360 - interior_angle
                
            return interior_angle
        
        sharp_angles = []
        n = len(self.vertices)
        
        for i in range(n):
            prev_vertex = self.vertices[(i - 1) % n]
            curr_vertex = self.vertices[i]
            next_vertex = self.vertices[(i + 1) % n]
            
            interior_angle = calculate_interior_angle(prev_vertex, curr_vertex, next_vertex)
            
            # Check if angle is sharp (less than threshold)
            if interior_angle < threshold_degrees:
                sharp_angles.append(i)
        
        return sharp_angles