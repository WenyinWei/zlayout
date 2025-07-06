/**
 * @file rectangle.cpp
 * @brief Implementation of Rectangle class
 */

#include <zlayout/geometry/rectangle.hpp>
#include <zlayout/geometry/polygon.hpp>
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <cmath>

namespace zlayout {
namespace geometry {

Rectangle::Rectangle(double x, double y, double width, double height)
    : x(x), y(y), width(width), height(height) {
    if (width < 0 || height < 0) {
        throw std::invalid_argument("Rectangle dimensions must be non-negative");
    }
}

Rectangle::Rectangle(const Point& bottom_left, const Point& top_right)
    : x(std::min(bottom_left.x, top_right.x)),
      y(std::min(bottom_left.y, top_right.y)),
      width(std::abs(top_right.x - bottom_left.x)),
      height(std::abs(top_right.y - bottom_left.y)) {
}

bool Rectangle::operator==(const Rectangle& other) const {
    return std::abs(x - other.x) < Point::TOLERANCE &&
           std::abs(y - other.y) < Point::TOLERANCE &&
           std::abs(width - other.width) < Point::TOLERANCE &&
           std::abs(height - other.height) < Point::TOLERANCE;
}

bool Rectangle::operator!=(const Rectangle& other) const {
    return !(*this == other);
}

Point Rectangle::center() const {
    return Point(x + width / 2.0, y + height / 2.0);
}

std::vector<Point> Rectangle::corners() const {
    return {
        Point(x, y),                    // bottom-left
        Point(x + width, y),            // bottom-right
        Point(x + width, y + height),   // top-right
        Point(x, y + height)            // top-left
    };
}

bool Rectangle::is_empty() const {
    return width < Point::TOLERANCE || height < Point::TOLERANCE;
}

bool Rectangle::is_valid() const {
    return width >= 0 && height >= 0;
}

bool Rectangle::contains_point(const Point& point) const {
    return point.x >= x && point.x <= x + width &&
           point.y >= y && point.y <= y + height;
}

bool Rectangle::contains_rectangle(const Rectangle& other) const {
    return other.x >= x && other.y >= y &&
           other.x + other.width <= x + width &&
           other.y + other.height <= y + height;
}

bool Rectangle::intersects(const Rectangle& other) const {
    return !(other.x >= x + width || other.x + other.width <= x ||
             other.y >= y + height || other.y + other.height <= y);
}

Rectangle Rectangle::intersection(const Rectangle& other) const {
    if (!intersects(other)) {
        return Rectangle(0, 0, 0, 0); // Empty rectangle
    }
    
    double left = std::max(x, other.x);
    double right = std::min(x + width, other.x + other.width);
    double bottom = std::max(y, other.y);
    double top = std::min(y + height, other.y + other.height);
    
    return Rectangle(left, bottom, right - left, top - bottom);
}

Rectangle Rectangle::union_with(const Rectangle& other) const {
    if (is_empty()) return other;
    if (other.is_empty()) return *this;
    
    double left = std::min(x, other.x);
    double right = std::max(x + width, other.x + other.width);
    double bottom = std::min(y, other.y);
    double top = std::max(y + height, other.y + other.height);
    
    return Rectangle(left, bottom, right - left, top - bottom);
}

Rectangle Rectangle::expand(double margin) const {
    return Rectangle(x - margin, y - margin, 
                    width + 2 * margin, height + 2 * margin);
}

Rectangle Rectangle::expand(double left, double right, double bottom, double top) const {
    return Rectangle(x - left, y - bottom,
                    width + left + right, height + bottom + top);
}

Rectangle Rectangle::translate(const Point& offset) const {
    return Rectangle(x + offset.x, y + offset.y, width, height);
}

Rectangle Rectangle::scale(double factor) const {
    Point center_point = center();
    double new_width = width * factor;
    double new_height = height * factor;
    
    return Rectangle(center_point.x - new_width / 2.0,
                    center_point.y - new_height / 2.0,
                    new_width, new_height);
}

Rectangle Rectangle::scale(double x_factor, double y_factor) const {
    Point center_point = center();
    double new_width = width * x_factor;
    double new_height = height * y_factor;
    
    return Rectangle(center_point.x - new_width / 2.0,
                    center_point.y - new_height / 2.0,
                    new_width, new_height);
}

double Rectangle::distance_to(const Rectangle& other) const {
    if (intersects(other)) {
        return 0.0;
    }
    
    // Calculate distance between closest edges
    double dx = 0.0, dy = 0.0;
    
    if (other.x + other.width < x) {
        dx = x - (other.x + other.width);
    } else if (other.x > x + width) {
        dx = other.x - (x + width);
    }
    
    if (other.y + other.height < y) {
        dy = y - (other.y + other.height);
    } else if (other.y > y + height) {
        dy = other.y - (y + height);
    }
    
    return std::sqrt(dx * dx + dy * dy);
}

double Rectangle::distance_to(const Point& point) const {
    if (contains_point(point)) {
        return 0.0;
    }
    
    double dx = std::max(0.0, std::max(x - point.x, point.x - (x + width)));
    double dy = std::max(0.0, std::max(y - point.y, point.y - (y + height)));
    
    return std::sqrt(dx * dx + dy * dy);
}

Polygon Rectangle::to_polygon() const {
    auto corner_points = corners();
    return Polygon(corner_points);
}

std::string Rectangle::to_string() const {
    std::ostringstream oss;
    oss << "Rectangle(x=" << x << ", y=" << y 
        << ", width=" << width << ", height=" << height << ")";
    return oss.str();
}

std::ostream& operator<<(std::ostream& os, const Rectangle& rect) {
    os << rect.to_string();
    return os;
}

// Static utility functions

Rectangle Rectangle::from_center(const Point& center, double width, double height) {
    return Rectangle(center.x - width / 2.0, center.y - height / 2.0, width, height);
}

Rectangle Rectangle::bounding_box(const std::vector<Point>& points) {
    if (points.empty()) {
        return Rectangle(0, 0, 0, 0);
    }
    
    double min_x = points[0].x, max_x = points[0].x;
    double min_y = points[0].y, max_y = points[0].y;
    
    for (const auto& point : points) {
        min_x = std::min(min_x, point.x);
        max_x = std::max(max_x, point.x);
        min_y = std::min(min_y, point.y);
        max_y = std::max(max_y, point.y);
    }
    
    return Rectangle(min_x, min_y, max_x - min_x, max_y - min_y);
}

Rectangle Rectangle::bounding_box(const std::vector<Rectangle>& rectangles) {
    if (rectangles.empty()) {
        return Rectangle(0, 0, 0, 0);
    }
    
    Rectangle result = rectangles[0];
    for (size_t i = 1; i < rectangles.size(); ++i) {
        result = result.union_with(rectangles[i]);
    }
    
    return result;
}

// RectangleHash implementation

std::size_t RectangleHash::operator()(const Rectangle& rect) const {
    std::hash<double> hasher;
    std::size_t h1 = hasher(rect.x);
    std::size_t h2 = hasher(rect.y);
    std::size_t h3 = hasher(rect.width);
    std::size_t h4 = hasher(rect.height);
    
    // Combine hashes
    return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
}

} // namespace geometry
} // namespace zlayout