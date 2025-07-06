/**
 * @file advanced_spatial.hpp
 * @brief Advanced spatial indexing for ultra-large scale EDA layouts
 * 
 * This module provides high-performance spatial indexing structures optimized for
 * billions of components in EDA layouts. Features include:
 * - Hierarchical spatial indexing with IP block support
 * - Multi-threaded parallel processing
 * - GPU acceleration (CUDA/OpenCL)
 * - Memory pool management
 * - Batch operations optimization
 * - Alternative algorithms (R-tree, Z-order curve)
 */

#pragma once

#include <zlayout/geometry/point.hpp>
#include <zlayout/geometry/rectangle.hpp>
#include <zlayout/spatial/quadtree.hpp>
#include <vector>
#include <memory>
#include <functional>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <queue>
#include <unordered_map>
#include <unordered_set>

#ifdef ZLAYOUT_USE_CUDA
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#endif

#ifdef ZLAYOUT_USE_OPENCL
#include <CL/cl.h>
#endif

namespace zlayout {
namespace spatial {

/**
 * @brief Memory pool for efficient object allocation
 */
template<typename T>
class MemoryPool {
private:
    struct Block {
        alignas(T) char data[sizeof(T)];
        Block* next;
    };
    
    std::vector<std::unique_ptr<Block[]>> chunks;
    Block* free_list;
    size_t chunk_size;
    std::mutex mutex;
    
public:
    explicit MemoryPool(size_t chunk_size = 1024) 
        : chunk_size(chunk_size), free_list(nullptr) {
        allocate_chunk();
    }
    
    ~MemoryPool() {
        clear();
    }
    
    T* allocate() {
        std::lock_guard<std::mutex> lock(mutex);
        if (!free_list) {
            allocate_chunk();
        }
        
        Block* block = free_list;
        free_list = free_list->next;
        return reinterpret_cast<T*>(block);
    }
    
    void deallocate(T* ptr) {
        std::lock_guard<std::mutex> lock(mutex);
        Block* block = reinterpret_cast<Block*>(ptr);
        block->next = free_list;
        free_list = block;
    }
    
private:
    void allocate_chunk() {
        auto chunk = std::make_unique<Block[]>(chunk_size);
        for (size_t i = 0; i < chunk_size - 1; ++i) {
            chunk[i].next = &chunk[i + 1];
        }
        chunk[chunk_size - 1].next = free_list;
        free_list = &chunk[0];
        chunks.push_back(std::move(chunk));
    }
    
    void clear() {
        chunks.clear();
        free_list = nullptr;
    }
};

/**
 * @brief Thread pool for parallel processing
 */
class ThreadPool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    std::atomic<bool> stop;
    
public:
    explicit ThreadPool(size_t threads = std::thread::hardware_concurrency());
    ~ThreadPool();
    
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type>;
    
    void wait_for_completion();
    size_t get_thread_count() const { return workers.size(); }
};

/**
 * @brief IP Block represents a hierarchical design block
 */
struct IPBlock {
    std::string name;
    geometry::Rectangle boundary;
    std::vector<std::unique_ptr<IPBlock>> sub_blocks;
    std::vector<size_t> component_ids;
    size_t level;  // Hierarchy level (0 = top level)
    
    IPBlock(const std::string& name, const geometry::Rectangle& boundary, size_t level = 0)
        : name(name), boundary(boundary), level(level) {}
    
    bool contains(const geometry::Rectangle& rect) const {
        return boundary.contains(rect);
    }
    
    bool intersects(const geometry::Rectangle& rect) const {
        return boundary.intersects(rect);
    }
    
    void add_sub_block(std::unique_ptr<IPBlock> block) {
        sub_blocks.push_back(std::move(block));
    }
    
    void add_component(size_t component_id) {
        component_ids.push_back(component_id);
    }
};

/**
 * @brief Z-order curve (Morton code) for spatial hashing
 */
class ZOrderCurve {
public:
    static uint64_t encode(uint32_t x, uint32_t y) {
        return interleave(x) | (interleave(y) << 1);
    }
    
    static std::pair<uint32_t, uint32_t> decode(uint64_t z) {
        return {deinterleave(z), deinterleave(z >> 1)};
    }
    
    static uint64_t encode_point(const geometry::Point& point, 
                                const geometry::Rectangle& bounds) {
        uint32_t x = static_cast<uint32_t>((point.x - bounds.x) / bounds.width * 0xFFFFFFFF);
        uint32_t y = static_cast<uint32_t>((point.y - bounds.y) / bounds.height * 0xFFFFFFFF);
        return encode(x, y);
    }
    
private:
    static uint64_t interleave(uint32_t x) {
        uint64_t result = x;
        result = (result | (result << 16)) & 0x0000FFFF0000FFFF;
        result = (result | (result << 8))  & 0x00FF00FF00FF00FF;
        result = (result | (result << 4))  & 0x0F0F0F0F0F0F0F0F;
        result = (result | (result << 2))  & 0x3333333333333333;
        result = (result | (result << 1))  & 0x5555555555555555;
        return result;
    }
    
