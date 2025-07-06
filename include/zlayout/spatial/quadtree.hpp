/**
 * @file quadtree.hpp
 * @brief QuadTree spatial indexing for efficient geometric queries
 */

#pragma once

#include <zlayout/geometry/point.hpp>
#include <zlayout/geometry/rectangle.hpp>
#include <vector>
#include <memory>
#include <functional>

namespace zlayout {
namespace spatial {

/**
 * @class QuadTreeNode
 * @brief A node in the quadtree structure
 * 
 * Represents a single node in the quadtree hierarchy. Each node can contain
 * objects up to a specified capacity before subdividing into four child nodes.
 */
template<typename T>
class QuadTreeNode {
public:
    using ObjectType = T;
    using BoundingBoxFunc = std::function<geometry::Rectangle(const T&)>;

    geometry::Rectangle boundary;  ///< Spatial boundary of this node
    std::vector<T> objects;       ///< Objects stored in this node
    std::unique_ptr<QuadTreeNode> children[4]; ///< Child nodes (NW, NE, SW, SE)
    bool divided;                 ///< Whether this node has been subdivided
    size_t capacity;              ///< Maximum objects before subdivision
    size_t max_depth;             ///< Maximum subdivision depth
    size_t depth;                 ///< Current depth in tree

    /**
     * @brief Constructor
     * @param boundary Spatial boundary for this node
     * @param capacity Maximum objects before subdivision
     * @param max_depth Maximum depth for subdivision
     * @param depth Current depth (0 for root)
     */
    QuadTreeNode(const geometry::Rectangle& boundary, 
                 size_t capacity = 10, 
                 size_t max_depth = 8, 
                 size_t depth = 0);

    /**
     * @brief Insert object into this node or appropriate child
     * @param object Object to insert
     * @param get_bbox Function to get bounding box from object
     * @return true if successfully inserted
     */
    bool insert(const T& object, const BoundingBoxFunc& get_bbox);

    /**
     * @brief Query objects in a rectangular range
     * @param range Query rectangle
     * @param get_bbox Function to get bounding box from object
     * @return Vector of objects intersecting the range
     */
    std::vector<T> query_range(const geometry::Rectangle& range, 
                               const BoundingBoxFunc& get_bbox) const;

    /**
     * @brief Query objects containing a specific point
     * @param point Query point
     * @param get_bbox Function to get bounding box from object
     * @return Vector of objects containing the point
     */
    std::vector<T> query_point(const geometry::Point& point, 
                               const BoundingBoxFunc& get_bbox) const;

    /**
     * @brief Get total number of objects in this subtree
     */
    size_t size() const;

    /**
     * @brief Clear all objects and children
     */
    void clear();

    /**
     * @brief Check if this node has been subdivided
     */
    bool is_divided() const { return divided; }

    /**
     * @brief Get all objects in this subtree
     */
    std::vector<T> get_all_objects() const;

private:
    /**
     * @brief Subdivide this node into four children
     */
    void subdivide();

    /**
     * @brief Check if object's bounding box intersects with node boundary
     */
    bool intersects_boundary(const T& object, const BoundingBoxFunc& get_bbox) const;
};

/**
 * @class QuadTree
 * @brief Quadtree spatial index for efficient range and intersection queries
 * 
 * High-performance spatial data structure that partitions 2D space recursively
 * into quadrants. Optimizes range queries, nearest neighbor searches, and
 * intersection detection for large numbers of geometric objects.
 */
template<typename T>
class QuadTree {
public:
    using ObjectType = T;
    using BoundingBoxFunc = std::function<geometry::Rectangle(const T&)>;

private:
    std::unique_ptr<QuadTreeNode<T>> root;
    BoundingBoxFunc get_bounding_box;
    size_t object_count;
    size_t capacity;
    size_t max_depth;

public:
    /**
     * @brief Constructor
     * @param boundary World boundary for the quadtree
     * @param get_bbox Function to extract bounding box from objects
     * @param capacity Maximum objects per node before subdivision
     * @param max_depth Maximum subdivision depth
     */
    QuadTree(const geometry::Rectangle& boundary,
             BoundingBoxFunc get_bbox,
             size_t capacity = 10,
             size_t max_depth = 8);

