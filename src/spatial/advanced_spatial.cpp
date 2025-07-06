/**
 * @file advanced_spatial.cpp
 * @brief Implementation of advanced spatial indexing for ultra-large scale EDA layouts
 */

#include <zlayout/spatial/advanced_spatial.hpp>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <stdexcept>

namespace zlayout {
namespace spatial {

// ThreadPool implementation
ThreadPool::ThreadPool(size_t threads) : stop(false) {
    for (size_t i = 0; i < threads; ++i) {
        workers.emplace_back([this] {
            for (;;) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
                    
                    if (this->stop && this->tasks.empty()) return;
                    
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread& worker : workers) {
        worker.join();
    }
}

template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;
    
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );
    
    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        
        if (stop) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }
        
        tasks.emplace([task]() { (*task)(); });
    }
    condition.notify_one();
    return res;
}

void ThreadPool::wait_for_completion() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    condition.wait(lock, [this] { return tasks.empty(); });
}

// R-tree implementation
template<typename T>
void RTree<T>::insert(const T& object, const geometry::Rectangle& bbox) {
    if (root->is_full() && !root->is_leaf) {
        // Split root
        auto new_root = std::make_unique<RTreeNode<T>>(false);
        new_root->children.push_back(std::move(root));
        auto sibling = split_node(new_root->children[0].get());
        new_root->children.push_back(std::move(sibling));
        new_root->update_mbr();
        root = std::move(new_root);
    }
    
    insert_recursive(root.get(), object, bbox);
    object_count++;
}

template<typename T>
void RTree<T>::insert_recursive(RTreeNode<T>* node, const T& object, const geometry::Rectangle& bbox) {
    if (node->is_leaf) {
        node->entries.emplace_back(object, bbox);
        node->update_mbr();
        return;
    }
    
    // Find child with minimum area increase
    double min_increase = std::numeric_limits<double>::max();
    size_t best_child = 0;
    
    for (size_t i = 0; i < node->children.size(); ++i) {
        double increase = calculate_area_increase(node->children[i]->mbr, bbox);
        if (increase < min_increase) {
            min_increase = increase;
            best_child = i;
        }
    }
    
    // Check if child needs splitting
    if (node->children[best_child]->is_full()) {
        auto sibling = split_node(node->children[best_child].get());
        node->children.push_back(std::move(sibling));
        
        // Recalculate best child after split
        min_increase = std::numeric_limits<double>::max();
        best_child = 0;
        
        for (size_t i = 0; i < node->children.size(); ++i) {
            double increase = calculate_area_increase(node->children[i]->mbr, bbox);
            if (increase < min_increase) {
                min_increase = increase;
                best_child = i;
            }
        }
    }
    
    insert_recursive(node->children[best_child].get(), object, bbox);
    node->update_mbr();
}

template<typename T>
std::unique_ptr<RTreeNode<T>> RTree<T>::split_node(RTreeNode<T>* node) {
    auto new_node = std::make_unique<RTreeNode<T>>(node->is_leaf);
    
    if (node->is_leaf) {
        // Split leaf node entries
        size_t mid = node->entries.size() / 2;
        new_node->entries.assign(node->entries.begin() + mid, node->entries.end());
        node->entries.erase(node->entries.begin() + mid, node->entries.end());
    } else {
        // Split internal node children
        size_t mid = node->children.size() / 2;
        new_node->children.assign(
            std::make_move_iterator(node->children.begin() + mid),
            std::make_move_iterator(node->children.end())
        );
        node->children.erase(node->children.begin() + mid, node->children.end());
    }
    
    node->update_mbr();
    new_node->update_mbr();
    
    return new_node;
}

template<typename T>
std::vector<T> RTree<T>::query_range(const geometry::Rectangle& range) const {
    std::vector<T> result;
    query_recursive(root.get(), range, result);
    return result;
}

template<typename T>
void RTree<T>::query_recursive(const RTreeNode<T>* node, const geometry::Rectangle& range, std::vector<T>& result) const {
    if (!node->mbr.intersects(range)) {
        return;
    }
    
    if (node->is_leaf) {
        for (const auto& [object, bbox] : node->entries) {
            if (bbox.intersects(range)) {
                result.push_back(object);
            }
        }
    } else {
        for (const auto& child : node->children) {
            query_recursive(child.get(), range, result);
        }
    }
}

