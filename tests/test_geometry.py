#!/usr/bin/env python3
"""
Unit tests for zlayout.geometry module.
"""

import unittest
import math
import sys
import os

# Add parent directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

from zlayout.geometry import Point, Rectangle, Polygon


class TestPoint(unittest.TestCase):
    """Test cases for Point class."""
    
    def setUp(self):
        self.p1 = Point(0, 0)
        self.p2 = Point(3, 4)
        self.p3 = Point(-2, 1)
    
    def test_point_creation(self):
        """Test point creation and basic properties."""
        self.assertEqual(self.p1.x, 0)
        self.assertEqual(self.p1.y, 0)
        self.assertEqual(self.p2.x, 3)
        self.assertEqual(self.p2.y, 4)
    
    def test_point_equality(self):
        """Test point equality comparison."""
        p1_copy = Point(0, 0)
        self.assertEqual(self.p1, p1_copy)
        self.assertNotEqual(self.p1, self.p2)
        
        # Test floating point tolerance
        p_almost_same = Point(1e-11, 1e-11)
        p_zero = Point(0, 0)
        self.assertEqual(p_almost_same, p_zero)
    
    def test_distance_calculation(self):
        """Test distance calculation between points."""
        # Distance from origin to (3,4) should be 5
        distance = self.p1.distance_to(self.p2)
        self.assertAlmostEqual(distance, 5.0, places=10)
        
        # Distance to self should be 0
        self.assertEqual(self.p1.distance_to(self.p1), 0)
    
    def test_distance_to_line(self):
        """Test distance from point to line segment."""
        # Point on line should have distance 0
        line_start = Point(0, 0)
        line_end = Point(10, 0)
        point_on_line = Point(5, 0)
        
        distance = point_on_line.distance_to_line(line_start, line_end)
        self.assertAlmostEqual(distance, 0, places=10)
        
        # Point perpendicular to line
        point_above = Point(5, 3)
        distance = point_above.distance_to_line(line_start, line_end)
        self.assertAlmostEqual(distance, 3, places=10)
    
    def test_point_repr(self):
        """Test string representation."""
        self.assertEqual(repr(self.p1), "Point(0, 0)")
        self.assertEqual(repr(self.p2), "Point(3, 4)")


class TestRectangle(unittest.TestCase):
    """Test cases for Rectangle class."""
    
    def setUp(self):
        self.rect1 = Rectangle(0, 0, 10, 5)
        self.rect2 = Rectangle(5, 2, 8, 6)
        self.rect3 = Rectangle(20, 20, 5, 5)
    
    def _assert_vertices_equal(self, expected_vertices, actual_vertices):
        """Helper method to assert vertex equality without loops in test."""
        self.assertEqual(len(expected_vertices), len(actual_vertices))
        self.assertEqual(expected_vertices[0], actual_vertices[0])
        self.assertEqual(expected_vertices[1], actual_vertices[1])
        self.assertEqual(expected_vertices[2], actual_vertices[2])
        self.assertEqual(expected_vertices[3], actual_vertices[3])
    
    def test_rectangle_creation(self):
        """Test rectangle creation and properties."""
        self.assertEqual(self.rect1.x, 0)
        self.assertEqual(self.rect1.y, 0)
        self.assertEqual(self.rect1.width, 10)
        self.assertEqual(self.rect1.height, 5)
    
    def test_rectangle_boundaries(self):
        """Test rectangle boundary properties."""
        self.assertEqual(self.rect1.left, 0)
        self.assertEqual(self.rect1.right, 10)
        self.assertEqual(self.rect1.bottom, 0)
        self.assertEqual(self.rect1.top, 5)
        
        center = self.rect1.center
        self.assertEqual(center.x, 5)
        self.assertEqual(center.y, 2.5)
    
    def test_point_containment(self):
        """Test point containment in rectangle."""
        # Point inside
        inside_point = Point(5, 2.5)
        self.assertTrue(self.rect1.contains_point(inside_point))
        
        # Point outside
        outside_point = Point(15, 10)
        self.assertFalse(self.rect1.contains_point(outside_point))
        
        # Point on boundary
        boundary_point = Point(10, 2.5)
        self.assertTrue(self.rect1.contains_point(boundary_point))
    
    def test_rectangle_intersection(self):
        """Test rectangle intersection detection."""
        # Overlapping rectangles
        self.assertTrue(self.rect1.intersects(self.rect2))
        self.assertTrue(self.rect2.intersects(self.rect1))
        
        # Non-overlapping rectangles
        self.assertFalse(self.rect1.intersects(self.rect3))
        self.assertFalse(self.rect3.intersects(self.rect1))
    
    def test_to_polygon_conversion(self):
        """Test conversion to polygon."""
        polygon = self.rect1.to_polygon()
        self.assertIsInstance(polygon, Polygon)
        self.assertEqual(len(polygon.vertices), 4)
        
        # Check vertices are correct
        expected_vertices = [
            Point(0, 0), Point(10, 0), Point(10, 5), Point(0, 5)
        ]
        self._assert_vertices_equal(expected_vertices, polygon.vertices)


