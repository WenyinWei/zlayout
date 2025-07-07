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

// Polygon class implementation

Polygon::Polygon(const std::vector<Point>& vertices) : vertices(vertices) {
    if (vertices.size() < 3) {
        throw std::invalid_argument("Polygon must have at least 3 vertices");
    }
}

Polygon::Polygon(std::initializer_list<Point> vertices) : vertices(vertices) {
    if (this->vertices.size() < 3) {
        throw std::invalid_argument("Polygon must have at least 3 vertices");
    }
}

bool Polygon::operator==(const Polygon& other) const {
    if (vertices.size() != other.vertices.size()) {
        return false;
    }
    
    for (size_t i = 0; i < vertices.size(); ++i) {
        if (vertices[i] != other.vertices[i]) {
            return false;
        }
    }
    return true;
}

bool Polygon::operator!=(const Polygon& other) const {
    return !(*this == other);
}

std::vector<std::pair<Point, Point>> Polygon::edges() const {
    std::vector<std::pair<Point, Point>> edge_list;
    size_t n = vertices.size();
    
    for (size_t i = 0; i < n; ++i) {
        edge_list.emplace_back(vertices[i], vertices[(i + 1) % n]);
    }
    return edge_list;
}

double Polygon::area() const {
    return std::abs(signed_area());
}

double Polygon::signed_area() const {
    if (vertices.size() < 3) {
        return 0.0;
    }
    
    double area = 0.0;
    size_t n = vertices.size();
    
    for (size_t i = 0; i < n; ++i) {
        size_t j = (i + 1) % n;
        area += vertices[i].x * vertices[j].y;
        area -= vertices[j].x * vertices[i].y;
    }
    
    return area * 0.5;
}

double Polygon::perimeter() const {
    double perim = 0.0;
    size_t n = vertices.size();
    
    for (size_t i = 0; i < n; ++i) {
        size_t j = (i + 1) % n;
        perim += vertices[i].distance_to(vertices[j]);
    }
    
    return perim;
}

Point Polygon::centroid() const {
    if (vertices.empty()) {
        return Point(0.0, 0.0);
    }
    
    double cx = 0.0, cy = 0.0;
    double area = signed_area();
    
    if (std::abs(area) < Point::TOLERANCE) {
        // Degenerate polygon, return arithmetic mean
        for (const auto& vertex : vertices) {
            cx += vertex.x;
            cy += vertex.y;
        }
        return Point(cx / vertices.size(), cy / vertices.size());
    }
    
    size_t n = vertices.size();
    for (size_t i = 0; i < n; ++i) {
        size_t j = (i + 1) % n;
        double cross = vertices[i].x * vertices[j].y - vertices[j].x * vertices[i].y;
        cx += (vertices[i].x + vertices[j].x) * cross;
        cy += (vertices[i].y + vertices[j].y) * cross;
    }
    
    double factor = 1.0 / (6.0 * area);
    return Point(cx * factor, cy * factor);
}

Rectangle Polygon::bounding_box() const {
    if (vertices.empty()) {
        return Rectangle(0.0, 0.0, 0.0, 0.0);
    }
    
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

bool Polygon::is_convex() const {
    if (vertices.size() < 3) {
        return true;
    }
    
    size_t n = vertices.size();
    bool positive = false, negative = false;
    
    for (size_t i = 0; i < n; ++i) {
        size_t j = (i + 1) % n;
        size_t k = (i + 2) % n;
        
        // Calculate cross product
        Point v1 = vertices[j] - vertices[i];
        Point v2 = vertices[k] - vertices[j];
        double cross = v1.cross(v2);
        
        if (cross > Point::TOLERANCE) {
            positive = true;
        } else if (cross < -Point::TOLERANCE) {
            negative = true;
        }
        
        if (positive && negative) {
            return false;
        }
    }
    
    return true;
}

bool Polygon::is_clockwise() const {
    return signed_area() < 0.0;
}

bool Polygon::is_simple() const {
    // Check for self-intersections
    auto edge_list = edges();
    size_t n = edge_list.size();
    
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = i + 2; j < n; ++j) {
            // Skip adjacent edges
            if (j == n - 1 && i == 0) continue;
            
            if (zlayout::geometry::segments_intersect(edge_list[i].first, edge_list[i].second,
                                                     edge_list[j].first, edge_list[j].second)) {
                return false;
            }
        }
    }
    
    return true;
}

