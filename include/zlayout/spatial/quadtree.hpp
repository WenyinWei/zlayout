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
#include <string>
#include <mutex>
#include <algorithm>

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
    bool collect_statistics;
    mutable std::mutex tree_mutex;  // For thread-safe operations

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

    /**
     * @brief Remove object from tree
     * @param object Object to remove
     * @return true if object was found and removed
     */
    bool remove(const T& object);

    /**
     * @brief Update object position (remove and re-insert)
     * @param old_object Object to update
     * @param new_object New object data
     * @return true if update was successful
     */
    bool update(const T& old_object, const T& new_object);

    /**
     * @brief Batch insert multiple objects efficiently
     * @param objects Vector of objects to insert
     * @return Number of objects successfully inserted
     */
    size_t batch_insert(const std::vector<T>& objects);

    /**
     * @brief Batch remove multiple objects efficiently
     * @param objects Vector of objects to remove
     * @return Number of objects successfully removed
     */
    size_t batch_remove(const std::vector<T>& objects);

    /**
     * @brief Iterator support for tree traversal
     */
    class Iterator {
    private:
        const QuadTree* tree;
        std::vector<T> objects;
        size_t current_index;
        
    public:
        Iterator(const QuadTree* tree, bool is_end = false);
        
        const T& operator*() const;
        const T* operator->() const;
        Iterator& operator++();
        bool operator!=(const Iterator& other) const;
        bool operator==(const Iterator& other) const;
    };

    /**
     * @brief Get iterator to beginning of tree
     */
    Iterator begin() const;

    /**
     * @brief Get iterator to end of tree
     */
    Iterator end() const;

    /**
     * @brief Check if object exists in tree
     * @param object Object to find
     * @return true if object exists
     */
    bool contains(const T& object) const;

    /**
     * @brief Get all objects in tree as vector
     * @return Vector containing all objects
     */
    std::vector<T> get_all_objects() const;

    /**
     * @brief Get objects within circular range
     * @param center Center point of search
     * @param radius Search radius
     * @return Vector of objects within radius
     */
    std::vector<T> query_circle(const geometry::Point& center, double radius) const;

    /**
     * @brief Get k nearest neighbors to target object
     * @param target Target object
     * @param k Number of neighbors to find
     * @return Vector of k nearest objects
     */
    std::vector<T> query_k_nearest(const T& target, size_t k) const;

    /**
     * @brief Optimize tree structure for better performance
     * This rebuilds the tree with better parameters based on current data
     */
    void optimize();

    /**
     * @brief Get detailed tree statistics
     */
    struct DetailedStatistics {
        size_t total_nodes;
        size_t leaf_nodes;
        size_t internal_nodes;
        size_t max_depth_reached;
        size_t min_depth_reached;
        size_t total_objects;
        double average_objects_per_leaf;
        double tree_efficiency;
        double memory_usage_bytes;
        double fill_factor;  // percentage of nodes that are at capacity
        std::vector<size_t> objects_per_level;  // objects at each depth level
    };

    /**
     * @brief Get detailed statistics about tree structure
     */
    DetailedStatistics get_detailed_statistics() const;

    /**
     * @brief Merge another quadtree into this one
     * @param other QuadTree to merge
     * @return true if merge was successful
     */
    bool merge(const QuadTree& other);

    /**
     * @brief Split tree into multiple subtrees based on spatial regions
     * @param regions Vector of rectangular regions to split into
     * @return Vector of new QuadTrees for each region
     */
    std::vector<std::unique_ptr<QuadTree>> split_by_regions(
        const std::vector<geometry::Rectangle>& regions) const;

    /**
     * @brief Validate tree structure integrity
     * @return true if tree structure is valid
     */
    bool validate() const;

    /**
     * @brief Get tree as string representation (for debugging)
     */
    std::string to_string() const;

    /**
     * @brief Enable/disable statistics collection
     * @param enable Whether to collect detailed statistics
     */
    void set_statistics_collection(bool enable) { collect_statistics = enable; }

    /**
     * @brief Get current load factor of the tree
     * @return Load factor (0.0 to 1.0)
     */
    double get_load_factor() const;