class TestPolygon(unittest.TestCase):
    """Test cases for Polygon class."""
    
    def setUp(self):
        # Triangle
        self.triangle = Polygon([
            Point(0, 0), Point(4, 0), Point(2, 3)
        ])
        
        # Square
        self.square = Polygon([
            Point(0, 0), Point(2, 0), Point(2, 2), Point(0, 2)
        ])
        
        # L-shaped polygon (concave)
        self.l_shape = Polygon([
            Point(0, 0), Point(3, 0), Point(3, 1),
            Point(1, 1), Point(1, 3), Point(0, 3)
        ])
    
    def _assert_edges_equal(self, expected_edges, actual_edges):
        """Helper method to assert edge equality without loops in test."""
        self.assertEqual(len(expected_edges), len(actual_edges))
        # Assert each edge individually
        self.assertEqual(expected_edges[0][0], actual_edges[0][0])
        self.assertEqual(expected_edges[0][1], actual_edges[0][1])
        self.assertEqual(expected_edges[1][0], actual_edges[1][0])
        self.assertEqual(expected_edges[1][1], actual_edges[1][1])
        self.assertEqual(expected_edges[2][0], actual_edges[2][0])
        self.assertEqual(expected_edges[2][1], actual_edges[2][1])
    
    def test_polygon_creation(self):
        """Test polygon creation."""
        self.assertEqual(len(self.triangle.vertices), 3)
        self.assertEqual(len(self.square.vertices), 4)
        
        # Should raise error for too few vertices
        with self.assertRaises(ValueError):
            Polygon([Point(0, 0), Point(1, 1)])
    
    def test_polygon_edges(self):
        """Test edge generation."""
        edges = self.triangle.edges
        self.assertEqual(len(edges), 3)
        
        # Check edge endpoints
        expected_edges = [
            (Point(0, 0), Point(4, 0)),
            (Point(4, 0), Point(2, 3)),
            (Point(2, 3), Point(0, 0))
        ]
        
        self._assert_edges_equal(expected_edges, edges)
    
    def test_bounding_box(self):
        """Test bounding box calculation."""
        bbox = self.triangle.bounding_box()
        
        self.assertEqual(bbox.x, 0)
        self.assertEqual(bbox.y, 0)
        self.assertEqual(bbox.width, 4)
        self.assertEqual(bbox.height, 3)
    
    def test_area_calculation(self):
        """Test polygon area calculation."""
        # Triangle area = 0.5 * base * height = 0.5 * 4 * 3 = 6
        triangle_area = self.triangle.area()
        self.assertAlmostEqual(triangle_area, 6.0, places=10)
        
        # Square area = 2 * 2 = 4
        square_area = self.square.area()
        self.assertAlmostEqual(square_area, 4.0, places=10)
    
    def test_convexity_check(self):
        """Test convexity detection."""
        self.assertTrue(self.triangle.is_convex())
        self.assertTrue(self.square.is_convex())
        self.assertFalse(self.l_shape.is_convex())
    
    def test_point_containment(self):
        """Test point-in-polygon detection."""
        # Point inside triangle
        inside_point = Point(2, 1)
        self.assertTrue(self.triangle.contains_point(inside_point))
        
        # Point outside triangle
        outside_point = Point(5, 5)
        self.assertFalse(self.triangle.contains_point(outside_point))
        
        # Point on edge (this can be tricky)
        edge_point = Point(2, 0)
        self.assertTrue(self.triangle.contains_point(edge_point))
    
    def test_sharp_angle_detection(self):
        """Test sharp angle detection."""
        # Create polygon with sharp angle
        sharp_poly = Polygon([
            Point(0, 0), Point(10, 0), Point(1, 1), Point(0, 10)
        ])
        
        sharp_angles = sharp_poly.get_sharp_angles(threshold_degrees=45)
        self.assertGreater(len(sharp_angles), 0)
        
        # Square should have no sharp angles (all 90 degrees)
        square_sharp = self.square.get_sharp_angles(threshold_degrees=45)
        self.assertEqual(len(square_sharp), 0)

    def test_sharp_angle_detection_at_boundary(self):
        """Test sharp angle detection at threshold boundary values."""
        # Square: all angles are 90 degrees
        # At threshold 90, should not be considered sharp (assuming strict <)
        square_90 = self.square.get_sharp_angles(threshold_degrees=90)
        self.assertEqual(len(square_90), 0)

        # At threshold just above 90, still should be 0
        square_91 = self.square.get_sharp_angles(threshold_degrees=91)
        self.assertEqual(len(square_91), 0)

        # At threshold just below 90, all 4 should be detected as sharp
        square_89 = self.square.get_sharp_angles(threshold_degrees=89)
        self.assertEqual(len(square_89), 4)

        # Triangle with 45 degree angle
        right_triangle = Polygon([
            Point(0, 0), Point(1, 0), Point(0, 1)
        ])
        # All angles: 90, 45, 45
        # At threshold 45, should not be considered sharp (assuming strict <)
        tri_45 = right_triangle.get_sharp_angles(threshold_degrees=45)
        self.assertEqual(len(tri_45), 0)

        # At threshold just below 45, both 45 degree angles should be detected
        tri_44 = right_triangle.get_sharp_angles(threshold_degrees=44.999)
        self.assertEqual(len(tri_44), 2)

        # At threshold just above 45, still should be 0
        tri_46 = right_triangle.get_sharp_angles(threshold_degrees=46)
        self.assertEqual(len(tri_46), 0)
