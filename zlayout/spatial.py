"""
Spatial indexing structures for efficient geometric queries.
"""

from typing import List, Set, Optional, Union, Tuple, Any
from .geometry import Point, Rectangle, Polygon


class QuadTreeNode:
    """A node in the quadtree structure."""
    
    def __init__(self, boundary: Rectangle, capacity: int = 10, max_depth: int = 8):
        self.boundary = boundary
        self.capacity = capacity
        self.max_depth = max_depth
        self.objects: List[Tuple[Any, Rectangle]] = []  # (object, bounding_box) pairs
        self.divided = False
        self.children: List['QuadTreeNode'] = []
    
    def subdivide(self) -> None:
        """Divide this node into four quadrants."""
        if self.divided:
            return
        
        x, y = self.boundary.x, self.boundary.y
        w, h = self.boundary.width / 2, self.boundary.height / 2
        
        # Create four child nodes
        self.children = [
            QuadTreeNode(Rectangle(x, y + h, w, h), self.capacity, self.max_depth - 1),      # NW
            QuadTreeNode(Rectangle(x + w, y + h, w, h), self.capacity, self.max_depth - 1),  # NE
            QuadTreeNode(Rectangle(x, y, w, h), self.capacity, self.max_depth - 1),          # SW
            QuadTreeNode(Rectangle(x + w, y, w, h), self.capacity, self.max_depth - 1),      # SE
        ]
        self.divided = True
    
    def insert(self, obj: Any, bbox: Rectangle) -> bool:
        """Insert an object with its bounding box."""
        # Check if object fits in this boundary
        if not self._intersects_boundary(bbox):
            return False
        
        # If we have space and haven't subdivided, just add it
        if len(self.objects) < self.capacity and not self.divided:
            self.objects.append((obj, bbox))
            return True
        
        # If we can still subdivide, do so
        if not self.divided and self.max_depth > 0:
            self.subdivide()
        
        # Try to insert into children
        if self.divided:
            for child in self.children:
                if child.insert(obj, bbox):
                    return True
            # If no child could contain it, store it at this level
            self.objects.append((obj, bbox))
            return True
        else:
            # No more subdivisions possible, store here
            self.objects.append((obj, bbox))
            return True
    
    def query_range(self, range_bbox: Rectangle) -> List[Any]:
        """Query all objects that intersect with the given range."""
        result = []
        
        # Check if this node intersects with query range
        if not self.boundary.intersects(range_bbox):
            return result
        
        # Check objects in this node
        for obj, bbox in self.objects:
            if bbox.intersects(range_bbox):
                result.append(obj)
        
        # Recursively check children
        if self.divided:
            for child in self.children:
                result.extend(child.query_range(range_bbox))
        
        return result
    
    def query_point(self, point: Point) -> List[Any]:
        """Query all objects that contain the given point."""
        result = []
        
        # Check if point is in this boundary
        if not self.boundary.contains_point(point):
            return result
        
        # Check objects in this node
        for obj, bbox in self.objects:
            if bbox.contains_point(point):
                result.append(obj)
        
        # Recursively check children
        if self.divided:
            for child in self.children:
                result.extend(child.query_point(point))
        
        return result
    
    def _intersects_boundary(self, bbox: Rectangle) -> bool:
        """Check if bounding box intersects with this node's boundary."""
        return self.boundary.intersects(bbox)
    
    def get_all_objects(self) -> List[Any]:
        """Get all objects in this subtree."""
        result = [obj for obj, _ in self.objects]
        
        if self.divided:
            for child in self.children:
                result.extend(child.get_all_objects())
        
        return result
    
    def size(self) -> int:
        """Get total number of objects in this subtree."""
        count = len(self.objects)
        if self.divided:
            for child in self.children:
                count += child.size()
        return count


