/**
 * @file basic_usage.cpp
 * @brief Basic usage example of ZLayout C++ library
 * 
 * Demonstrates the three core interview problems:
 * 1. Sharp angle detection in polygons
 * 2. Narrow distance detection between edges
 * 3. Quadtree-optimized edge intersection detection
 */

#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <zlayout/zlayout.hpp>

using namespace zlayout;
using namespace zlayout::geometry;
using namespace zlayout::spatial;

void print_separator(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << " " << title << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}

void demonstrate_basic_geometry() {
    print_separator("Basic Geometry Features");
    
    // Create points
    Point p1(0.0, 0.0);
    Point p2(3.0, 4.0);
    Point p3(1.0, 1.0);
    
    std::cout << "Point 1: " << p1 << std::endl;
    std::cout << "Point 2: " << p2 << std::endl;
    std::cout << "Distance from Point 1 to Point 2: " << std::fixed << std::setprecision(3) 
              << p1.distance_to(p2) << std::endl;
    
    // Create rectangles
    Rectangle rect1(0.0, 0.0, 10.0, 5.0);
    Rectangle rect2(5.0, 2.0, 8.0, 6.0);
    
    std::cout << "\nRectangle 1: " << rect1 << std::endl;
    std::cout << "Rectangle 2: " << rect2 << std::endl;
    std::cout << "Rectangles intersect: " << (rect1.intersects(rect2) ? "Yes" : "No") << std::endl;
    std::cout << "Point " << p3 << " is inside Rectangle 1: " << (rect1.contains_point(p3) ? "Yes" : "No") << std::endl;
    
    // Create triangle
    Polygon triangle({Point(0.0, 0.0), Point(4.0, 0.0), Point(2.0, 3.0)});
    
    std::cout << "\nTriangle: " << triangle << std::endl;
    std::cout << "Triangle area: " << std::fixed << std::setprecision(2) 
              << triangle.area() << std::endl;
    std::cout << "Triangle is convex: " << (triangle.is_convex() ? "Yes" : "No") << std::endl;
}

void demonstrate_sharp_angle_detection() {
    print_separator("Interview Problem 1: Sharp Angle Detection");
    
    // Create polygons with sharp angles
    std::vector<Polygon> test_polygons = {
        // Standard triangle
        Polygon({Point(0.0, 0.0), Point(4.0, 0.0), Point(2.0, 3.0)}),
        
        // Polygon with sharp angle
        Polygon({Point(0.0, 0.0), Point(10.0, 0.0), Point(1.0, 1.0), Point(0.0, 10.0)}),
        
        // L-shape
        Polygon({Point(0.0, 0.0), Point(3.0, 0.0), Point(3.0, 1.0), 
                Point(1.0, 1.0), Point(1.0, 3.0), Point(0.0, 3.0)}),
        
        // Polygon with very sharp angle
        Polygon({Point(5.0, 5.0), Point(15.0, 5.1), Point(6.0, 8.0)})
    };
    
    std::vector<std::string> polygon_names = {
        "Standard Triangle", "Sharp Angle Polygon", "L-Shape Polygon", "Acute Triangle"
    };
    
    double threshold_degrees = 45.0;
    std::cout << "Sharp angle threshold: " << threshold_degrees << " degrees" << std::endl;
    
    for (size_t i = 0; i < test_polygons.size(); ++i) {
        std::cout << "\n--- " << polygon_names[i] << " ---" << std::endl;
        
        auto sharp_angles = test_polygons[i].get_sharp_angles(threshold_degrees);
        std::cout << "Found " << sharp_angles.size() << " sharp angles" << std::endl;
        
        if (!sharp_angles.empty()) {
            std::cout << "Sharp angle positions and angles:" << std::endl;
            for (size_t vertex_idx : sharp_angles) {
                double angle = test_polygons[i].vertex_angle(vertex_idx);
                std::cout << "  Vertex " << vertex_idx << ": " 
                         << std::fixed << std::setprecision(1) << angle << "°" << std::endl;
            }
        }
        
        // Show all angles
        auto all_angles = test_polygons[i].all_vertex_angles();
        std::cout << "All vertex angles: ";
        for (size_t j = 0; j < all_angles.size(); ++j) {
            if (j > 0) std::cout << ", ";
            std::cout << std::fixed << std::setprecision(1) << all_angles[j] << "°";
        }
        std::cout << std::endl;
    }
}

