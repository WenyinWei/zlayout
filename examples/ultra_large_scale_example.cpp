/**
 * @file ultra_large_scale_example.cpp
 * @brief Example demonstrating ultra-large scale EDA layout optimization
 * 
 * This example shows how to use the advanced spatial indexing system to handle
 * billions of components in EDA layouts, with support for:
 * - Hierarchical IP blocks
 * - Multi-threaded parallel processing
 * - Efficient batch operations
 * - Memory pool management
 */

#include <zlayout/zlayout.hpp>
#include <zlayout/spatial/advanced_spatial.hpp>
#include <chrono>
#include <iostream>
#include <random>
#include <iomanip>

using namespace zlayout;
using namespace zlayout::spatial;

/**
 * @brief Simulate creating a massive EDA layout with billions of components
 */
class UltraLargeScaleDemo {
private:
    std::mt19937 rng;
    std::uniform_real_distribution<double> coord_dist;
    std::uniform_real_distribution<double> size_dist;
    
public:
    UltraLargeScaleDemo() : rng(std::random_device{}()), 
                           coord_dist(0.0, 100000.0),  // 100mm x 100mm chip
                           size_dist(0.001, 0.1) {}    // 1μm to 100μm components
    
    /**
     * @brief Generate random components for testing
     */
    std::vector<std::pair<geometry::Rectangle, geometry::Rectangle>> 
    generate_components(size_t count) {
        std::vector<std::pair<geometry::Rectangle, geometry::Rectangle>> components;
        components.reserve(count);
        
        std::cout << "Generating " << count << " components..." << std::endl;
        
        for (size_t i = 0; i < count; ++i) {
            double x = coord_dist(rng);
            double y = coord_dist(rng);
            double width = size_dist(rng);
            double height = size_dist(rng);
            
            geometry::Rectangle component(x, y, width, height);
            components.emplace_back(component, component);  // object, bbox pair
            
            if (i % 10000000 == 0 && i > 0) {
                std::cout << "  Generated " << i << " components..." << std::endl;
            }
        }
        
        return components;
    }
    
    /**
     * @brief Demonstrate hierarchical IP block creation
     */
    void demonstrate_ip_blocks() {
        std::cout << "\n=== IP Block Hierarchy Demo ===" << std::endl;
        
        // Create world bounds (100mm x 100mm)
        geometry::Rectangle world_bounds(0, 0, 100000, 100000);
        
        // Create hierarchical spatial index
        HierarchicalSpatialIndex<geometry::Rectangle> index(world_bounds, 1000000, 10);
        
        // Create CPU complex
        index.create_ip_block("CPU", geometry::Rectangle(10000, 10000, 20000, 20000));
        index.create_ip_block("ALU", geometry::Rectangle(12000, 12000, 5000, 5000), "CPU");
        index.create_ip_block("FPU", geometry::Rectangle(18000, 12000, 5000, 5000), "CPU");
        index.create_ip_block("Cache", geometry::Rectangle(12000, 18000, 10000, 8000), "CPU");
        
        // Create GPU complex
        index.create_ip_block("GPU", geometry::Rectangle(40000, 10000, 30000, 30000));
        index.create_ip_block("Shader_Array", geometry::Rectangle(42000, 12000, 26000, 26000), "GPU");
        
        // Create Memory complex
        index.create_ip_block("Memory", geometry::Rectangle(10000, 50000, 60000, 40000));
        index.create_ip_block("DDR_Controller", geometry::Rectangle(12000, 52000, 15000, 8000), "Memory");
        index.create_ip_block("L3_Cache", geometry::Rectangle(30000, 52000, 20000, 15000), "Memory");
        
        std::cout << "Created hierarchical IP block structure" << std::endl;
        
        // Get statistics
        auto stats = index.get_statistics();
        std::cout << "Total blocks: " << stats.total_blocks << std::endl;
        std::cout << "Max depth: " << stats.max_depth << std::endl;
    }
    
