/**
 * @file point.hpp
 * @brief 2D Point class for geometric calculations
 */

#pragma once

#include <cmath>
#include <iostream>
#include <functional>

namespace zlayout {
namespace geometry {

/**
 * @class Point
 * @brief 2D point with high-precision coordinates and utility methods
 * 
 * Represents a point in 2D space with double precision coordinates.
 * Provides distance calculations, geometric operations, and tolerance-based
 * equality comparisons suitable for EDA applications.
 */
class Point {
public:
    double x; ///< X coordinate
    double y; ///< Y coordinate

    /// Default precision tolerance for floating point comparisons
    static constexpr double TOLERANCE = 1e-10;

    /**
     * @brief Default constructor - creates point at origin
     */
    Point() : x(0.0), y(0.0) {}

    /**
     * @brief Constructor with coordinates
     * @param x X coordinate
     * @param y Y coordinate
     */
    Point(double x, double y) : x(x), y(y) {}

    /**
     * @brief Copy constructor
     */
    Point(const Point& other) = default;

    /**
     * @brief Assignment operator
     */
    Point& operator=(const Point& other) = default;

    /**
     * @brief Equality operator with tolerance
     * @param other Point to compare with
     * @return true if points are equal within tolerance
     */
    bool operator==(const Point& other) const;

    /**
     * @brief Inequality operator
     */
    bool operator!=(const Point& other) const;

    /**
     * @brief Addition operator
     */
    Point operator+(const Point& other) const;

    /**
     * @brief Subtraction operator
     */
    Point operator-(const Point& other) const;

    /**
     * @brief Scalar multiplication
     */
    Point operator*(double scalar) const;

    /**
     * @brief Scalar division
     */
    Point operator/(double scalar) const;

    /**
     * @brief Addition assignment
     */
    Point& operator+=(const Point& other);

    /**
     * @brief Subtraction assignment
     */
    Point& operator-=(const Point& other);

    /**
     * @brief Calculate Euclidean distance to another point
     * @param other Target point
     * @return Distance between points
     */
    double distance_to(const Point& other) const;

    /**
     * @brief Calculate squared distance (faster, avoids sqrt)
     * @param other Target point
     * @return Squared distance between points
     */
    double distance_squared_to(const Point& other) const;

    /**
     * @brief Calculate distance from this point to a line segment
     * @param line_start Start point of line segment
     * @param line_end End point of line segment
     * @return Minimum distance to line segment
     */
    double distance_to_line(const Point& line_start, const Point& line_end) const;

    /**
     * @brief Calculate dot product with another point (as vector)
     * @param other Other point/vector
     * @return Dot product
     */
    double dot(const Point& other) const;

    /**
     * @brief Calculate cross product magnitude (2D cross product)
     * @param other Other point/vector
     * @return Cross product magnitude
     */
    double cross(const Point& other) const;

    /**
     * @brief Calculate vector magnitude (length)
     * @return Length of vector from origin to this point
     */
    double magnitude() const;

    /**
     * @brief Calculate squared magnitude (faster, avoids sqrt)
     * @return Squared length of vector
     */
    double magnitude_squared() const;

    /**
     * @brief Normalize vector to unit length
     * @return Normalized point/vector
     */
    Point normalize() const;

    /**
     * @brief Rotate point around origin by angle (radians)
     * @param angle Rotation angle in radians
     * @return Rotated point
     */
    Point rotate(double angle) const;

    /**
     * @brief Rotate point around another point by angle (radians)
     * @param center Center of rotation
     * @param angle Rotation angle in radians
     * @return Rotated point
     */
    Point rotate_around(const Point& center, double angle) const;

    /**
     * @brief Calculate angle from this point to another (radians)
     * @param other Target point
     * @return Angle in radians
     */
    double angle_to(const Point& other) const;

    /**
     * @brief Check if point is approximately zero
     * @return true if point is within tolerance of origin
     */
    bool is_zero() const;

    /**
     * @brief Get string representation
     * @return String representation of point
     */
    std::string to_string() const;

    /**
     * @brief Stream output operator
     */
    friend std::ostream& operator<<(std::ostream& os, const Point& point);
};

/**
 * @brief Hash function for Point (for use in std::unordered_map, etc.)
 */
struct PointHash {
    std::size_t operator()(const Point& point) const;
};

/**
 * @brief Calculate distance between two points
 * @param p1 First point
 * @param p2 Second point
 * @return Distance between points
 */
double distance(const Point& p1, const Point& p2);

/**
 * @brief Calculate midpoint between two points
 * @param p1 First point
 * @param p2 Second point
 * @return Midpoint
 */
Point midpoint(const Point& p1, const Point& p2);

/**
 * @brief Calculate angle between three points (p1-p2-p3)
 * @param p1 First point
 * @param p2 Vertex point
 * @param p3 Third point
 * @return Angle in radians
 */
double angle_between_points(const Point& p1, const Point& p2, const Point& p3);

/**
 * @brief Check if three points are collinear
 * @param p1 First point
 * @param p2 Second point
 * @param p3 Third point
 * @return true if points are collinear within tolerance
 */
bool are_collinear(const Point& p1, const Point& p2, const Point& p3);

/**
 * @brief Calculate orientation of three points
 * @param p1 First point
 * @param p2 Second point
 * @param p3 Third point
 * @return 0 if collinear, 1 if clockwise, 2 if counterclockwise
 */
int orientation(const Point& p1, const Point& p2, const Point& p3);

} // namespace geometry
} // namespace zlayout