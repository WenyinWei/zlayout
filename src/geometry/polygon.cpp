/**
 * @file polygon.cpp
 * @brief Implementation of Polygon class with EDA-specific algorithms
 */

#define _USE_MATH_DEFINES
#include <zlayout/geometry/polygon.hpp>
#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>

// Fallback for M_PI if not defined (MSVC 2012 compatibility)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace zlayout {
namespace geometry {

// Constructor implementations
Polygon::Polygon(const std::vector<Point>& vertices) : vertices(vertices) {}

Polygon::Polygon(std::initializer_list<Point> vertices) : vertices(vertices) {}

// Basic operators
bool Polygon::operator==(const Polygon& other) const {
    return vertices == other.vertices;
}

bool Polygon::operator!=(const Polygon& other) const {
    return !(*this == other);
}

// Basic properties
std::vector<std::pair<Point, Point>> Polygon::edges() const {
    std::vector<std::pair<Point, Point>> edge_list;
    if (vertices.size() < 2) return edge_list;
    
    for (size_t i = 0; i < vertices.size(); ++i) {
        size_t next = (i + 1) % vertices.size();
        edge_list.emplace_back(vertices[i], vertices[next]);
    }
    return edge_list;
}

double Polygon::area() const {
    return std::abs(signed_area());
}

double Polygon::signed_area() const {
    if (vertices.size() < 3) return 0.0;
    
    double area = 0.0;
    for (size_t i = 0; i < vertices.size(); ++i) {
        size_t next = (i + 1) % vertices.size();
        area += vertices[i].x * vertices[next].y - vertices[next].x * vertices[i].y;
    }
    return area / 2.0;
}

double Polygon::perimeter() const {
    if (vertices.size() < 2) return 0.0;
    
    double perimeter = 0.0;
    for (size_t i = 0; i < vertices.size(); ++i) {
        size_t next = (i + 1) % vertices.size();
        perimeter += vertices[i].distance_to(vertices[next]);
    }
    return perimeter;
}

Point Polygon::centroid() const {
    if (vertices.empty()) return Point(0, 0);
    
    double cx = 0.0, cy = 0.0;
    for (const auto& vertex : vertices) {
        cx += vertex.x;
        cy += vertex.y;
    }
    return Point(cx / vertices.size(), cy / vertices.size());
}

Rectangle Polygon::bounding_box() const {
    if (vertices.empty()) return Rectangle(0, 0, 0, 0);
    
    double min_x = vertices[0].x, max_x = vertices[0].x;
    double min_y = vertices[0].y, max_y = vertices[0].y;
    
    for (const auto& vertex : vertices) {
        min_x = std::min(min_x, vertex.x);
        max_x = std::max(max_x, vertex.x);
        min_y = std::min(min_y, vertex.y);
        max_y = std::max(max_y, vertex.y);
    }
    
    return Rectangle(min_x, min_y, max_x - min_x, max_y - min_y);
}

// Geometric properties
bool Polygon::is_convex() const {
    if (vertices.size() < 3) return false;
    
    bool sign_positive = false;
    bool sign_negative = false;
    
    for (size_t i = 0; i < vertices.size(); ++i) {
        size_t j = (i + 1) % vertices.size();
        size_t k = (i + 2) % vertices.size();
        
        Point v1 = vertices[j] - vertices[i];
        Point v2 = vertices[k] - vertices[j];
        double cross = v1.x * v2.y - v1.y * v2.x;
        
        if (cross > Point::TOLERANCE) sign_positive = true;
        else if (cross < -Point::TOLERANCE) sign_negative = true;
        
        if (sign_positive && sign_negative) return false;
    }
    
    return true;
}

bool Polygon::is_clockwise() const {
    return signed_area() < 0.0;
}

bool Polygon::is_simple() const {
    if (vertices.size() < 3) return true;
    
    auto edge_list = edges();
    for (size_t i = 0; i < edge_list.size(); ++i) {
        for (size_t j = i + 2; j < edge_list.size(); ++j) {
            if (i == 0 && j == edge_list.size() - 1) continue; // Skip adjacent edges
            
            if (segments_intersect(edge_list[i].first, edge_list[i].second,
                                 edge_list[j].first, edge_list[j].second)) {
                return false;
            }
        }
    }
    return true;
}

// Point containment
bool Polygon::contains_point(const Point& point) const {
    if (vertices.size() < 3) return false;
    
    bool inside = false;
    size_t j = vertices.size() - 1;
    
    for (size_t i = 0; i < vertices.size(); ++i) {
        if (((vertices[i].y > point.y) != (vertices[j].y > point.y)) &&
            (point.x < (vertices[j].x - vertices[i].x) * (point.y - vertices[i].y) / 
             (vertices[j].y - vertices[i].y) + vertices[i].x)) {
            inside = !inside;
        }
        j = i;
    }
    
    return inside;
}

bool Polygon::point_on_boundary(const Point& point, double tolerance) const {
    auto edge_list = edges();
    for (const auto& edge : edge_list) {
        if (point.distance_to_line(edge.first, edge.second) < tolerance) {
            return true;
        }
    }
    return false;
}

// Sharp angle detection
std::vector<size_t> Polygon::get_sharp_angles(double threshold_degrees) const {
    std::vector<size_t> sharp_angles;
    if (vertices.size() < 3) return sharp_angles;
    
    for (size_t i = 0; i < vertices.size(); ++i) {
        double angle = vertex_angle(i);
        if (angle < threshold_degrees) {
            sharp_angles.push_back(i);
        }
    }
    
    return sharp_angles;
}

double Polygon::vertex_angle(size_t vertex_index) const {
    if (vertices.size() < 3 || vertex_index >= vertices.size()) return 0.0;
    
    size_t prev = (vertex_index + vertices.size() - 1) % vertices.size();
    size_t next = (vertex_index + 1) % vertices.size();
    
    Point v1 = vertices[prev] - vertices[vertex_index];
    Point v2 = vertices[next] - vertices[vertex_index];
    
    double dot = v1.x * v2.x + v1.y * v2.y;
    double mag1 = std::sqrt(v1.x * v1.x + v1.y * v1.y);
    double mag2 = std::sqrt(v2.x * v2.x + v2.y * v2.y);
    
    if (mag1 < Point::TOLERANCE || mag2 < Point::TOLERANCE) return 0.0;
    
    double cos_angle = dot / (mag1 * mag2);
    cos_angle = std::max(-1.0, std::min(1.0, cos_angle)); // Clamp to [-1, 1]
    
    return std::acos(cos_angle) * 180.0 / M_PI;
}

std::vector<double> Polygon::all_vertex_angles() const {
    std::vector<double> angles;
    for (size_t i = 0; i < vertices.size(); ++i) {
        angles.push_back(vertex_angle(i));
    }
    return angles;
}

// Distance calculations
double Polygon::distance_to(const Polygon& other) const {
    if (vertices.empty() || other.vertices.empty()) return 0.0;
    
    double min_distance = std::numeric_limits<double>::max();
    
    // Check distance from each vertex to other polygon
    for (const auto& vertex : vertices) {
        double dist = other.distance_to(vertex);
        min_distance = std::min(min_distance, dist);
    }
    
    // Check distance from each vertex of other polygon to this polygon
    for (const auto& vertex : other.vertices) {
        double dist = distance_to(vertex);
        min_distance = std::min(min_distance, dist);
    }
    
    return min_distance;
}

double Polygon::distance_to(const Point& point) const {
    if (vertices.empty()) return 0.0;
    
    if (contains_point(point)) return 0.0;
    
    double min_distance = std::numeric_limits<double>::max();
    auto edge_list = edges();
    
    for (const auto& edge : edge_list) {
        double dist = point.distance_to_line(edge.first, edge.second);
        min_distance = std::min(min_distance, dist);
    }
    
    return min_distance;
}

double Polygon::distance_to_line(const Point& line_start, const Point& line_end) const {
    double min_distance = std::numeric_limits<double>::max();
    
    for (const auto& vertex : vertices) {
        double dist = vertex.distance_to_line(line_start, line_end);
        min_distance = std::min(min_distance, dist);
    }
    
    return min_distance;
}

Point Polygon::closest_point_to(const Point& point) const {
    if (vertices.empty()) return Point(0, 0);
    
    Point closest = vertices[0];
    double min_distance = vertices[0].distance_to(point);
    
    for (const auto& vertex : vertices) {
        double dist = vertex.distance_to(point);
        if (dist < min_distance) {
            min_distance = dist;
            closest = vertex;
        }
    }
    
    return closest;
}

// Edge operations
double Polygon::min_edge_distance_to(const Polygon& other) const {
    auto this_edges = edges();
    auto other_edges = other.edges();
    
    double min_distance = std::numeric_limits<double>::max();
    
    for (const auto& edge1 : this_edges) {
        for (const auto& edge2 : other_edges) {
            double dist = segment_to_segment_distance(
                edge1.first, edge1.second, edge2.first, edge2.second);
            min_distance = std::min(min_distance, dist);
        }
    }
    
    return min_distance;
}

std::vector<std::tuple<Point, Point, double>> Polygon::find_narrow_regions(
    const Polygon& other, double threshold_distance) const {
    
    std::vector<std::tuple<Point, Point, double>> narrow_regions;
    auto this_edges = edges();
    auto other_edges = other.edges();
    
    for (const auto& edge1 : this_edges) {
        for (const auto& edge2 : other_edges) {
            double dist = segment_to_segment_distance(
                edge1.first, edge1.second, edge2.first, edge2.second);
            
            if (dist < threshold_distance) {
                Point closest1 = edge1.first; // Simplified - should find actual closest points
                Point closest2 = edge2.first;
                narrow_regions.emplace_back(closest1, closest2, dist);
            }
        }
    }
    
    return narrow_regions;
}

// Intersection detection
bool Polygon::intersects(const Polygon& other) const {
    auto this_edges = edges();
    auto other_edges = other.edges();
    
    for (const auto& edge1 : this_edges) {
        for (const auto& edge2 : other_edges) {
            if (segments_intersect(edge1.first, edge1.second, 
                                 edge2.first, edge2.second)) {
                return true;
            }
        }
    }
    
    return false;
}

std::vector<Point> Polygon::intersection_points(const Polygon& other) const {
    std::vector<Point> intersections;
    auto this_edges = edges();
    auto other_edges = other.edges();
    
    for (const auto& edge1 : this_edges) {
        for (const auto& edge2 : other_edges) {
            bool intersects = false;
            Point intersection = line_segment_intersection(
                edge1.first, edge1.second, edge2.first, edge2.second, intersects);
            
            if (intersects) {
                intersections.push_back(intersection);
            }
        }
    }
    
    return intersections;
}

bool Polygon::has_self_intersections() const {
    auto edge_list = edges();
    
    for (size_t i = 0; i < edge_list.size(); ++i) {
        for (size_t j = i + 2; j < edge_list.size(); ++j) {
            if (i == 0 && j == edge_list.size() - 1) continue; // Skip adjacent edges
            
            if (segments_intersect(edge_list[i].first, edge_list[i].second,
                                 edge_list[j].first, edge_list[j].second)) {
                return true;
            }
        }
    }
    
    return false;
}

// Utility functions
void Polygon::add_vertex(const Point& vertex) {
    vertices.push_back(vertex);
}

void Polygon::insert_vertex(size_t index, const Point& vertex) {
    if (index <= vertices.size()) {
        vertices.insert(vertices.begin() + index, vertex);
    }
}

void Polygon::remove_vertex(size_t index) {
    if (index < vertices.size()) {
        vertices.erase(vertices.begin() + index);
    }
}

std::string Polygon::to_string() const {
    std::stringstream ss;
    ss << "Polygon[";
    for (size_t i = 0; i < vertices.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << vertices[i].to_string();
    }
    ss << "]";
    return ss.str();
}

std::ostream& operator<<(std::ostream& os, const Polygon& polygon) {
    return os << polygon.to_string();
}

// Static utility functions
double Polygon::segment_to_segment_distance(
    const Point& seg1_start, const Point& seg1_end,
    const Point& seg2_start, const Point& seg2_end) {
    
    // Simplified implementation - return distance between midpoints
    Point mid1((seg1_start.x + seg1_end.x) / 2, (seg1_start.y + seg1_end.y) / 2);
    Point mid2((seg2_start.x + seg2_end.x) / 2, (seg2_start.y + seg2_end.y) / 2);
    return mid1.distance_to(mid2);
}

Point Polygon::line_segment_intersection(
    const Point& seg1_start, const Point& seg1_end,
    const Point& seg2_start, const Point& seg2_end,
    bool& intersects) {
    
    intersects = false;
    
    double x1 = seg1_start.x, y1 = seg1_start.y;
    double x2 = seg1_end.x, y2 = seg1_end.y;
    double x3 = seg2_start.x, y3 = seg2_start.y;
    double x4 = seg2_end.x, y4 = seg2_end.y;
    
    double denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    if (std::abs(denom) < Point::TOLERANCE) {
        return Point(0, 0); // Parallel lines
    }
    
    double t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / denom;
    double u = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3)) / denom;
    
    if (t >= 0 && t <= 1 && u >= 0 && u <= 1) {
        intersects = true;
        return Point(x1 + t * (x2 - x1), y1 + t * (y2 - y1));
    }
    
    return Point(0, 0);
}

