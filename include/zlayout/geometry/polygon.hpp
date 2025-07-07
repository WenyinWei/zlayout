/**
 * @file polygon.hpp
 * @brief Polygon class for complex geometric shapes and EDA components
 */

#pragma once

#include <zlayout/geometry/point.hpp>
#include <zlayout/geometry/rectangle.hpp>
#include <vector>
#include <string>
#include <utility>

namespace zlayout {
namespace geometry {

/**
 * @class Polygon
 * @brief Polygon class supporting both convex and concave polygons
 * 
 * Represents a polygon in 2D space with arbitrary vertex count.
 * Supports complex geometric operations, sharp angle detection,
 * and various polygon analysis functions essential for EDA applications.
 */
class Polygon {
public:
    std::vector<Point> vertices; ///< Polygon vertices in order

    /**
     * @brief Default constructor - creates empty polygon
     */
    Polygon() = default;

    /**
     * @brief Constructor from vertex list
     * @param vertices Vector of polygon vertices
     */
    explicit Polygon(const std::vector<Point>& vertices);

    /**
     * @brief Constructor from initializer list
     * @param vertices Initializer list of vertices
     */
    Polygon(std::initializer_list<Point> vertices);

    /**
     * @brief Copy constructor
     */
    Polygon(const Polygon& other) = default;

    /**
     * @brief Move constructor
     */
    Polygon(Polygon&& other) noexcept = default;

    /**
     * @brief Assignment operator
     */
    Polygon& operator=(const Polygon& other) = default;

    /**
     * @brief Move assignment operator
     */
    Polygon& operator=(Polygon&& other) noexcept = default;

    /**
     * @brief Equality operator
     */
    bool operator==(const Polygon& other) const;

    /**
     * @brief Inequality operator
     */
    bool operator!=(const Polygon& other) const;

    // Basic properties

    /**
     * @brief Get number of vertices
     */
    size_t vertex_count() const { return vertices.size(); }

    /**
     * @brief Check if polygon is empty
     */
    bool is_empty() const { return vertices.empty(); }

    /**
     * @brief Check if polygon is valid (at least 3 vertices)
     */
    bool is_valid() const { return vertices.size() >= 3; }

    /**
     * @brief Get polygon edges as pairs of consecutive vertices
     * @return Vector of edge pairs (start_point, end_point)
     */
    std::vector<std::pair<Point, Point>> edges() const;

    /**
     * @brief Calculate polygon area using shoelace formula
     * @return Polygon area (positive for counterclockwise, negative for clockwise)
     */
    double area() const;

    /**
     * @brief Calculate signed area (preserves orientation)
     */
    double signed_area() const;

    /**
     * @brief Calculate polygon perimeter
     */
    double perimeter() const;

    /**
     * @brief Calculate polygon centroid
     */
    Point centroid() const;

    /**
     * @brief Calculate axis-aligned bounding box
     */
    Rectangle bounding_box() const;

    // Geometric properties

    /**
     * @brief Check if polygon is convex
     * @return true if polygon is convex
     */
    bool is_convex() const;

    /**
     * @brief Check if polygon is clockwise oriented
     * @return true if polygon vertices are in clockwise order
     */
    bool is_clockwise() const;

    /**
     * @brief Check if polygon is counterclockwise oriented
     */
    bool is_counterclockwise() const { return !is_clockwise(); }

    /**
     * @brief Check if polygon is simple (no self-intersections)
     * @return true if polygon has no self-intersections
     */
    bool is_simple() const;

    // Point containment

    /**
     * @brief Check if point is inside polygon (ray casting algorithm)
     * @param point Point to test
     * @return true if point is inside polygon
     */
    bool contains_point(const Point& point) const;

    /**
     * @brief Check if point is on polygon boundary
     * @param point Point to test
     * @param tolerance Distance tolerance for boundary check
     * @return true if point is on polygon boundary
     */
    bool point_on_boundary(const Point& point, double tolerance = Point::TOLERANCE) const;