bool Polygon::contains_point(const Point& point) const {
    if (vertices.size() < 3) {
        return false;
    }
    
    // Ray casting algorithm
    double x = point.x, y = point.y;
    size_t n = vertices.size();
    bool inside = false;
    
    for (size_t i = 0, j = n - 1; i < n; j = i++) {
        double xi = vertices[i].x, yi = vertices[i].y;
        double xj = vertices[j].x, yj = vertices[j].y;
        
        if (((yi > y) != (yj > y)) &&
            (x < (xj - xi) * (y - yi) / (yj - yi) + xi)) {
            inside = !inside;
        }
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

// 面试题1: 尖角检测
std::vector<size_t> Polygon::get_sharp_angles(double threshold_degrees) const {
    std::vector<size_t> sharp_vertices;
    size_t n = vertices.size();
    
    if (n < 3) {
        return sharp_vertices;
    }
    
    double threshold_radians = threshold_degrees * M_PI / 180.0;
    
    for (size_t i = 0; i < n; ++i) {
        size_t prev_idx = (i + n - 1) % n;
        size_t next_idx = (i + 1) % n;
        
        Point prev_vertex = vertices[prev_idx];
        Point curr_vertex = vertices[i];
        Point next_vertex = vertices[next_idx];
        
        // Calculate vectors from current vertex
        Point v1 = prev_vertex - curr_vertex;
        Point v2 = next_vertex - curr_vertex;
        
        // Calculate angle between vectors
        double dot_product = v1.dot(v2);
        double mag1 = v1.magnitude();
        double mag2 = v2.magnitude();
        
        if (mag1 < Point::TOLERANCE || mag2 < Point::TOLERANCE) {
            continue; // Skip degenerate cases
        }
        
        double cos_angle = dot_product / (mag1 * mag2);
        cos_angle = std::max(-1.0, std::min(1.0, cos_angle)); // Clamp to [-1, 1]
        double angle = std::acos(cos_angle);
        
        // Check if angle is sharp (less than threshold or close to 180°)
        if (angle < threshold_radians || angle > (M_PI - threshold_radians)) {
            sharp_vertices.push_back(i);
        }
    }
    
    return sharp_vertices;
}

double Polygon::vertex_angle(size_t vertex_index) const {
    if (vertex_index >= vertices.size() || vertices.size() < 3) {
        return 0.0;
    }
    
    size_t n = vertices.size();
    size_t prev_idx = (vertex_index + n - 1) % n;
    size_t next_idx = (vertex_index + 1) % n;
    
    Point prev_vertex = vertices[prev_idx];
    Point curr_vertex = vertices[vertex_index];
    Point next_vertex = vertices[next_idx];
    
    Point v1 = prev_vertex - curr_vertex;
    Point v2 = next_vertex - curr_vertex;
    
    double dot_product = v1.dot(v2);
    double mag1 = v1.magnitude();
    double mag2 = v2.magnitude();
    
    if (mag1 < Point::TOLERANCE || mag2 < Point::TOLERANCE) {
        return 0.0;
    }
    
    double cos_angle = dot_product / (mag1 * mag2);
    cos_angle = std::max(-1.0, std::min(1.0, cos_angle));
    double angle_radians = std::acos(cos_angle);
    
    return angle_radians * 180.0 / M_PI; // Convert to degrees
}

std::vector<double> Polygon::all_vertex_angles() const {
    std::vector<double> angles;
    for (size_t i = 0; i < vertices.size(); ++i) {
        angles.push_back(vertex_angle(i));
    }
    return angles;
}

// 面试题2: 距离计算和窄距离检测
double Polygon::distance_to(const Polygon& other) const {
    double min_distance = std::numeric_limits<double>::max();
    
    // Check distance between all edge pairs
    auto edges1 = this->edges();
    auto edges2 = other.edges();
    
    for (const auto& edge1 : edges1) {
        for (const auto& edge2 : edges2) {
            double dist = segment_to_segment_distance(
                edge1.first, edge1.second, edge2.first, edge2.second);
            min_distance = std::min(min_distance, dist);
        }
    }
    
    return min_distance;
}

double Polygon::distance_to(const Point& point) const {
    double min_distance = std::numeric_limits<double>::max();
    
    auto edge_list = edges();
    for (const auto& edge : edge_list) {
        double dist = point.distance_to_line(edge.first, edge.second);
        min_distance = std::min(min_distance, dist);
    }
    
    return min_distance;
}

double Polygon::min_edge_distance_to(const Polygon& other) const {
    return distance_to(other);
}

std::vector<std::tuple<Point, Point, double>> 
Polygon::find_narrow_regions(const Polygon& other, double threshold_distance) const {
    std::vector<std::tuple<Point, Point, double>> narrow_regions;
    
    auto edges1 = this->edges();
    auto edges2 = other.edges();
    
    for (const auto& edge1 : edges1) {
        for (const auto& edge2 : edges2) {
            double dist = segment_to_segment_distance(
                edge1.first, edge1.second, edge2.first, edge2.second);
            
            if (dist < threshold_distance) {
                // Find closest points on the two edges
                Point closest1, closest2;
                // For simplicity, use edge midpoints (could be improved)
                closest1 = zlayout::geometry::midpoint(edge1.first, edge1.second);
                closest2 = zlayout::geometry::midpoint(edge2.first, edge2.second);
                
                narrow_regions.emplace_back(closest1, closest2, dist);
            }
        }
    }
    
    return narrow_regions;
}

// 面试题3: 相交检测
bool Polygon::intersects(const Polygon& other) const {
    auto edges1 = this->edges();
    auto edges2 = other.edges();
    
    for (const auto& edge1 : edges1) {
        for (const auto& edge2 : edges2) {
            if (zlayout::geometry::segments_intersect(edge1.first, edge1.second, 
                                                     edge2.first, edge2.second)) {
                return true;
            }
        }
    }
    
    // Also check if one polygon is completely inside the other
    if (!vertices.empty() && other.contains_point(vertices[0])) {
        return true;
    }
    if (!other.vertices.empty() && this->contains_point(other.vertices[0])) {
        return true;
    }
    
    return false;
}

std::vector<Point> Polygon::intersection_points(const Polygon& other) const {
    std::vector<Point> intersections;
    
    auto edges1 = this->edges();
    auto edges2 = other.edges();
    
    for (const auto& edge1 : edges1) {
        for (const auto& edge2 : edges2) {
            bool intersects;
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
    size_t n = edge_list.size();
    
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = i + 2; j < n; ++j) {
            // Skip adjacent edges and last-first edge pair
            if (j == n - 1 && i == 0) continue;
            
            if (zlayout::geometry::segments_intersect(edge_list[i].first, edge_list[i].second,
                                                     edge_list[j].first, edge_list[j].second)) {
                return true;
            }
        }
    }
    
    return false;
}

// Transformations
Polygon Polygon::translate(const Point& offset) const {
    std::vector<Point> new_vertices;
    new_vertices.reserve(vertices.size());
    
    for (const auto& vertex : vertices) {
        new_vertices.push_back(vertex + offset);
    }
    
    return Polygon(new_vertices);
}

Polygon Polygon::rotate(double angle) const {
    std::vector<Point> new_vertices;
    new_vertices.reserve(vertices.size());
    
    for (const auto& vertex : vertices) {
        new_vertices.push_back(vertex.rotate(angle));
    }
    
    return Polygon(new_vertices);
}

Polygon Polygon::rotate_around(const Point& center, double angle) const {
    std::vector<Point> new_vertices;
    new_vertices.reserve(vertices.size());
    
    for (const auto& vertex : vertices) {
        new_vertices.push_back(vertex.rotate_around(center, angle));
    }
    
    return Polygon(new_vertices);
}

Polygon Polygon::scale(double factor) const {
    Point center = centroid();
    std::vector<Point> new_vertices;
    new_vertices.reserve(vertices.size());
    
    for (const auto& vertex : vertices) {
        Point offset = vertex - center;
        new_vertices.push_back(center + offset * factor);
    }
    
    return Polygon(new_vertices);
}

std::string Polygon::to_string() const {
    std::ostringstream oss;
    oss << "Polygon(" << vertices.size() << " vertices: ";
    for (size_t i = 0; i < vertices.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << vertices[i].to_string();
    }
    oss << ")";
    return oss.str();
}

// Static helper functions
double Polygon::segment_to_segment_distance(const Point& seg1_start, const Point& seg1_end,
                                          const Point& seg2_start, const Point& seg2_end) {
    // Calculate all possible distances
    std::vector<double> distances = {
        seg1_start.distance_to_line(seg2_start, seg2_end),
        seg1_end.distance_to_line(seg2_start, seg2_end),
        seg2_start.distance_to_line(seg1_start, seg1_end),
        seg2_end.distance_to_line(seg1_start, seg1_end)
    };
    
    return *std::min_element(distances.begin(), distances.end());
}

Point Polygon::line_segment_intersection(const Point& seg1_start, const Point& seg1_end,
                                       const Point& seg2_start, const Point& seg2_end,
                                       bool& intersects) {
    intersects = false;
    
    double denom = (seg1_start.x - seg1_end.x) * (seg2_start.y - seg2_end.y) - 
                   (seg1_start.y - seg1_end.y) * (seg2_start.x - seg2_end.x);
    
    if (std::abs(denom) < Point::TOLERANCE) {
        return Point(0, 0); // Lines are parallel
    }
    
    double t = ((seg1_start.x - seg2_start.x) * (seg2_start.y - seg2_end.y) - 
                (seg1_start.y - seg2_start.y) * (seg2_start.x - seg2_end.x)) / denom;
    double u = -((seg1_start.x - seg1_end.x) * (seg1_start.y - seg2_start.y) - 
                 (seg1_start.y - seg1_end.y) * (seg1_start.x - seg2_start.x)) / denom;
    
    constexpr double EPSILON = 1e-9;
    if (t >= -EPSILON && t <= 1.0 + EPSILON && u >= -EPSILON && u <= 1.0 + EPSILON) {
        intersects = true;
        double x = seg1_start.x + t * (seg1_end.x - seg1_start.x);
        double y = seg1_start.y + t * (seg1_end.y - seg1_start.y);
        return Point(x, y);
    }
    
    return Point(0, 0);
}

bool Polygon::segments_intersect(const Point& seg1_start, const Point& seg1_end,
                               const Point& seg2_start, const Point& seg2_end) {
    bool intersects;
    line_segment_intersection(seg1_start, seg1_end, seg2_start, seg2_end, intersects);
    return intersects;
}

// Utility functions
bool segments_intersect(const Point& seg1_start, const Point& seg1_end,
                       const Point& seg2_start, const Point& seg2_end) {
    return Polygon::segments_intersect(seg1_start, seg1_end, seg2_start, seg2_end);
}

double angle_between_vectors(const Point& v1, const Point& v2) {
    double dot_product = v1.dot(v2);
    double mag1 = v1.magnitude();
    double mag2 = v2.magnitude();
    
    if (mag1 < Point::TOLERANCE || mag2 < Point::TOLERANCE) {
        return 0.0;
    }
    
    double cos_angle = dot_product / (mag1 * mag2);
    cos_angle = std::max(-1.0, std::min(1.0, cos_angle));
    double angle_radians = std::acos(cos_angle);
    
    return angle_radians * 180.0 / M_PI; // Convert to degrees
}

} // namespace geometry
} // namespace zlayout