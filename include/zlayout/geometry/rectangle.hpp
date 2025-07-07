/**
 * @file rectangle.hpp
 * @brief Axis-aligned rectangle class for bounding boxes and simple components
 */

#pragma once

#include <zlayout/geometry/point.hpp>
#include <vector>
#include <string>

namespace zlayout {
namespace geometry {

// Forward declaration
class Polygon;

/**
 * @class Rectangle
 * @brief Axis-aligned rectangle for bounding boxes and simple EDA components
 * 
 * Represents an axis-aligned rectangle in 2D space. Commonly used for
 * component bounding boxes, chip outlines, and simple rectangular components
 * in EDA layouts.
 */
class Rectangle {
public:
    double x;      ///< X coordinate of bottom-left corner
    double y;      ///< Y coordinate of bottom-left corner
    double width;  ///< Width of rectangle
    double height; ///< Height of rectangle

    /**
     * @brief Default constructor - creates empty rectangle at origin
     */
    Rectangle() : x(0.0), y(0.0), width(0.0), height(0.0) {}

    /**
     * @brief Constructor with position and dimensions
     * @param x X coordinate of bottom-left corner
     * @param y Y coordinate of bottom-left corner
     * @param width Rectangle width
     * @param height Rectangle height
     */
    Rectangle(double x, double y, double width, double height);

    /**
     * @brief Constructor from two corner points
     * @param bottom_left Bottom-left corner
     * @param top_right Top-right corner
     */
    Rectangle(const Point& bottom_left, const Point& top_right);

    /**
     * @brief Copy constructor
     */
    Rectangle(const Rectangle& other) = default;

    /**
     * @brief Assignment operator
     */
    Rectangle& operator=(const Rectangle& other) = default;

    /**
     * @brief Equality operator
     */
    bool operator==(const Rectangle& other) const;

    /**
     * @brief Inequality operator
     */
    bool operator!=(const Rectangle& other) const;

    /**
     * @brief Less than operator for sorting
     */
    bool operator<(const Rectangle& other) const;

    // Boundary accessors
    /**
     * @brief Get left edge X coordinate
     */
    double left() const { return x; }

    /**
     * @brief Get right edge X coordinate
     */
    double right() const { return x + width; }

    /**
     * @brief Get bottom edge Y coordinate
     */
    double bottom() const { return y; }

    /**
     * @brief Get top edge Y coordinate
     */
    double top() const { return y + height; }

    /**
     * @brief Get center point of rectangle
     */
    Point center() const;

    /**
     * @brief Get bottom-left corner point
     */
    Point bottom_left() const { return Point(x, y); }

    /**
     * @brief Get bottom-right corner point
     */
    Point bottom_right() const { return Point(x + width, y); }

    /**
     * @brief Get top-left corner point
     */
    Point top_left() const { return Point(x, y + height); }

    /**
     * @brief Get top-right corner point
     */
    Point top_right() const { return Point(x + width, y + height); }

    /**
     * @brief Get all four corner points
     * @return Vector of corner points in order: bottom-left, bottom-right, top-right, top-left
     */
    std::vector<Point> corners() const;

    /**
     * @brief Calculate rectangle area
     */
    double area() const { return width * height; }

    /**
     * @brief Calculate rectangle perimeter
     */
    double perimeter() const { return 2.0 * (width + height); }

    /**
     * @brief Check if rectangle is empty (zero area)
     */
    bool is_empty() const;

    /**
     * @brief Check if rectangle is valid (positive dimensions)
     */
    bool is_valid() const;

    /**
     * @brief Check if point is inside rectangle (inclusive of boundary)
     * @param point Point to test
     * @return true if point is inside or on rectangle boundary
     */
    bool contains_point(const Point& point) const;

    /**
     * @brief Check if another rectangle is completely inside this rectangle
     * @param other Rectangle to test
     * @return true if other rectangle is completely inside
     */
    bool contains_rectangle(const Rectangle& other) const;

    /**
     * @brief Check if this rectangle intersects with another rectangle
     * @param other Rectangle to test intersection with
     * @return true if rectangles intersect
     */
    bool intersects(const Rectangle& other) const;

    /**
     * @brief Calculate intersection rectangle with another rectangle
     * @param other Rectangle to intersect with
     * @return Intersection rectangle (may be empty)
     */
    Rectangle intersection(const Rectangle& other) const;

    /**
     * @brief Calculate union rectangle that contains both rectangles
     * @param other Rectangle to union with
     * @return Union rectangle
     */
    Rectangle union_with(const Rectangle& other) const;

    /**
     * @brief Expand rectangle by given margin in all directions
     * @param margin Margin to add (can be negative to shrink)
     * @return Expanded rectangle
     */
    Rectangle expand(double margin) const;

    /**
     * @brief Expand rectangle by different margins in each direction
     * @param left Left margin
     * @param right Right margin
     * @param bottom Bottom margin
     * @param top Top margin
     * @return Expanded rectangle
     */
    Rectangle expand(double left, double right, double bottom, double top) const;

    /**
     * @brief Translate rectangle by given offset
     * @param offset Translation vector
     * @return Translated rectangle
     */
    Rectangle translate(const Point& offset) const;

    /**
     * @brief Scale rectangle by given factor (around center)
     * @param factor Scale factor
     * @return Scaled rectangle
     */
    Rectangle scale(double factor) const;

    /**
     * @brief Scale rectangle by different factors in X and Y
     * @param x_factor X scale factor
     * @param y_factor Y scale factor
     * @return Scaled rectangle
     */
    Rectangle scale(double x_factor, double y_factor) const;

    /**
     * @brief Calculate minimum distance to another rectangle
     * @param other Target rectangle
     * @return Minimum distance (0 if rectangles intersect)
     */
    double distance_to(const Rectangle& other) const;

    /**
     * @brief Calculate minimum distance to a point
     * @param point Target point
     * @return Minimum distance to point
     */
    double distance_to(const Point& point) const;

    /**
     * @brief Convert rectangle to polygon representation
     * @return Polygon with rectangle vertices
     */
    Polygon to_polygon() const;

    /**
     * @brief Get string representation
     */
    std::string to_string() const;

    /**
     * @brief Stream output operator
     */
    friend std::ostream& operator<<(std::ostream& os, const Rectangle& rect);

    // Static utility functions

    /**
     * @brief Create rectangle from center point and dimensions
     * @param center Center point
     * @param width Rectangle width
     * @param height Rectangle height
     * @return Rectangle centered at given point
     */
    static Rectangle from_center(const Point& center, double width, double height);

    /**
     * @brief Create bounding rectangle for a set of points
     * @param points Vector of points
     * @return Bounding rectangle
     */
    static Rectangle bounding_box(const std::vector<Point>& points);

    /**
     * @brief Create rectangle that bounds multiple rectangles
     * @param rectangles Vector of rectangles
     * @return Bounding rectangle
     */
    static Rectangle bounding_box(const std::vector<Rectangle>& rectangles);
};

/**
 * @brief Hash function for Rectangle (for use in std::unordered_map, etc.)
 */
struct RectangleHash {
    std::size_t operator()(const Rectangle& rect) const;
};

} // namespace geometry
} // namespace zlayout