    /**
     * @brief Test performance with different scales
     */
    void performance_benchmark() {
        std::cout << "\n=== Performance Benchmark ===" << std::endl;
        
        std::vector<size_t> test_sizes = {
            1000000,      // 1M components
            10000000,     // 10M components
            100000000,    // 100M components
            1000000000    // 1B components (if memory allows)
        };
        
        geometry::Rectangle world_bounds(0, 0, 100000, 100000);
        
        for (size_t size : test_sizes) {
            std::cout << "\nTesting with " << size << " components:" << std::endl;
            
            try {
                // Create optimized index
                auto index = SpatialIndexFactory::create_optimized_index<geometry::Rectangle>(
                    world_bounds, size);
                
                // Generate test data
                auto components = generate_components(size);
                
                // Measure insertion time
                auto start = std::chrono::high_resolution_clock::now();
                index->parallel_bulk_insert(components);
                auto end = std::chrono::high_resolution_clock::now();
                
                auto insertion_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                std::cout << "  Insertion time: " << insertion_time.count() << " ms" << std::endl;
                
                // Test queries
                start = std::chrono::high_resolution_clock::now();
                geometry::Rectangle query_rect(25000, 25000, 10000, 10000);
                auto results = index->parallel_query_range(query_rect);
                end = std::chrono::high_resolution_clock::now();
                
                auto query_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
                std::cout << "  Query time: " << query_time.count() << " μs" << std::endl;
                std::cout << "  Results found: " << results.size() << std::endl;
                
                // Test intersection detection
                start = std::chrono::high_resolution_clock::now();
                auto intersections = index->parallel_find_intersections();
                end = std::chrono::high_resolution_clock::now();
                
                auto intersection_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                std::cout << "  Intersection detection time: " << intersection_time.count() << " ms" << std::endl;
                std::cout << "  Potential intersections: " << intersections.size() << std::endl;
                
                // Get statistics
                auto stats = index->get_statistics();
                std::cout << "  Memory usage: " << std::fixed << std::setprecision(2) 
                         << stats.memory_usage_mb << " MB" << std::endl;
                std::cout << "  Total blocks: " << stats.total_blocks << std::endl;
                std::cout << "  Avg objects per block: " << stats.avg_objects_per_block << std::endl;
                
            } catch (const std::exception& e) {
                std::cout << "  Error: " << e.what() << std::endl;
                std::cout << "  (Likely insufficient memory for this scale)" << std::endl;
                break;
            }
        }
    }
    
    /**
     * @brief Demonstrate design rule checking at scale
     */
    void design_rule_checking() {
        std::cout << "\n=== Large Scale Design Rule Checking ===" << std::endl;
        
        const size_t component_count = 10000000;  // 10M components
        geometry::Rectangle world_bounds(0, 0, 100000, 100000);
        
        auto index = SpatialIndexFactory::create_optimized_index<geometry::Rectangle>(
            world_bounds, component_count);
        
        // Generate components
        auto components = generate_components(component_count);
        
        std::cout << "Inserting " << component_count << " components..." << std::endl;
        auto start = std::chrono::high_resolution_clock::now();
        index->parallel_bulk_insert(components);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto insertion_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Insertion completed in " << insertion_time.count() << " ms" << std::endl;
        
        // DRC: Check minimum spacing violations
        std::cout << "\nPerforming design rule checking..." << std::endl;
        const double min_spacing = 0.01;  // 10μm minimum spacing
        
        start = std::chrono::high_resolution_clock::now();
        auto potential_violations = index->parallel_find_intersections();
        end = std::chrono::high_resolution_clock::now();
        
        auto drc_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "DRC completed in " << drc_time.count() << " ms" << std::endl;
        
        // Filter for actual violations
        size_t real_violations = 0;
        for (const auto& [rect1, rect2] : potential_violations) {
            double distance = rect1.distance_to(rect2);
            if (distance < min_spacing) {
                real_violations++;
            }
        }
        
        std::cout << "Potential violations: " << potential_violations.size() << std::endl;
        std::cout << "Actual violations: " << real_violations << std::endl;
        std::cout << "Violation rate: " << std::fixed << std::setprecision(4) 
                 << (double)real_violations / component_count * 100 << "%" << std::endl;
    }
    