void demonstrate_narrow_distance_detection() {
    print_separator("Interview Problem 2: Narrow Distance Detection");
    
    // Create test polygon pairs
    std::vector<std::pair<Polygon, Polygon>> test_pairs = {
        // Close rectangles
        {Polygon({Point(0, 0), Point(5, 0), Point(5, 3), Point(0, 3)}),
         Polygon({Point(6, 0), Point(11, 0), Point(11, 3), Point(6, 3)})},
        
        // Overlapping polygons
        {Polygon({Point(0, 0), Point(8, 0), Point(8, 5), Point(0, 5)}),
         Polygon({Point(6, 2), Point(14, 2), Point(14, 7), Point(6, 7)})},
        
        // Close triangles
        {Polygon({Point(0, 0), Point(3, 0), Point(1.5, 2)}),
         Polygon({Point(3.2, 0), Point(6.2, 0), Point(4.7, 2)})},
    };
    
    std::vector<std::string> pair_names = {
        "Adjacent Rectangles", "Overlapping Rectangles", "Adjacent Triangles"
    };
    
    double threshold_distance = 2.0;
    std::cout << "Narrow distance threshold: " << threshold_distance << " units" << std::endl;
    
    for (size_t i = 0; i < test_pairs.size(); ++i) {
        std::cout << "\n--- " << pair_names[i] << " ---" << std::endl;
        
        const auto& poly1 = test_pairs[i].first;
        const auto& poly2 = test_pairs[i].second;
        
        // Calculate minimum distance
        double min_distance = poly1.distance_to(poly2);
        std::cout << "Minimum distance: " << std::fixed << std::setprecision(3) 
                 << min_distance << " units" << std::endl;
        
        // Find narrow distance regions
        auto narrow_regions = poly1.find_narrow_regions(poly2, threshold_distance);
        std::cout << "Narrow distance regions: " << narrow_regions.size() << std::endl;
        
        if (!narrow_regions.empty()) {
            std::cout << "Narrow distance details:" << std::endl;
            for (size_t j = 0; j < narrow_regions.size(); ++j) {
                auto [point1, point2, distance] = narrow_regions[j];
                std::cout << "  Region " << (j+1) << ": " << point1 << " to " << point2 
                         << ", distance " << std::fixed << std::setprecision(3) << distance << std::endl;
            }
        }
        
        // Check for intersection
        if (poly1.intersects(poly2)) {
            std::cout << "WARNING: Polygons intersect!" << std::endl;
            auto intersections = poly1.intersection_points(poly2);
            std::cout << "Intersection points: " << intersections.size() << std::endl;
        }
    }
}