class QuadTree:
    """Quadtree spatial index for efficient range and intersection queries."""
    
    def __init__(self, boundary: Rectangle, capacity: int = 10, max_depth: int = 8):
        self.root = QuadTreeNode(boundary, capacity, max_depth)
        self.object_count = 0
    
    def insert(self, obj: Any, bbox: Optional[Rectangle] = None) -> bool:
        """Insert an object. If bbox is not provided, try to get it from the object."""
        if bbox is None:
            if hasattr(obj, 'bounding_box'):
                bbox = obj.bounding_box()
            elif hasattr(obj, 'boundary'):
                bbox = obj.boundary
            elif isinstance(obj, Rectangle):
                bbox = obj
            else:
                raise ValueError("Cannot determine bounding box for object")
        
        # bbox should now be guaranteed to be a Rectangle
        if bbox is None:
            raise ValueError("Could not determine bounding box")
            
        success = self.root.insert(obj, bbox)
        if success:
            self.object_count += 1
        return success
    
    def query_range(self, range_bbox: Rectangle) -> List[Any]:
        """Find all objects that intersect with the given range."""
        return self.root.query_range(range_bbox)
    
    def query_point(self, point: Point) -> List[Any]:
        """Find all objects that contain the given point."""
        return self.root.query_point(point)
    
    def find_intersections(self) -> List[Tuple[Any, Any]]:
        """Find all pairs of objects that potentially intersect."""
        intersections = []
        all_objects = [(obj, bbox) for obj, bbox in self._get_all_object_bbox_pairs()]
        
        # Use spatial indexing to reduce comparison count
        for i, (obj1, bbox1) in enumerate(all_objects):
            # Query objects that intersect with obj1's bounding box
            candidates = self.query_range(bbox1)
            
            for obj2 in candidates:
                if obj1 != obj2 and id(obj1) < id(obj2):  # Avoid duplicates
                    intersections.append((obj1, obj2))
        
        return intersections
    
    def find_nearby_objects(self, obj: Any, distance: float) -> List[Any]:
        """Find objects within a certain distance of the given object."""
        if hasattr(obj, 'bounding_box'):
            bbox = obj.bounding_box()
        elif isinstance(obj, Rectangle):
            bbox = obj
        else:
            raise ValueError("Cannot determine bounding box for object")
        
        # Expand bounding box by distance
        expanded_bbox = Rectangle(
            bbox.x - distance,
            bbox.y - distance,
            bbox.width + 2 * distance,
            bbox.height + 2 * distance
        )
        
        candidates = self.query_range(expanded_bbox)
        return [candidate for candidate in candidates if candidate != obj]
    
    def _get_all_object_bbox_pairs(self) -> List[Tuple[Any, Rectangle]]:
        """Get all (object, bbox) pairs from the tree."""
        return self._collect_object_bbox_pairs(self.root)
    
    def _collect_object_bbox_pairs(self, node: QuadTreeNode) -> List[Tuple[Any, Rectangle]]:
        """Recursively collect all object-bbox pairs."""
        pairs = node.objects.copy()
        
        if node.divided:
            for child in node.children:
                pairs.extend(self._collect_object_bbox_pairs(child))
        
        return pairs
    
    def size(self) -> int:
        """Get total number of objects in the tree."""
        return self.object_count
    
    def clear(self) -> None:
        """Remove all objects from the tree."""
        boundary = self.root.boundary
        capacity = self.root.capacity
        max_depth = self.root.max_depth
        self.root = QuadTreeNode(boundary, capacity, max_depth)
        self.object_count = 0


class SpatialIndex:
    """High-level spatial indexing interface."""
    
    def __init__(self, world_bounds: Rectangle, capacity: int = 10, max_depth: int = 8):
        self.quadtree = QuadTree(world_bounds, capacity, max_depth)
        self.objects = {}  # object_id -> object mapping
        self._next_id = 0
    
    def add_polygon(self, polygon: Polygon) -> int:
        """Add a polygon to the spatial index."""
        obj_id = self._next_id
        self._next_id += 1
        
        self.objects[obj_id] = polygon
        self.quadtree.insert(polygon)
        return obj_id
    
    def add_rectangle(self, rectangle: Rectangle) -> int:
        """Add a rectangle to the spatial index."""
        obj_id = self._next_id
        self._next_id += 1
        
        self.objects[obj_id] = rectangle
        self.quadtree.insert(rectangle)
        return obj_id
    
    def remove_object(self, obj_id: int) -> bool:
        """Remove an object by ID. Note: QuadTree doesn't support efficient removal."""
        if obj_id in self.objects:
            del self.objects[obj_id]
            # For efficient removal, we'd need to rebuild the tree
            # For now, just mark as removed
            return True
        return False
    
    def find_intersecting_edges(self) -> List[Tuple[int, int]]:
        """Find all pairs of polygons with potentially intersecting edges."""
        intersections = []
        all_polygons = [(obj_id, obj) for obj_id, obj in self.objects.items() 
                       if isinstance(obj, Polygon)]
        
        for i, (id1, poly1) in enumerate(all_polygons):
            bbox1 = poly1.bounding_box()
            candidates = self.quadtree.query_range(bbox1)
            
            for candidate in candidates:
                if isinstance(candidate, Polygon):
                    # Find the ID of the candidate
                    id2 = None
                    for oid, obj in self.objects.items():
                        if obj is candidate:
                            id2 = oid
                            break
                    
                    if id2 is not None and id1 < id2:  # Avoid duplicates
                        intersections.append((id1, id2))
        
        return intersections
    
    def find_nearby_objects(self, obj_id: int, distance: float) -> List[int]:
        """Find objects within distance of the given object."""
        if obj_id not in self.objects:
            return []
        
        obj = self.objects[obj_id]
        nearby = self.quadtree.find_nearby_objects(obj, distance)
        
        # Convert objects back to IDs
        nearby_ids = []
        for nearby_obj in nearby:
            for oid, stored_obj in self.objects.items():
                if stored_obj is nearby_obj:
                    nearby_ids.append(oid)
                    break
        
        return nearby_ids
    
    def query_region(self, region: Rectangle) -> List[int]:
        """Find all objects that intersect with the given region."""
        objects_in_region = self.quadtree.query_range(region)
        
        # Convert objects back to IDs
        object_ids = []
        for obj in objects_in_region:
            for oid, stored_obj in self.objects.items():
                if stored_obj is obj:
                    object_ids.append(oid)
                    break
        
        return object_ids