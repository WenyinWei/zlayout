/**
 * @file point.cpp
 * @brief Implementation of Point class
 */

#include <zlayout/geometry/point.hpp>
#include <cmath>
#include <sstream>
#include <iomanip>

namespace zlayout {
namespace geometry {

// Point class implementation

bool Point::operator==(const Point& other) const {
    return std::abs(x - other.x) < TOLERANCE && 
           std::abs(y - other.y) < TOLERANCE;
}

bool Point::operator!=(const Point& other) const {
    return !(*this == other);
}

Point Point::operator+(const Point& other) const {
    return Point(x + other.x, y + other.y);
}

Point Point::operator-(const Point& other) const {
    return Point(x - other.x, y - other.y);
}

Point Point::operator*(double scalar) const {
    return Point(x * scalar, y * scalar);
}

Point Point::operator/(double scalar) const {
    if (std::abs(scalar) < TOLERANCE) {
        throw std::invalid_argument("Division by zero in Point::operator/");
    }
    return Point(x / scalar, y / scalar);
}

Point& Point::operator+=(const Point& other) {
    x += other.x;
    y += other.y;
    return *this;
}

Point& Point::operator-=(const Point& other) {
    x -= other.x;
    y -= other.y;
    return *this;
}

double Point::distance_to(const Point& other) const {
    double dx = x - other.x;
    double dy = y - other.y;
    return std::sqrt(dx * dx + dy * dy);
}

double Point::distance_squared_to(const Point& other) const {
    double dx = x - other.x;
    double dy = y - other.y;
    return dx * dx + dy * dy;
}

double Point::distance_to_line(const Point& line_start, const Point& line_end) const {
    // Vector from line_start to line_end
    Point line_vec = line_end - line_start;
    double line_length_sq = line_vec.magnitude_squared();
    
    // Handle degenerate line (point)
    if (line_length_sq < TOLERANCE) {
        return distance_to(line_start);
    }
    
    // Vector from line_start to this point
    Point point_vec = *this - line_start;
    
    // Project point onto line (parameter t)
    double t = point_vec.dot(line_vec) / line_length_sq;
    
    // Clamp t to [0, 1] to stay on line segment
    t = std::max(0.0, std::min(1.0, t));
    
    // Find closest point on line segment
    Point closest = line_start + line_vec * t;
    
    return distance_to(closest);
}

double Point::dot(const Point& other) const {
    return x * other.x + y * other.y;
}

double Point::cross(const Point& other) const {
    return x * other.y - y * other.x;
}

double Point::magnitude() const {
    return std::sqrt(x * x + y * y);
}

double Point::magnitude_squared() const {
    return x * x + y * y;
}

Point Point::normalize() const {
    double mag = magnitude();
    if (mag < TOLERANCE) {
        return Point(0.0, 0.0); // Return zero vector if magnitude is too small
    }
    return Point(x / mag, y / mag);
}

Point Point::rotate(double angle) const {
    double cos_a = std::cos(angle);
    double sin_a = std::sin(angle);
    return Point(x * cos_a - y * sin_a, x * sin_a + y * cos_a);
}

Point Point::rotate_around(const Point& center, double angle) const {
    Point translated = *this - center;
    Point rotated = translated.rotate(angle);
    return rotated + center;
}

double Point::angle_to(const Point& other) const {
    Point vec = other - *this;
    return std::atan2(vec.y, vec.x);
}

bool Point::is_zero() const {
    return magnitude() < TOLERANCE;
}

std::string Point::to_string() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6) << "Point(" << x << ", " << y << ")";
    return oss.str();
}

std::ostream& operator<<(std::ostream& os, const Point& point) {
    os << point.to_string();
    return os;
}

// PointHash implementation

std::size_t PointHash::operator()(const Point& point) const {
    // Use a simple hash combining function
    std::hash<double> hasher;
    std::size_t h1 = hasher(std::round(point.x / Point::TOLERANCE) * Point::TOLERANCE);
    std::size_t h2 = hasher(std::round(point.y / Point::TOLERANCE) * Point::TOLERANCE);
    return h1 ^ (h2 << 1);
}

// Utility functions

double distance(const Point& p1, const Point& p2) {
    return p1.distance_to(p2);
}

Point midpoint(const Point& p1, const Point& p2) {
    return Point((p1.x + p2.x) * 0.5, (p1.y + p2.y) * 0.5);
}

double angle_between_points(const Point& p1, const Point& p2, const Point& p3) {
    // Calculate vectors from p2 to p1 and p2 to p3
    Point v1 = p1 - p2;
    Point v2 = p3 - p2;
    
    // Calculate angle using dot product
    double dot_product = v1.dot(v2);
    double mag1 = v1.magnitude();
    double mag2 = v2.magnitude();
    
    if (mag1 < Point::TOLERANCE || mag2 < Point::TOLERANCE) {
        return 0.0;
    }
    
    double cos_angle = dot_product / (mag1 * mag2);
    // Clamp to [-1, 1] to handle numerical errors
    cos_angle = std::max(-1.0, std::min(1.0, cos_angle));
    
    return std::acos(cos_angle);
}

bool are_collinear(const Point& p1, const Point& p2, const Point& p3) {
    // Calculate cross product to determine if points are collinear
    Point v1 = p2 - p1;
    Point v2 = p3 - p1;
    double cross_product = v1.cross(v2);
    return std::abs(cross_product) < Point::TOLERANCE;
}

int orientation(const Point& p1, const Point& p2, const Point& p3) {
    // Calculate cross product to determine orientation
    Point v1 = p2 - p1;
    Point v2 = p3 - p2;
    double cross_product = v1.cross(v2);
    
    if (std::abs(cross_product) < Point::TOLERANCE) {
        return 0; // Collinear
    }
    return (cross_product > 0) ? 1 : 2; // 1 for clockwise, 2 for counterclockwise
}

} // namespace geometry
} // namespace zlayout