void demonstrate_quadtree_intersection_detection() {
    print_separator("Interview Problem 3: Quadtree-Optimized Edge Intersection");
    
    // Create multiple test components
    std::vector<Rectangle> rectangles = {
        Rectangle(10, 10, 5, 5),    // Component 1
        Rectangle(20, 20, 8, 6),    // Component 2
        Rectangle(50, 50, 12, 8),   // Component 3
        Rectangle(75, 25, 6, 10),   // Component 4
        Rectangle(15, 35, 5, 3),    // Component 5 (may be close to others)
        Rectangle(21, 35, 5, 3),    // Component 6 (close to Component 5)
    };
    
    // Set world boundaries
    Rectangle world_bounds(0, 0, 100, 100);
    
    // Create quadtree (using lambda to provide bounding box function)
    auto rect_quadtree = QuadTree<Rectangle>(
        world_bounds,
        [](const Rectangle& rect) -> Rectangle { return rect; }, // Rectangle is its own bounding box
        3,  // Capacity
        4   // Max depth
    );
    
    std::cout << "Inserting " << rectangles.size() << " rectangle components into quadtree..." << std::endl;
    
    // Insert all rectangles
    auto start_time = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < rectangles.size(); ++i) {
        bool success = rect_quadtree.insert(rectangles[i]);
        std::cout << "  Component " << (i+1) << ": " << (success ? "Success" : "Failed") << std::endl;
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto insert_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    std::cout << "Insertion time: " << insert_duration.count() << " microseconds" << std::endl;
    std::cout << "Quadtree size: " << rect_quadtree.size() << " objects" << std::endl;
    
    // Range query test
    std::cout << "\n--- Range Query Test ---" << std::endl;
    Rectangle query_region(0, 0, 30, 30);
    
    start_time = std::chrono::high_resolution_clock::now();
    auto objects_in_region = rect_quadtree.query_range(query_region);
    end_time = std::chrono::high_resolution_clock::now();
    auto query_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    std::cout << "Query region " << query_region << std::endl;
    std::cout << "Found " << objects_in_region.size() << " objects" << std::endl;
    std::cout << "Query time: " << query_duration.count() << " microseconds" << std::endl;
    
    // Point query test
    std::cout << "\n--- Point Query Test ---" << std::endl;
    Point query_point(12, 12);
    
    start_time = std::chrono::high_resolution_clock::now();
    auto containing_objects = rect_quadtree.query_point(query_point);
    end_time = std::chrono::high_resolution_clock::now();
    auto point_query_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    std::cout << "Query point " << query_point << std::endl;
    std::cout << "Found " << containing_objects.size() << " objects containing the point" << std::endl;
    std::cout << "Point query time: " << point_query_duration.count() << " microseconds" << std::endl;
    
    // Potential intersection detection
    std::cout << "\n--- Potential Intersection Detection ---" << std::endl;
    start_time = std::chrono::high_resolution_clock::now();
    auto potential_intersections = rect_quadtree.find_potential_intersections();
    end_time = std::chrono::high_resolution_clock::now();
    auto intersection_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    std::cout << "Found " << potential_intersections.size() << " pairs of potentially intersecting objects" << std::endl;
    std::cout << "Intersection detection time: " << intersection_duration.count() << " microseconds" << std::endl;
    
    // Actual intersection verification
    std::cout << "\nVerifying actual intersections:" << std::endl;
    size_t actual_intersections = 0;
    for (const auto& pair : potential_intersections) {
        if (pair.first.intersects(pair.second)) {
            actual_intersections++;
            std::cout << "  Actual intersection: " << pair.first << " with " << pair.second << std::endl;
        }
    }
    std::cout << "Actual intersection pairs: " << actual_intersections << std::endl;
    
    // Quadtree statistics
    auto stats = rect_quadtree.get_statistics();
    std::cout << "\n--- Quadtree Performance Statistics ---" << std::endl;
    std::cout << "Total nodes: " << stats.total_nodes << std::endl;
    std::cout << "Leaf nodes: " << stats.leaf_nodes << std::endl;
    std::cout << "Max depth: " << stats.max_depth_reached << std::endl;
    std::cout << "Average objects per leaf: " << std::fixed << std::setprecision(2) 
              << stats.average_objects_per_leaf << std::endl;
    std::cout << "Tree efficiency: " << std::fixed << std::setprecision(3) 
              << stats.tree_efficiency << std::endl;
}

