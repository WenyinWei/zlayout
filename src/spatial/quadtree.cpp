/**
 * @file quadtree.cpp
 * @brief Implementation of QuadTree non-template functions
 */

#include <zlayout/spatial/quadtree.hpp>
#include <zlayout/geometry/point.hpp>

namespace zlayout {
namespace spatial {

// QuadTree template method implementations (continued from header)

template<typename T>
QuadTree<T>::QuadTree(const geometry::Rectangle& boundary,
                      BoundingBoxFunc get_bbox,
                      size_t capacity,
                      size_t max_depth)
    : get_bounding_box(get_bbox), object_count(0), capacity(capacity), max_depth(max_depth) {
    root = std::make_unique<QuadTreeNode<T>>(boundary, capacity, max_depth, 0);
}

template<typename T>
bool QuadTree<T>::insert(const T& object) {
    if (root->insert(object, get_bounding_box)) {
        object_count++;
        return true;
    }
    return false;
}

template<typename T>
std::vector<T> QuadTree<T>::query_range(const geometry::Rectangle& range) const {
    return root->query_range(range, get_bounding_box);
}

template<typename T>
std::vector<T> QuadTree<T>::query_point(const geometry::Point& point) const {
    return root->query_point(point, get_bounding_box);
}

template<typename T>
std::vector<T> QuadTree<T>::query_nearby(const T& target, double distance) const {
    geometry::Rectangle target_bbox = get_bounding_box(target);
    geometry::Rectangle search_area = target_bbox.expand(distance);
    
    auto candidates = query_range(search_area);
    std::vector<T> result;
    
    for (const auto& candidate : candidates) {
        geometry::Rectangle candidate_bbox = get_bounding_box(candidate);
        if (target_bbox.distance_to(candidate_bbox) <= distance) {
            result.push_back(candidate);
        }
    }
    
    return result;
}

template<typename T>
std::vector<std::pair<T, T>> QuadTree<T>::find_potential_intersections() const {
    std::vector<std::pair<T, T>> pairs;
    collect_intersection_pairs(root.get(), pairs);
    return pairs;
}

template<typename T>
std::vector<std::pair<T, T>> QuadTree<T>::find_intersections(
    std::function<bool(const T&, const T&)> collision_func) const {
    
    auto potential_pairs = find_potential_intersections();
    std::vector<std::pair<T, T>> result;
    
    for (const auto& pair : potential_pairs) {
        if (collision_func(pair.first, pair.second)) {
            result.push_back(pair);
        }
    }
    
    return result;
}

template<typename T>
void QuadTree<T>::clear() {
    root->clear();
    object_count = 0;
}

template<typename T>
typename QuadTree<T>::Statistics QuadTree<T>::get_statistics() const {
    Statistics stats = {0, 0, 0, 0, 0.0, 0.0};
    calculate_statistics_recursive(root.get(), stats);
    
    if (stats.leaf_nodes > 0) {
        stats.average_objects_per_leaf = static_cast<double>(stats.total_objects) / stats.leaf_nodes;
    }
    
    if (stats.total_nodes > 0) {
        stats.tree_efficiency = static_cast<double>(stats.total_objects) / stats.total_nodes;
    }
    
    return stats;
}

template<typename T>
void QuadTree<T>::rebuild(size_t new_capacity, size_t new_max_depth) {
    // Get all objects
    auto all_objects = root->get_all_objects();
    
    // Create new root with new parameters
    capacity = new_capacity;
    max_depth = new_max_depth;
    root = std::make_unique<QuadTreeNode<T>>(root->boundary, capacity, max_depth, 0);
    object_count = 0;
    
    // Re-insert all objects
    for (const auto& object : all_objects) {
        insert(object);
    }
}

template<typename T>
void QuadTree<T>::collect_intersection_pairs(const QuadTreeNode<T>* node,
                                            std::vector<std::pair<T, T>>& pairs) const {
    if (!node) return;
    
    // Check pairs within this node
    for (size_t i = 0; i < node->objects.size(); ++i) {
        for (size_t j = i + 1; j < node->objects.size(); ++j) {
            pairs.emplace_back(node->objects[i], node->objects[j]);
        }
    }
    
    // If subdivided, recursively check child nodes
    if (node->divided) {
        for (int i = 0; i < 4; ++i) {
            collect_intersection_pairs(node->children[i].get(), pairs);
        }
        
        // Check pairs between different child nodes
        for (int i = 0; i < 4; ++i) {
            for (int j = i + 1; j < 4; ++j) {
                auto objects_i = node->children[i]->get_all_objects();
                auto objects_j = node->children[j]->get_all_objects();
                
                for (const auto& obj_i : objects_i) {
                    for (const auto& obj_j : objects_j) {
                        // Only add if bounding boxes might intersect
                        geometry::Rectangle bbox_i = get_bounding_box(obj_i);
                        geometry::Rectangle bbox_j = get_bounding_box(obj_j);
                        if (bbox_i.intersects(bbox_j)) {
                            pairs.emplace_back(obj_i, obj_j);
                        }
                    }
                }
            }
        }
    }
}

template<typename T>
void QuadTree<T>::calculate_statistics_recursive(const QuadTreeNode<T>* node,
                                                 Statistics& stats) const {
    if (!node) return;
    
    stats.total_nodes++;
    stats.total_objects += node->objects.size();
    stats.max_depth_reached = std::max(stats.max_depth_reached, node->depth);
    
    if (!node->divided) {
        stats.leaf_nodes++;
    } else {
        for (int i = 0; i < 4; ++i) {
            calculate_statistics_recursive(node->children[i].get(), stats);
        }
    }
}

// Specialized QuadTree factory functions

std::unique_ptr<RectangleQuadTree> create_rectangle_quadtree(
    const geometry::Rectangle& boundary,
    size_t capacity,
    size_t max_depth) {
    
    auto bbox_func = [](const geometry::Rectangle& rect) -> geometry::Rectangle {
        return rect; // Rectangle is its own bounding box
    };
    
    return std::make_unique<RectangleQuadTree>(boundary, bbox_func, capacity, max_depth);
}

std::unique_ptr<PointQuadTree> create_point_quadtree(
    const geometry::Rectangle& boundary,
    size_t capacity,
    size_t max_depth) {
    
    auto bbox_func = [](const geometry::Point& point) -> geometry::Rectangle {
        return geometry::Rectangle(point.x, point.y, 0.0, 0.0); // Point as zero-area rectangle
    };
    
    return std::make_unique<PointQuadTree>(boundary, bbox_func, capacity, max_depth);
}

// Explicit template instantiations for common types
template class QuadTree<geometry::Rectangle>;
template class QuadTree<geometry::Point>;
template class QuadTreeNode<geometry::Rectangle>;
template class QuadTreeNode<geometry::Point>;

} // namespace spatial
} // namespace zlayout