    // Sharp angle detection (面试题1)

    /**
     * @brief Find vertices with sharp angles
     * @param threshold_degrees Angle threshold in degrees (angles less than this are considered sharp)
     * @return Vector of vertex indices with sharp angles
     */
    std::vector<size_t> get_sharp_angles(double threshold_degrees = 30.0) const;

    /**
     * @brief Calculate angle at a specific vertex
     * @param vertex_index Index of vertex to calculate angle for
     * @return Angle in degrees (0-180)
     */
    double vertex_angle(size_t vertex_index) const;

    /**
     * @brief Get all vertex angles
     * @return Vector of angles in degrees for each vertex
     */
    std::vector<double> all_vertex_angles() const;

    // Distance calculations (面试题2)

    /**
     * @brief Calculate minimum distance to another polygon
     * @param other Target polygon
     * @return Minimum distance between polygons
     */
    double distance_to(const Polygon& other) const;

    /**
     * @brief Calculate minimum distance to a point
     * @param point Target point
     * @return Minimum distance to point
     */
    double distance_to(const Point& point) const;

    /**
     * @brief Calculate minimum distance to a line segment
     * @param line_start Start point of line segment
     * @param line_end End point of line segment
     * @return Minimum distance to line segment
     */
    double distance_to_line(const Point& line_start, const Point& line_end) const;

    /**
     * @brief Find closest point on polygon boundary to given point
     * @param point Target point
     * @return Closest point on polygon boundary
     */
    Point closest_point_to(const Point& point) const;

    // Edge operations

    /**
     * @brief Calculate minimum distance between this polygon's edges and another polygon's edges
     * @param other Target polygon
     * @return Minimum edge-to-edge distance
     */
    double min_edge_distance_to(const Polygon& other) const;

    /**
     * @brief Find narrow regions where edges are too close
     * @param other Target polygon
     * @param threshold_distance Maximum allowed distance
     * @return Vector of (point1, point2, distance) for narrow regions
     */
    std::vector<std::tuple<Point, Point, double>> find_narrow_regions(
        const Polygon& other, double threshold_distance) const;

    // Intersection detection (面试题3)

    /**
     * @brief Check if this polygon intersects with another polygon
     * @param other Target polygon
     * @return true if polygons intersect
     */
    bool intersects(const Polygon& other) const;

    /**
     * @brief Find intersection points between polygon edges
     * @param other Target polygon
     * @return Vector of intersection points
     */
    std::vector<Point> intersection_points(const Polygon& other) const;

    /**
     * @brief Check if polygon edges intersect (for self-intersection detection)
     * @return true if polygon has self-intersecting edges
     */
    bool has_self_intersections() const;

    // Transformations

    /**
     * @brief Translate polygon by given offset
     * @param offset Translation vector
     * @return Translated polygon
     */
    Polygon translate(const Point& offset) const;

    /**
     * @brief Rotate polygon around origin by given angle
     * @param angle Rotation angle in radians
     * @return Rotated polygon
     */
    Polygon rotate(double angle) const;

    /**
     * @brief Rotate polygon around a specific center point
     * @param center Center of rotation
     * @param angle Rotation angle in radians
     * @return Rotated polygon
     */
    Polygon rotate_around(const Point& center, double angle) const;

    /**
     * @brief Scale polygon by given factor around its centroid
     * @param factor Scale factor
     * @return Scaled polygon
     */
    Polygon scale(double factor) const;

    /**
     * @brief Scale polygon by different factors in X and Y
     * @param x_factor X scale factor
     * @param y_factor Y scale factor
     * @return Scaled polygon
     */
    Polygon scale(double x_factor, double y_factor) const;

    /**
     * @brief Reverse vertex order (change orientation)
     * @return Polygon with reversed vertex order
     */
    Polygon reverse() const;

    // Polygon operations