template<typename T>
std::vector<T> RTree<T>::query_point(const geometry::Point& point) const {
    geometry::Rectangle point_rect(point.x, point.y, 0, 0);
    return query_range(point_rect);
}

template<typename T>
double RTree<T>::calculate_area_increase(const geometry::Rectangle& current, const geometry::Rectangle& new_rect) const {
    double current_area = current.area();
    double union_area = current.union_with(new_rect).area();
    return union_area - current_area;
}

template<typename T>
void RTree<T>::clear() {
    root = std::make_unique<RTreeNode<T>>(true);
    object_count = 0;
}

template<typename T>
bool RTree<T>::remove(const T& object, const geometry::Rectangle& bbox) {
    // Implementation of removal is complex and would require rebalancing
    // For now, we'll implement a simple version
    // In production, you'd want a more sophisticated removal algorithm
    return false;  // Not implemented yet
}

// HierarchicalSpatialIndex implementation
template<typename T>
void HierarchicalSpatialIndex<T>::create_ip_block(const std::string& name, 
                                                  const geometry::Rectangle& boundary, 
                                                  const std::string& parent_name) {
    IPBlock* parent = find_block(parent_name);
    if (!parent) {
        throw std::runtime_error("Parent block not found: " + parent_name);
    }
    
    auto new_block = std::make_unique<IPBlock>(name, boundary, parent->level + 1);
    parent->add_sub_block(std::move(new_block));
    
    // Create spatial index for this block
    create_block_index(name, boundary);
}

template<typename T>
void HierarchicalSpatialIndex<T>::create_block_index(const std::string& block_name, 
                                                     const geometry::Rectangle& boundary) {
    // Create both QuadTree and R-tree indices for comparison
    auto bbox_func = [](const T& obj) -> geometry::Rectangle {
        // This assumes T has a bounding_box() method or is itself a Rectangle
        if constexpr (std::is_same_v<T, geometry::Rectangle>) {
            return obj;
        } else {
            return obj.bounding_box();
        }
    };
    
    block_indices[block_name] = std::make_unique<QuadTree<T>>(boundary, bbox_func, 100, 8);
    rtree_indices[block_name] = std::make_unique<RTree<T>>();
}

template<typename T>
IPBlock* HierarchicalSpatialIndex<T>::find_block(const std::string& name) const {
    if (name == "root") {
        return root_block.get();
    }
    
    // Simple recursive search - in production, you'd want a hash map
    std::function<IPBlock*(IPBlock*, const std::string&)> search = 
        [&](IPBlock* block, const std::string& target_name) -> IPBlock* {
        if (block->name == target_name) {
            return block;
        }
        for (const auto& sub_block : block->sub_blocks) {
            if (auto found = search(sub_block.get(), target_name)) {
                return found;
            }
        }
        return nullptr;
    };
    
    return search(root_block.get(), name);
}

template<typename T>
std::vector<std::pair<T, T>> HierarchicalSpatialIndex<T>::parallel_find_intersections() const {
    std::vector<std::future<std::vector<std::pair<T, T>>>> futures;
    
    // Dispatch intersection finding to different threads for each block
    for (const auto& [block_name, index] : block_indices) {
        auto future = thread_pool.enqueue([&index]() {
            return index->find_potential_intersections();
        });
        futures.push_back(std::move(future));
    }
    
    // Collect results
    std::vector<std::pair<T, T>> result;
    for (auto& future : futures) {
        auto block_intersections = future.get();
        result.insert(result.end(), block_intersections.begin(), block_intersections.end());
    }
    
    return result;
}

template<typename T>
std::vector<std::future<std::vector<T>>> HierarchicalSpatialIndex<T>::dispatch_parallel_queries(
    const geometry::Rectangle& range) const {
    
    std::vector<std::future<std::vector<T>>> futures;
    
    for (const auto& [block_name, index] : block_indices) {
        // Check if block intersects with query range
        IPBlock* block = find_block(block_name);
        if (block && block->intersects(range)) {
            auto future = thread_pool.enqueue([&index, range]() {
                return index->query_range(range);
            });
            futures.push_back(std::move(future));
        }
    }
    
    return futures;
}