private:
    /**
     * @brief Helper function to remove object from node
     */
    bool remove_from_node(QuadTreeNode<T>* node, const T& object);
    
    /**
     * @brief Helper function to collect all objects recursively
     */
    void collect_all_objects(const QuadTreeNode<T>* node, std::vector<T>& objects) const;
    
    /**
     * @brief Helper function to find k nearest neighbors
     */
    void find_k_nearest_recursive(const QuadTreeNode<T>* node, const T& target, 
                                  size_t k, std::vector<std::pair<T, double>>& candidates) const;
    
    /**
     * @brief Helper function to calculate detailed statistics
     */
    void calculate_detailed_statistics_recursive(const QuadTreeNode<T>* node,
                                                DetailedStatistics& stats) const;
    
    /**
     * @brief Helper function to validate tree structure
     */
    bool validate_node(const QuadTreeNode<T>* node) const;
    
    /**
     * @brief Helper function to convert tree to string
     */
    void node_to_string(const QuadTreeNode<T>* node, std::string& result, 
                       const std::string& indent = "") const;
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

template<typename T>
QuadTree<T>::QuadTree(const geometry::Rectangle& boundary,
                     BoundingBoxFunc get_bbox,
                     size_t capacity,
                     size_t max_depth)
    : root(std::make_unique<QuadTreeNode<T>>(boundary, capacity, max_depth, 0)),
      get_bounding_box(get_bbox),
      object_count(0),
      capacity(capacity),
      max_depth(max_depth),
      collect_statistics(true) {
    // Initialize other members
}

template<typename T>
bool QuadTree<T>::insert(const T& object) {
    std::lock_guard<std::mutex> lock(tree_mutex);
    
    if (root->insert(object, get_bounding_box)) {
        object_count++;
        return true;
    }
    return false;
}

template<typename T>
std::vector<T> QuadTree<T>::query_range(const geometry::Rectangle& range) const {
    std::lock_guard<std::mutex> lock(tree_mutex);
    
    return root->query_range(range, get_bounding_box);
}

template<typename T>
std::vector<T> QuadTree<T>::query_point(const geometry::Point& point) const {
    std::lock_guard<std::mutex> lock(tree_mutex);
    
    return root->query_point(point, get_bounding_box);
}

template<typename T>
std::vector<T> QuadTree<T>::query_nearby(const T& target, double distance) const {
    // Implementation of query_nearby method
    // This is a placeholder and should be implemented
    return std::vector<T>();
}

template<typename T>
std::vector<std::pair<T, T>> QuadTree<T>::find_potential_intersections() const {
    // Implementation of find_potential_intersections method
    // This is a placeholder and should be implemented
    return std::vector<std::pair<T, T>>();
}

template<typename T>
std::vector<std::pair<T, T>> QuadTree<T>::find_intersections(
    std::function<bool(const T&, const T&)> collision_func) const {
    // Implementation of find_intersections method
    // This is a placeholder and should be implemented
    return std::vector<std::pair<T, T>>();
}

template<typename T>
void QuadTree<T>::clear() {
    std::lock_guard<std::mutex> lock(tree_mutex);
    
    root->clear();
    object_count = 0;
}

template<typename T>
typename QuadTree<T>::Statistics QuadTree<T>::get_statistics() const {
    std::lock_guard<std::mutex> lock(tree_mutex);
    
    Statistics stats = {};
    stats.total_nodes = 1;  // Assuming a single root node
    stats.total_objects = object_count;
    stats.max_depth_reached = 1;  // Assuming a single level tree
    
    return stats;
}

template<typename T>
void QuadTree<T>::rebuild(size_t new_capacity, size_t new_max_depth) {
    // Implementation of rebuild method
    // This is a placeholder and should be implemented
}

template<typename T>
bool QuadTree<T>::remove(const T& object) {
    std::lock_guard<std::mutex> lock(tree_mutex);
    
    if (remove_from_node(root.get(), object)) {
        object_count--;
        return true;
    }
    return false;
}