void demonstrate_eda_design_rules() {
    print_separator("EDA Design Rule Check Example");
    
    // Simulate EDA components
    std::vector<Polygon> eda_components = {
        // Microcontroller (rectangle)
        Polygon({Point(30, 40), Point(45, 40), Point(45, 55), Point(30, 55)}),
        
        // Resistor (small rectangle)  
        Polygon({Point(60, 50), Point(66, 50), Point(66, 52), Point(60, 52)}),
        
        // Capacitor (small rectangle)
        Polygon({Point(70, 65), Point(74, 65), Point(74, 68), Point(70, 68)}),
        
        // Connector (with sharp angle)
        Polygon({Point(110, 20), Point(125, 22), Point(112, 28), Point(108, 24)}),
        
        // Trace (narrow polygon)
        Polygon({Point(45, 47), Point(60, 50), Point(60, 50.2), Point(45, 47.2)}),
        
        // Potentially overlapping components
        Polygon({Point(40, 50), Point(55, 52), Point(53, 65), Point(38, 63)}),
        Polygon({Point(50, 55), Point(65, 57), Point(63, 70), Point(48, 68)})
    };
    
    std::vector<std::string> component_names = {
        "Microcontroller", "Resistor", "Capacitor", "Connector", "Trace", "Component A", "Component B"
    };
    
    // Different process design rules
    struct ProcessRules {
        std::string name;
        double min_spacing;
        double sharp_angle_limit;
    };
    
    std::vector<ProcessRules> processes = {
        {"Prototype", 0.1, 20.0},
        {"Standard", 0.15, 30.0},
        {"High Precision", 0.05, 45.0}
    };
    
    std::cout << "Analyzing " << eda_components.size() << " EDA components..." << std::endl;
    
    for (const auto& process : processes) {
        std::cout << "\n--- " << process.name << " Process Check ---" << std::endl;
        std::cout << "Min spacing: " << process.min_spacing << ", Sharp angle limit: " 
                 << process.sharp_angle_limit << "°" << std::endl;
        
        int violations = 0;
        
        // Sharp angle check
        std::cout << "\nSharp angle check:" << std::endl;
        for (size_t i = 0; i < eda_components.size(); ++i) {
            auto sharp_angles = eda_components[i].get_sharp_angles(process.sharp_angle_limit);
            if (!sharp_angles.empty()) {
                violations += sharp_angles.size();
                std::cout << "  " << component_names[i] << ": " << sharp_angles.size() 
                         << " sharp angle violations" << std::endl;
            }
        }
        
        // Spacing check
        std::cout << "\nSpacing check:" << std::endl;
        for (size_t i = 0; i < eda_components.size(); ++i) {
            for (size_t j = i + 1; j < eda_components.size(); ++j) {
                double distance = eda_components[i].distance_to(eda_components[j]);
                if (distance < process.min_spacing) {
                    violations++;
                    std::cout << "  " << component_names[i] << " and " << component_names[j] 
                             << ": distance " << std::fixed << std::setprecision(3) 
                             << distance << " < " << process.min_spacing << std::endl;
                }
            }
        }
        
        // Intersection check
        std::cout << "\nIntersection check:" << std::endl;
        for (size_t i = 0; i < eda_components.size(); ++i) {
            for (size_t j = i + 1; j < eda_components.size(); ++j) {
                if (eda_components[i].intersects(eda_components[j])) {
                    violations++;
                    std::cout << "  " << component_names[i] << " and " << component_names[j] 
                             << ": intersection violation" << std::endl;
                }
            }
        }
        
        // Summary
        if (violations == 0) {
            std::cout << "PASS: " << process.name << " process check" << std::endl;
        } else {
            std::cout << "FAIL: " << violations << " violations, does not meet " << process.name 
                     << " process requirements" << std::endl;
        }
    }
}

int main() {
    std::cout << "ZLayout C++ - High Performance EDA Layout Processing Library" << std::endl;
    std::cout << "Version: " << zlayout::get_version() << std::endl;
    
    // Initialize library
    if (!zlayout::initialize()) {
        std::cerr << "Library initialization failed!" << std::endl;
        return 1;
    }
    
    try {
        // Demonstrate basic geometry features
        demonstrate_basic_geometry();
        
        // Demonstrate interview problem solutions
        demonstrate_sharp_angle_detection();
        demonstrate_narrow_distance_detection();
        demonstrate_quadtree_intersection_detection();
        
        // Demonstrate EDA application
        demonstrate_eda_design_rules();
        
        print_separator("Demo Complete");
        std::cout << "All feature demonstrations completed successfully!" << std::endl;
        std::cout << "\nCore algorithm performance:" << std::endl;
        std::cout << "  - Sharp angle detection: O(n) per polygon" << std::endl;
        std::cout << "  - Narrow distance detection: O(n²) optimized with bounding box pre-filtering" << std::endl;
        std::cout << "  - Quadtree intersection detection: O(log n) average query complexity" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    // Clean up resources
    zlayout::cleanup();
    
    return 0;
}