class TestGeometryIntegration(unittest.TestCase):
    """Integration tests for geometry classes."""
    
    def test_rectangle_polygon_consistency(self):
        """Test that rectangle-to-polygon conversion is consistent."""
        rect = Rectangle(5, 10, 15, 8)
        polygon = rect.to_polygon()
        
        # Areas should match
        rect_area = rect.width * rect.height
        poly_area = polygon.area()
        self.assertAlmostEqual(rect_area, poly_area, places=10)
        
        # Bounding boxes should match
        poly_bbox = polygon.bounding_box()
        self.assertAlmostEqual(rect.x, poly_bbox.x, places=10)
        self.assertAlmostEqual(rect.y, poly_bbox.y, places=10)
        self.assertAlmostEqual(rect.width, poly_bbox.width, places=10)
        self.assertAlmostEqual(rect.height, poly_bbox.height, places=10)
    
    def test_point_line_distance_edge_cases(self):
        """Test edge cases for point-to-line distance."""
        # Degenerate line (point)
        p1 = Point(5, 5)
        degenerate_start = Point(0, 0)
        degenerate_end = Point(0, 0)
        
        distance = p1.distance_to_line(degenerate_start, degenerate_end)
        expected_distance = p1.distance_to(degenerate_start)
        self.assertAlmostEqual(distance, expected_distance, places=10)
        
        # Point at line endpoint
        line_start = Point(0, 0)
        line_end = Point(10, 0)
        endpoint = Point(0, 0)
        
        distance = endpoint.distance_to_line(line_start, line_end)
        self.assertAlmostEqual(distance, 0, places=10)


if __name__ == '__main__':
    unittest.main()