    static uint32_t deinterleave(uint64_t x) {
        x = x & 0x5555555555555555;
        x = (x | (x >> 1))  & 0x3333333333333333;
        x = (x | (x >> 2))  & 0x0F0F0F0F0F0F0F0F;
        x = (x | (x >> 4))  & 0x00FF00FF00FF00FF;
        x = (x | (x >> 8))  & 0x0000FFFF0000FFFF;
        x = (x | (x >> 16)) & 0x00000000FFFFFFFF;
        return static_cast<uint32_t>(x);
    }
};

/**
 * @brief R-tree node for efficient rectangle indexing
 */
template<typename T>
struct RTreeNode {
    geometry::Rectangle mbr;  // Minimum bounding rectangle
    std::vector<std::pair<T, geometry::Rectangle>> entries;
    std::vector<std::unique_ptr<RTreeNode<T>>> children;
    bool is_leaf;
    static const size_t MAX_ENTRIES = 16;
    static const size_t MIN_ENTRIES = 4;
    
    RTreeNode(bool leaf = true) : is_leaf(leaf) {}
    
    bool is_full() const {
        return entries.size() >= MAX_ENTRIES;
    }
    
    void update_mbr() {
        if (entries.empty() && children.empty()) {
            mbr = geometry::Rectangle(0, 0, 0, 0);
            return;
        }
        
        if (is_leaf) {
            mbr = entries[0].second;
            for (size_t i = 1; i < entries.size(); ++i) {
                mbr = mbr.union_with(entries[i].second);
            }
        } else {
            mbr = children[0]->mbr;
            for (size_t i = 1; i < children.size(); ++i) {
                mbr = mbr.union_with(children[i]->mbr);
            }
        }
    }
};

/**
 * @brief High-performance R-tree implementation
 */
template<typename T>
class RTree {
private:
    std::unique_ptr<RTreeNode<T>> root;
    size_t object_count;
    
public:
    RTree() : object_count(0) {
        root = std::make_unique<RTreeNode<T>>(true);
    }
    
    void insert(const T& object, const geometry::Rectangle& bbox);
    std::vector<T> query_range(const geometry::Rectangle& range) const;
    std::vector<T> query_point(const geometry::Point& point) const;
    bool remove(const T& object, const geometry::Rectangle& bbox);
    
    size_t size() const { return object_count; }
    bool empty() const { return object_count == 0; }
    void clear();
    
private:
    void insert_recursive(RTreeNode<T>* node, const T& object, const geometry::Rectangle& bbox);
    std::unique_ptr<RTreeNode<T>> split_node(RTreeNode<T>* node);
    void query_recursive(const RTreeNode<T>* node, const geometry::Rectangle& range, std::vector<T>& result) const;
    double calculate_area_increase(const geometry::Rectangle& current, const geometry::Rectangle& new_rect) const;
};

/**
 * @brief Hierarchical spatial index for ultra-large datasets
 */
template<typename T>
class HierarchicalSpatialIndex {
private:
    std::unique_ptr<IPBlock> root_block;
    std::unordered_map<std::string, std::unique_ptr<QuadTree<T>>> block_indices;
    std::unordered_map<std::string, std::unique_ptr<RTree<T>>> rtree_indices;
    std::unordered_map<uint64_t, std::vector<T>> zorder_buckets;
    
    ThreadPool thread_pool;
    MemoryPool<QuadTreeNode<T>> node_pool;
    
    geometry::Rectangle world_bounds;
    size_t max_objects_per_block;
    size_t max_hierarchy_levels;
    
public:
    HierarchicalSpatialIndex(const geometry::Rectangle& world_bounds,
                            size_t max_objects_per_block = 1000000,
                            size_t max_hierarchy_levels = 8)
        : world_bounds(world_bounds),
          max_objects_per_block(max_objects_per_block),
          max_hierarchy_levels(max_hierarchy_levels),
          thread_pool(std::thread::hardware_concurrency()) {
        
        root_block = std::make_unique<IPBlock>("root", world_bounds, 0);
    }
    
    // Batch operations for efficiency
    void bulk_insert(const std::vector<std::pair<T, geometry::Rectangle>>& objects);
    void parallel_bulk_insert(const std::vector<std::pair<T, geometry::Rectangle>>& objects);
    
    std::vector<T> parallel_query_range(const geometry::Rectangle& range) const;
    std::vector<std::pair<T, T>> parallel_find_intersections() const;
    
    // IP block management
    void create_ip_block(const std::string& name, const geometry::Rectangle& boundary, 
                        const std::string& parent_name = "root");
    void optimize_hierarchy();
    