    /**
     * @brief Insert object into quadtree
     * @param object Object to insert
     * @return true if successfully inserted
     */
    bool insert(const T& object);

    /**
     * @brief Query objects in rectangular range
     * @param range Query rectangle
     * @return Vector of objects intersecting the range
     */
    std::vector<T> query_range(const geometry::Rectangle& range) const;

    /**
     * @brief Query objects containing a specific point
     * @param point Query point
     * @return Vector of objects containing the point
     */
    std::vector<T> query_point(const geometry::Point& point) const;

    /**
     * @brief Find objects within distance of target object
     * @param target Target object
     * @param distance Maximum distance
     * @return Vector of nearby objects
     */
    std::vector<T> query_nearby(const T& target, double distance) const;

    /**
     * @brief Find all potentially intersecting pairs of objects
     * @return Vector of object pairs that might intersect
     */
    std::vector<std::pair<T, T>> find_potential_intersections() const;

    /**
     * @brief Find intersecting objects using efficient spatial queries
     * @param collision_func Function to test actual collision between objects
     * @return Vector of intersecting object pairs
     */
    std::vector<std::pair<T, T>> find_intersections(
        std::function<bool(const T&, const T&)> collision_func) const;

    /**
     * @brief Get total number of objects in tree
     */
    size_t size() const { return object_count; }

    /**
     * @brief Check if tree is empty
     */
    bool empty() const { return object_count == 0; }

    /**
     * @brief Clear all objects from tree
     */
    void clear();

    /**
     * @brief Get tree statistics for performance analysis
     */
    struct Statistics {
        size_t total_nodes;
        size_t leaf_nodes;
        size_t max_depth_reached;
        size_t total_objects;
        double average_objects_per_leaf;
        double tree_efficiency; // objects / total_nodes
    };

    /**
     * @brief Calculate tree statistics
     */
    Statistics get_statistics() const;

    /**
     * @brief Get root node (for visualization/debugging)
     */
    const QuadTreeNode<T>* get_root() const { return root.get(); }

    /**
     * @brief Rebuild tree with new parameters
     */
    void rebuild(size_t new_capacity, size_t new_max_depth);

private:
    /**
     * @brief Helper function to collect potential intersection pairs
     */
    void collect_intersection_pairs(const QuadTreeNode<T>* node,
                                   std::vector<std::pair<T, T>>& pairs) const;