    /**
     * @brief Simplify polygon by removing collinear vertices
     * @param tolerance Tolerance for collinearity check
     * @return Simplified polygon
     */
    Polygon simplify(double tolerance = Point::TOLERANCE) const;

    /**
     * @brief Ensure polygon has counterclockwise orientation
     * @return Polygon with counterclockwise orientation
     */
    Polygon ensure_counterclockwise() const;

    /**
     * @brief Ensure polygon has clockwise orientation
     * @return Polygon with clockwise orientation
     */
    Polygon ensure_clockwise() const;

    // Utility functions

    /**
     * @brief Add vertex to polygon
     * @param vertex Vertex to add
     */
    void add_vertex(const Point& vertex);

    /**
     * @brief Insert vertex at specific index
     * @param index Index to insert at
     * @param vertex Vertex to insert
     */
    void insert_vertex(size_t index, const Point& vertex);

    /**
     * @brief Remove vertex at specific index
     * @param index Index of vertex to remove
     */
    void remove_vertex(size_t index);

    /**
     * @brief Clear all vertices
     */
    void clear() { vertices.clear(); }

    /**
     * @brief Get string representation
     */
    std::string to_string() const;

    /**
     * @brief Stream output operator
     */
    friend std::ostream& operator<<(std::ostream& os, const Polygon& polygon);

    // Static utility functions

    /**
     * @brief Create regular polygon with given center, radius, and vertex count
     * @param center Center point
     * @param radius Radius from center to vertices
     * @param vertex_count Number of vertices
     * @return Regular polygon
     */
    static Polygon regular_polygon(const Point& center, double radius, size_t vertex_count);

    /**
     * @brief Create polygon from rectangle
     * @param rect Rectangle to convert
     * @return Polygon with rectangle vertices
     */
    static Polygon from_rectangle(const Rectangle& rect);

    // Static utility functions for geometry calculations
    
    /**
     * @brief Helper function to calculate distance between two line segments
     */
    static double segment_to_segment_distance(
        const Point& seg1_start, const Point& seg1_end,
        const Point& seg2_start, const Point& seg2_end);

    /**
     * @brief Helper function to find intersection point of two line segments
     */
    static Point line_segment_intersection(
        const Point& seg1_start, const Point& seg1_end,
        const Point& seg2_start, const Point& seg2_end,
        bool& intersects);

    /**
     * @brief Helper function to check if two line segments intersect
     */
    static bool segments_intersect(
        const Point& seg1_start, const Point& seg1_end,
        const Point& seg2_start, const Point& seg2_end);

private:
};

/**
 * @brief Hash function for Polygon (for use in std::unordered_map, etc.)
 */
struct PolygonHash {
    std::size_t operator()(const Polygon& polygon) const;
};

// Utility functions for line segment operations

/**
 * @brief Calculate intersection point of two infinite lines
 * @param line1_start First point on line 1
 * @param line1_end Second point on line 1
 * @param line2_start First point on line 2
 * @param line2_end Second point on line 2
 * @param intersection Output intersection point
 * @return true if lines intersect (not parallel)
 */
bool line_intersection(const Point& line1_start, const Point& line1_end,
                      const Point& line2_start, const Point& line2_end,
                      Point& intersection);

/**
 * @brief Check if two line segments intersect
 * @param seg1_start Start of first segment
 * @param seg1_end End of first segment
 * @param seg2_start Start of second segment
 * @param seg2_end End of second segment
 * @return true if segments intersect
 */
bool segments_intersect(const Point& seg1_start, const Point& seg1_end,
                       const Point& seg2_start, const Point& seg2_end);

/**
 * @brief Calculate angle between two vectors in degrees
 * @param v1 First vector
 * @param v2 Second vector
 * @return Angle in degrees (0-180)
 */
double angle_between_vectors(const Point& v1, const Point& v2);

} // namespace geometry
} // namespace zlayout