    // GPU acceleration interfaces
    #ifdef ZLAYOUT_USE_CUDA
    void cuda_bulk_insert(const std::vector<std::pair<T, geometry::Rectangle>>& objects);
    std::vector<T> cuda_query_range(const geometry::Rectangle& range) const;
    #endif
    
    #ifdef ZLAYOUT_USE_OPENCL
    void opencl_bulk_insert(const std::vector<std::pair<T, geometry::Rectangle>>& objects);
    std::vector<T> opencl_query_range(const geometry::Rectangle& range) const;
    #endif
    
    // Statistics and optimization
    struct Statistics {
        size_t total_objects;
        size_t total_blocks;
        size_t max_depth;
        size_t avg_objects_per_block;
        double memory_usage_mb;
        double query_performance_ms;
    };
    
    Statistics get_statistics() const;
    void optimize_for_query_pattern(const std::vector<geometry::Rectangle>& query_patterns);
    
private:
    void create_block_index(const std::string& block_name, const geometry::Rectangle& boundary);
    IPBlock* find_block(const std::string& name) const;
    void partition_objects_by_zorder(const std::vector<std::pair<T, geometry::Rectangle>>& objects);
    
    std::vector<std::future<std::vector<T>>> dispatch_parallel_queries(
        const geometry::Rectangle& range) const;
};

/**
 * @brief Ultra-high performance spatial index factory
 */
class SpatialIndexFactory {
public:
    enum class IndexType {
        QUADTREE,
        RTREE,
        HIERARCHICAL,
        ZORDER_HASH,
        HYBRID
    };
    
    template<typename T>
    static std::unique_ptr<HierarchicalSpatialIndex<T>> create_optimized_index(
        const geometry::Rectangle& world_bounds,
        size_t expected_object_count,
        IndexType preferred_type = IndexType::HYBRID) {
        
        // Choose optimal parameters based on expected scale
        size_t max_objects_per_block = 1000000;
        size_t max_hierarchy_levels = 8;
        
        if (expected_object_count > 100000000) {  // > 100M objects
            max_objects_per_block = 10000000;      // 10M per block
            max_hierarchy_levels = 12;
        } else if (expected_object_count > 10000000) {  // > 10M objects
            max_objects_per_block = 1000000;       // 1M per block
            max_hierarchy_levels = 10;
        }
        
        return std::make_unique<HierarchicalSpatialIndex<T>>(
            world_bounds, max_objects_per_block, max_hierarchy_levels);
    }
};

// Template implementations

template<typename T>
void HierarchicalSpatialIndex<T>::bulk_insert(
    const std::vector<std::pair<T, geometry::Rectangle>>& objects) {
    
    // Pre-sort objects by Z-order for better spatial locality
    std::vector<std::pair<T, geometry::Rectangle>> sorted_objects = objects;
    std::sort(sorted_objects.begin(), sorted_objects.end(),
        [this](const auto& a, const auto& b) {
            uint64_t z_a = ZOrderCurve::encode_point(a.second.center(), world_bounds);
            uint64_t z_b = ZOrderCurve::encode_point(b.second.center(), world_bounds);
            return z_a < z_b;
        });
    
    // Create blocks as needed and insert objects
    for (const auto& [object, bbox] : sorted_objects) {
        // Find appropriate block
        std::string block_name = find_optimal_block(bbox);
        
        // Create block index if needed
        if (block_indices.find(block_name) == block_indices.end()) {
            create_block_index(block_name, bbox);
        }
        
        // Insert into block
        block_indices[block_name]->insert(object);
    }
}

template<typename T>
void HierarchicalSpatialIndex<T>::parallel_bulk_insert(
    const std::vector<std::pair<T, geometry::Rectangle>>& objects) {
    
    const size_t num_threads = thread_pool.get_thread_count();
    const size_t chunk_size = (objects.size() + num_threads - 1) / num_threads;
    
    std::vector<std::future<void>> futures;
    
    for (size_t i = 0; i < num_threads; ++i) {
        size_t start = i * chunk_size;
        size_t end = std::min(start + chunk_size, objects.size());
        
        if (start >= objects.size()) break;
        
        auto future = thread_pool.enqueue([this, &objects, start, end]() {
            std::vector<std::pair<T, geometry::Rectangle>> chunk(
                objects.begin() + start, objects.begin() + end);
            this->bulk_insert(chunk);
        });
        
        futures.push_back(std::move(future));
    }
    
    // Wait for all insertions to complete
    for (auto& future : futures) {
        future.wait();
    }
}

template<typename T>
std::vector<T> HierarchicalSpatialIndex<T>::parallel_query_range(
    const geometry::Rectangle& range) const {
    
    auto futures = dispatch_parallel_queries(range);
    
    std::vector<T> result;
    for (auto& future : futures) {
        auto partial_result = future.get();
        result.insert(result.end(), partial_result.begin(), partial_result.end());
    }
    
    // Remove duplicates (objects might be in multiple blocks)
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    
    return result;
}

} // namespace spatial
} // namespace zlayout 