template<typename T>
bool QuadTree<T>::remove_from_node(QuadTreeNode<T>* node, const T& object) {
    if (!node) return false;
    
    // Try to remove from this node's objects
    auto it = std::find(node->objects.begin(), node->objects.end(), object);
    if (it != node->objects.end()) {
        node->objects.erase(it);
        return true;
    }
    
    // Try to remove from children
    if (node->divided) {
        for (int i = 0; i < 4; ++i) {
            if (remove_from_node(node->children[i].get(), object)) {
                return true;
            }
        }
    }
    
    return false;
}

template<typename T>
bool QuadTree<T>::update(const T& old_object, const T& new_object) {
    std::lock_guard<std::mutex> lock(tree_mutex);
    
    if (remove(old_object)) {
        return insert(new_object);
    }
    return false;
}

template<typename T>
size_t QuadTree<T>::batch_insert(const std::vector<T>& objects) {
    std::lock_guard<std::mutex> lock(tree_mutex);
    
    size_t success_count = 0;
    for (const auto& object : objects) {
        if (root->insert(object, get_bounding_box)) {
            object_count++;
            success_count++;
        }
    }
    return success_count;
}

template<typename T>
size_t QuadTree<T>::batch_remove(const std::vector<T>& objects) {
    std::lock_guard<std::mutex> lock(tree_mutex);
    
    size_t success_count = 0;
    for (const auto& object : objects) {
        if (remove_from_node(root.get(), object)) {
            object_count--;
            success_count++;
        }
    }
    return success_count;
}

template<typename T>
typename QuadTree<T>::Iterator QuadTree<T>::begin() const {
    return Iterator(this, false);
}

template<typename T>
typename QuadTree<T>::Iterator QuadTree<T>::end() const {
    return Iterator(this, true);
}

template<typename T>
bool QuadTree<T>::contains(const T& object) const {
    geometry::Rectangle bbox = get_bounding_box(object);
    auto candidates = query_range(bbox);
    return std::find(candidates.begin(), candidates.end(), object) != candidates.end();
}

template<typename T>
std::vector<T> QuadTree<T>::get_all_objects() const {
    std::lock_guard<std::mutex> lock(tree_mutex);
    
    std::vector<T> result;
    collect_all_objects(root.get(), result);
    return result;
}

template<typename T>
void QuadTree<T>::collect_all_objects(const QuadTreeNode<T>* node, std::vector<T>& objects) const {
    if (!node) return;
    
    objects.insert(objects.end(), node->objects.begin(), node->objects.end());
    
    if (node->divided) {
        for (int i = 0; i < 4; ++i) {
            collect_all_objects(node->children[i].get(), objects);
        }
    }
}

template<typename T>
std::vector<T> QuadTree<T>::query_circle(const geometry::Point& center, double radius) const {
    // Query square region first, then filter by distance
    geometry::Rectangle search_area(center.x - radius, center.y - radius, 
                                   2 * radius, 2 * radius);
    auto candidates = query_range(search_area);
    
    std::vector<T> result;
    for (const auto& candidate : candidates) {
        geometry::Rectangle bbox = get_bounding_box(candidate);
        geometry::Point candidate_center = bbox.center();
        
        if (center.distance_to(candidate_center) <= radius) {
            result.push_back(candidate);
        }
    }
    
    return result;
}

template<typename T>
std::vector<T> QuadTree<T>::query_k_nearest(const T& target, size_t k) const {
    std::vector<std::pair<T, double>> candidates;
    find_k_nearest_recursive(root.get(), target, k, candidates);
    
    // Sort by distance and return top k
    std::sort(candidates.begin(), candidates.end(),
              [](const auto& a, const auto& b) { return a.second < b.second; });
    
    std::vector<T> result;
    for (size_t i = 0; i < std::min(k, candidates.size()); ++i) {
        result.push_back(candidates[i].first);
    }
    
    return result;
}

template<typename T>
void QuadTree<T>::find_k_nearest_recursive(const QuadTreeNode<T>* node, const T& target, 
                                          size_t k, std::vector<std::pair<T, double>>& candidates) const {
    if (!node) return;
    
    geometry::Rectangle target_bbox = get_bounding_box(target);
    
    // Check objects in this node
    for (const auto& object : node->objects) {
        if (object == target) continue;  // Skip self
        
        geometry::Rectangle obj_bbox = get_bounding_box(object);
        double distance = target_bbox.distance_to(obj_bbox);
        candidates.emplace_back(object, distance);
    }
    
    // Recursively check children
    if (node->divided) {
        for (int i = 0; i < 4; ++i) {
            find_k_nearest_recursive(node->children[i].get(), target, k, candidates);
        }
    }
}