    /**
     * @brief Helper function to calculate statistics recursively
     */
    void calculate_statistics_recursive(const QuadTreeNode<T>* node,
                                       Statistics& stats) const;
};

// Specialized QuadTree for common geometric types

/**
 * @brief QuadTree specialized for Rectangle objects
 */
using RectangleQuadTree = QuadTree<geometry::Rectangle>;

/**
 * @brief QuadTree specialized for Point objects (using Point as both object and bounding box)
 */
using PointQuadTree = QuadTree<geometry::Point>;

/**
 * @brief Create QuadTree for rectangles with default bounding box function
 */
std::unique_ptr<RectangleQuadTree> create_rectangle_quadtree(
    const geometry::Rectangle& boundary,
    size_t capacity = 10,
    size_t max_depth = 8);

/**
 * @brief Create QuadTree for points with default bounding box function
 */
std::unique_ptr<PointQuadTree> create_point_quadtree(
    const geometry::Rectangle& boundary,
    size_t capacity = 10,
    size_t max_depth = 8);

// Template implementation (must be in header for template instantiation)

template<typename T>
QuadTreeNode<T>::QuadTreeNode(const geometry::Rectangle& boundary, 
                              size_t capacity, 
                              size_t max_depth, 
                              size_t depth)
    : boundary(boundary), divided(false), capacity(capacity), 
      max_depth(max_depth), depth(depth) {
    objects.reserve(capacity);
}

template<typename T>
bool QuadTreeNode<T>::insert(const T& object, const BoundingBoxFunc& get_bbox) {
    // Check if object fits in this boundary
    if (!intersects_boundary(object, get_bbox)) {
        return false;
    }

    // If we have space and haven't subdivided, add it here
    if (objects.size() < capacity && !divided) {
        objects.push_back(object);
        return true;
    }

    // If we can still subdivide, do so
    if (!divided && depth < max_depth) {
        subdivide();
    }

    // Try to insert into children
    if (divided) {
        for (int i = 0; i < 4; ++i) {
            if (children[i]->insert(object, get_bbox)) {
                return true;
            }
        }
        // If no child could contain it, store it at this level
        objects.push_back(object);
        return true;
    } else {
        // No more subdivisions possible, store here
        objects.push_back(object);
        return true;
    }
}

template<typename T>
std::vector<T> QuadTreeNode<T>::query_range(const geometry::Rectangle& range, 
                                            const BoundingBoxFunc& get_bbox) const {
    std::vector<T> result;

    // Check if this node intersects with query range
    if (!boundary.intersects(range)) {
        return result;
    }

    // Check objects in this node
    for (const auto& object : objects) {
        geometry::Rectangle obj_bbox = get_bbox(object);
        if (obj_bbox.intersects(range)) {
            result.push_back(object);
        }
    }

    // Recursively check children
    if (divided) {
        for (int i = 0; i < 4; ++i) {
            auto child_results = children[i]->query_range(range, get_bbox);
            result.insert(result.end(), child_results.begin(), child_results.end());
        }
    }

    return result;
}

template<typename T>
std::vector<T> QuadTreeNode<T>::query_point(const geometry::Point& point, 
                                            const BoundingBoxFunc& get_bbox) const {
    std::vector<T> result;

    // Check if point is in this boundary
    if (!boundary.contains_point(point)) {
        return result;
    }

    // Check objects in this node
    for (const auto& object : objects) {
        geometry::Rectangle obj_bbox = get_bbox(object);
        if (obj_bbox.contains_point(point)) {
            result.push_back(object);
        }
    }

    // Recursively check children
    if (divided) {
        for (int i = 0; i < 4; ++i) {
            auto child_results = children[i]->query_point(point, get_bbox);
            result.insert(result.end(), child_results.begin(), child_results.end());
        }
    }

    return result;
}

template<typename T>
void QuadTreeNode<T>::subdivide() {
    double x = boundary.x;
    double y = boundary.y;
    double w = boundary.width / 2.0;
    double h = boundary.height / 2.0;

    // Create four child nodes (NW, NE, SW, SE)
    children[0] = std::make_unique<QuadTreeNode>(
        geometry::Rectangle(x, y + h, w, h), capacity, max_depth, depth + 1);
    children[1] = std::make_unique<QuadTreeNode>(
        geometry::Rectangle(x + w, y + h, w, h), capacity, max_depth, depth + 1);
    children[2] = std::make_unique<QuadTreeNode>(
        geometry::Rectangle(x, y, w, h), capacity, max_depth, depth + 1);
    children[3] = std::make_unique<QuadTreeNode>(
        geometry::Rectangle(x + w, y, w, h), capacity, max_depth, depth + 1);

    divided = true;
}

template<typename T>
bool QuadTreeNode<T>::intersects_boundary(const T& object, const BoundingBoxFunc& get_bbox) const {
    return boundary.intersects(get_bbox(object));
}

template<typename T>
size_t QuadTreeNode<T>::size() const {
    size_t count = objects.size();
    if (divided) {
        for (int i = 0; i < 4; ++i) {
            count += children[i]->size();
        }
    }
    return count;
}

template<typename T>
void QuadTreeNode<T>::clear() {
    objects.clear();
    if (divided) {
        for (int i = 0; i < 4; ++i) {
            children[i].reset();
        }
        divided = false;
    }
}

template<typename T>
std::vector<T> QuadTreeNode<T>::get_all_objects() const {
    std::vector<T> result = objects;
    if (divided) {
        for (int i = 0; i < 4; ++i) {
            auto child_objects = children[i]->get_all_objects();
            result.insert(result.end(), child_objects.begin(), child_objects.end());
        }
    }
    return result;
}

} // namespace spatial
} // namespace zlayout