template<typename T>
void HierarchicalSpatialIndex<T>::optimize_hierarchy() {
    // Analyze current structure and optimize
    // This is a simplified version - real optimization would be much more complex
    
    std::function<void(IPBlock*)> optimize_block = [&](IPBlock* block) {
        // If block has too many components, create sub-blocks
        if (block->component_ids.size() > max_objects_per_block && 
            block->level < max_hierarchy_levels) {
            
            // Create 4 sub-blocks (quadrants)
            double half_width = block->boundary.width / 2.0;
            double half_height = block->boundary.height / 2.0;
            
            std::vector<geometry::Rectangle> quadrants = {
                geometry::Rectangle(block->boundary.x, block->boundary.y + half_height, 
                                  half_width, half_height),
                geometry::Rectangle(block->boundary.x + half_width, block->boundary.y + half_height, 
                                  half_width, half_height),
                geometry::Rectangle(block->boundary.x, block->boundary.y, 
                                  half_width, half_height),
                geometry::Rectangle(block->boundary.x + half_width, block->boundary.y, 
                                  half_width, half_height)
            };
            
            for (size_t i = 0; i < 4; ++i) {
                std::string sub_name = block->name + "_q" + std::to_string(i);
                auto sub_block = std::make_unique<IPBlock>(sub_name, quadrants[i], block->level + 1);
                block->add_sub_block(std::move(sub_block));
                create_block_index(sub_name, quadrants[i]);
            }
        }
        
        // Recursively optimize sub-blocks
        for (const auto& sub_block : block->sub_blocks) {
            optimize_block(sub_block.get());
        }
    };
    
    optimize_block(root_block.get());
}

template<typename T>
typename HierarchicalSpatialIndex<T>::Statistics HierarchicalSpatialIndex<T>::get_statistics() const {
    Statistics stats = {0, 0, 0, 0, 0.0, 0.0};
    
    // Count total objects and blocks
    std::function<void(const IPBlock*)> count_block = [&](const IPBlock* block) {
        stats.total_blocks++;
        stats.total_objects += block->component_ids.size();
        stats.max_depth = std::max(stats.max_depth, block->level);
        
        for (const auto& sub_block : block->sub_blocks) {
            count_block(sub_block.get());
        }
    };
    
    count_block(root_block.get());
    
    if (stats.total_blocks > 0) {
        stats.avg_objects_per_block = stats.total_objects / stats.total_blocks;
    }
    
    // Estimate memory usage (simplified)
    stats.memory_usage_mb = (stats.total_objects * sizeof(T) + 
                           stats.total_blocks * sizeof(IPBlock)) / (1024.0 * 1024.0);
    
    return stats;
}

template<typename T>
void HierarchicalSpatialIndex<T>::optimize_for_query_pattern(
    const std::vector<geometry::Rectangle>& query_patterns) {
    
    // Analyze query patterns and optimize index structure
    // This is a simplified implementation
    
    std::unordered_map<std::string, int> block_access_count;
    
    for (const auto& query_rect : query_patterns) {
        for (const auto& [block_name, index] : block_indices) {
            IPBlock* block = find_block(block_name);
            if (block && block->intersects(query_rect)) {
                block_access_count[block_name]++;
            }
        }
    }
    
    // Reorganize frequently accessed blocks for better performance
    // In a real implementation, you might:
    // 1. Adjust block boundaries based on query patterns
    // 2. Change the spatial index type for hot blocks
    // 3. Pre-cache results for common queries
    // 4. Adjust memory allocation patterns
}

// Explicit template instantiations for common types
template class RTree<geometry::Rectangle>;
template class RTree<geometry::Point>;
template class HierarchicalSpatialIndex<geometry::Rectangle>;
template class HierarchicalSpatialIndex<geometry::Point>;

} // namespace spatial
} // namespace zlayout
```