template<typename T>
void QuadTree<T>::optimize() {
    auto stats = get_detailed_statistics();
    
    // Determine optimal parameters based on current data
    size_t optimal_capacity = std::max(1UL, object_count / stats.total_nodes);
    size_t optimal_depth = std::max(1UL, stats.max_depth_reached);
    
    // Rebuild with optimal parameters
    rebuild(optimal_capacity, optimal_depth);
}

template<typename T>
typename QuadTree<T>::DetailedStatistics QuadTree<T>::get_detailed_statistics() const {
    DetailedStatistics stats = {};
    stats.objects_per_level.resize(20, 0);  // Assume max 20 levels
    
    calculate_detailed_statistics_recursive(root.get(), stats);
    
    if (stats.leaf_nodes > 0) {
        stats.average_objects_per_leaf = static_cast<double>(stats.total_objects) / stats.leaf_nodes;
    }
    
    if (stats.total_nodes > 0) {
        stats.tree_efficiency = static_cast<double>(stats.total_objects) / stats.total_nodes;
    }
    
    stats.memory_usage_bytes = stats.total_nodes * sizeof(QuadTreeNode<T>) + 
                              stats.total_objects * sizeof(T);
    
    return stats;
}

template<typename T>
void QuadTree<T>::calculate_detailed_statistics_recursive(const QuadTreeNode<T>* node,
                                                        DetailedStatistics& stats) const {
    if (!node) return;
    
    stats.total_nodes++;
    stats.total_objects += node->objects.size();
    stats.max_depth_reached = std::max(stats.max_depth_reached, node->depth);
    
    if (node->depth < stats.objects_per_level.size()) {
        stats.objects_per_level[node->depth] += node->objects.size();
    }
    
    if (!node->divided) {
        stats.leaf_nodes++;
        if (stats.min_depth_reached == 0 || node->depth < stats.min_depth_reached) {
            stats.min_depth_reached = node->depth;
        }
    } else {
        stats.internal_nodes++;
        for (int i = 0; i < 4; ++i) {
            calculate_detailed_statistics_recursive(node->children[i].get(), stats);
        }
    }
}

template<typename T>
bool QuadTree<T>::validate() const {
    return validate_node(root.get());
}

template<typename T>
bool QuadTree<T>::validate_node(const QuadTreeNode<T>* node) const {
    if (!node) return true;
    
    // Check that all objects fit within boundary
    for (const auto& object : node->objects) {
        geometry::Rectangle bbox = get_bounding_box(object);
        if (!node->boundary.intersects(bbox)) {
            return false;
        }
    }
    
    // Check children
    if (node->divided) {
        for (int i = 0; i < 4; ++i) {
            if (!validate_node(node->children[i].get())) {
                return false;
            }
        }
    }
    
    return true;
}

template<typename T>
std::string QuadTree<T>::to_string() const {
    std::string result = "QuadTree Structure:\n";
    node_to_string(root.get(), result);
    return result;
}

template<typename T>
void QuadTree<T>::node_to_string(const QuadTreeNode<T>* node, std::string& result, 
                                const std::string& indent) const {
    if (!node) return;
    
    result += indent + "Node: " + std::to_string(node->objects.size()) + " objects\n";
    
    if (node->divided) {
        for (int i = 0; i < 4; ++i) {
            result += indent + "  Child " + std::to_string(i) + ":\n";
            node_to_string(node->children[i].get(), result, indent + "    ");
        }
    }
}

template<typename T>
double QuadTree<T>::get_load_factor() const {
    if (object_count == 0) return 0.0;
    
    auto stats = get_statistics();
    size_t total_capacity = stats.total_nodes * capacity;
    return static_cast<double>(object_count) / total_capacity;
}

} // namespace spatial
} // namespace zlayout