    /**
     * @brief Demonstrate memory pool efficiency
     */
    void memory_pool_demo() {
        std::cout << "\n=== Memory Pool Efficiency Demo ===" << std::endl;
        
        const size_t allocation_count = 1000000;
        
        // Test with standard allocation
        std::cout << "Testing standard allocation..." << std::endl;
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<geometry::Rectangle*> ptrs;
        for (size_t i = 0; i < allocation_count; ++i) {
            ptrs.push_back(new geometry::Rectangle(0, 0, 1, 1));
        }
        for (auto* ptr : ptrs) {
            delete ptr;
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto std_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Standard allocation time: " << std_time.count() << " ms" << std::endl;
        
        // Test with memory pool
        std::cout << "Testing memory pool allocation..." << std::endl;
        start = std::chrono::high_resolution_clock::now();
        
        MemoryPool<geometry::Rectangle> pool;
        std::vector<geometry::Rectangle*> pool_ptrs;
        
        for (size_t i = 0; i < allocation_count; ++i) {
            pool_ptrs.push_back(pool.allocate());
        }
        for (auto* ptr : pool_ptrs) {
            pool.deallocate(ptr);
        }
        
        end = std::chrono::high_resolution_clock::now();
        auto pool_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Memory pool allocation time: " << pool_time.count() << " ms" << std::endl;
        
        double speedup = (double)std_time.count() / pool_time.count();
        std::cout << "Memory pool speedup: " << std::fixed << std::setprecision(2) 
                 << speedup << "x" << std::endl;
    }
    
    /**
     * @brief Demonstrate Z-order curve spatial hashing
     */
    void zorder_demo() {
        std::cout << "\n=== Z-Order Curve Spatial Hashing Demo ===" << std::endl;
        
        geometry::Rectangle bounds(0, 0, 1000, 1000);
        
        // Test points
        std::vector<geometry::Point> points = {
            geometry::Point(100, 100),
            geometry::Point(200, 100),
            geometry::Point(100, 200),
            geometry::Point(200, 200),
            geometry::Point(500, 500),
            geometry::Point(750, 250),
            geometry::Point(250, 750)
        };
        
        std::cout << "Point -> Z-order code mapping:" << std::endl;
        for (const auto& point : points) {
            uint64_t z_code = ZOrderCurve::encode_point(point, bounds);
            auto decoded = ZOrderCurve::decode(z_code);
            
            std::cout << "  (" << point.x << ", " << point.y << ") -> " 
                     << std::hex << z_code << std::dec 
                     << " -> (" << decoded.first << ", " << decoded.second << ")" << std::endl;
        }
        
        // Demonstrate spatial locality
        std::cout << "\nSpatial locality demonstration:" << std::endl;
        std::vector<std::pair<geometry::Point, uint64_t>> point_codes;
        
        for (const auto& point : points) {
            uint64_t z_code = ZOrderCurve::encode_point(point, bounds);
            point_codes.emplace_back(point, z_code);
        }
        
        // Sort by Z-order
        std::sort(point_codes.begin(), point_codes.end(),
                 [](const auto& a, const auto& b) { return a.second < b.second; });
        
        std::cout << "Points sorted by Z-order:" << std::endl;
        for (const auto& [point, z_code] : point_codes) {
            std::cout << "  (" << point.x << ", " << point.y << ") [" 
                     << std::hex << z_code << std::dec << "]" << std::endl;
        }
    }
    
    /**
     * @brief Compare different spatial indexing algorithms
     */
    void algorithm_comparison() {
        std::cout << "\n=== Spatial Index Algorithm Comparison ===" << std::endl;
        
        const size_t component_count = 1000000;  // 1M components for comparison
        geometry::Rectangle world_bounds(0, 0, 10000, 10000);
        
        // Generate test data
        auto components = generate_components(component_count);
        
        // Test QuadTree
        std::cout << "\nTesting QuadTree..." << std::endl;
        auto bbox_func = [](const geometry::Rectangle& rect) { return rect; };
        QuadTree<geometry::Rectangle> quadtree(world_bounds, bbox_func, 100, 8);
        
        auto start = std::chrono::high_resolution_clock::now();
        for (const auto& [component, bbox] : components) {
            quadtree.insert(component);
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto qt_insert_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        // Test R-tree
        std::cout << "Testing R-tree..." << std::endl;
        RTree<geometry::Rectangle> rtree;
        
        start = std::chrono::high_resolution_clock::now();
        for (const auto& [component, bbox] : components) {
            rtree.insert(component, bbox);
        }
        end = std::chrono::high_resolution_clock::now();
        auto rt_insert_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        // Test query performance
        geometry::Rectangle query_rect(2500, 2500, 1000, 1000);
        
        start = std::chrono::high_resolution_clock::now();
        auto qt_results = quadtree.query_range(query_rect);
        end = std::chrono::high_resolution_clock::now();
        auto qt_query_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        start = std::chrono::high_resolution_clock::now();
        auto rt_results = rtree.query_range(query_rect);
        end = std::chrono::high_resolution_clock::now();
        auto rt_query_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        // Print results
        std::cout << "\nResults:" << std::endl;
        std::cout << "  QuadTree insertion: " << qt_insert_time.count() << " ms" << std::endl;
        std::cout << "  R-tree insertion: " << rt_insert_time.count() << " ms" << std::endl;
        std::cout << "  QuadTree query: " << qt_query_time.count() << " μs (" 
                 << qt_results.size() << " results)" << std::endl;
        std::cout << "  R-tree query: " << rt_query_time.count() << " μs (" 
                 << rt_results.size() << " results)" << std::endl;
        
        // Memory usage comparison
        auto qt_stats = quadtree.get_statistics();
        std::cout << "\nMemory and structure:" << std::endl;
        std::cout << "  QuadTree nodes: " << qt_stats.total_nodes << std::endl;
        std::cout << "  QuadTree depth: " << qt_stats.max_depth_reached << std::endl;
        std::cout << "  QuadTree efficiency: " << std::fixed << std::setprecision(2) 
                 << qt_stats.tree_efficiency << std::endl;
    }
};

int main() {
    std::cout << "=== Ultra-Large Scale EDA Layout Optimization Demo ===" << std::endl;
    std::cout << "This demo showcases handling billions of components efficiently" << std::endl;
    
    try {
        UltraLargeScaleDemo demo;
        
        // Run demonstrations
        demo.demonstrate_ip_blocks();
        demo.zorder_demo();
        demo.memory_pool_demo();
        demo.algorithm_comparison();
        
        // Performance benchmarks (may take a while)
        std::cout << "\nStarting performance benchmarks..." << std::endl;
        std::cout << "Note: Large scale tests may take several minutes and require significant memory" << std::endl;
        
        demo.performance_benchmark();
        demo.design_rule_checking();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\nDemo completed successfully!" << std::endl;
    return 0;
} 