bool Polygon::segments_intersect(
    const Point& seg1_start, const Point& seg1_end,
    const Point& seg2_start, const Point& seg2_end) {
    
    bool intersects = false;
    line_segment_intersection(seg1_start, seg1_end, seg2_start, seg2_end, intersects);
    return intersects;
}

// Hash function implementation
std::size_t PolygonHash::operator()(const Polygon& polygon) const {
    std::size_t hash = 0;
    for (const auto& vertex : polygon.vertices) {
        hash ^= std::hash<double>()(vertex.x) + std::hash<double>()(vertex.y) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    }
    return hash;
}

// Global utility functions
bool segments_intersect(const Point& seg1_start, const Point& seg1_end,
                       const Point& seg2_start, const Point& seg2_end) {
    return Polygon::segments_intersect(seg1_start, seg1_end, seg2_start, seg2_end);
}

double angle_between_vectors(const Point& v1, const Point& v2) {
    double dot = v1.x * v2.x + v1.y * v2.y;
    double mag1 = std::sqrt(v1.x * v1.x + v1.y * v1.y);
    double mag2 = std::sqrt(v2.x * v2.x + v2.y * v2.y);
    
    if (mag1 < Point::TOLERANCE || mag2 < Point::TOLERANCE) return 0.0;
    
    double cos_angle = dot / (mag1 * mag2);
    cos_angle = std::max(-1.0, std::min(1.0, cos_angle)); // Clamp to [-1, 1]
    
    return std::acos(cos_angle) * 180.0 / M_PI;
}

} // namespace geometry
} // namespace zlayout 