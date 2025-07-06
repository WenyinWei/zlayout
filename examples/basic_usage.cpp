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
    print_separator("基础几何功能演示");
    
    // 创建点
    Point p1(0.0, 0.0);
    Point p2(3.0, 4.0);
    Point p3(1.0, 1.0);
    
    std::cout << "点 1: " << p1 << std::endl;
    std::cout << "点 2: " << p2 << std::endl;
    std::cout << "点 1 到点 2 的距离: " << std::fixed << std::setprecision(3) 
              << p1.distance_to(p2) << std::endl;
    
    // 创建矩形
    Rectangle rect1(0.0, 0.0, 10.0, 5.0);
    Rectangle rect2(5.0, 2.0, 8.0, 6.0);
    
    std::cout << "\n矩形 1: " << rect1 << std::endl;
    std::cout << "矩形 2: " << rect2 << std::endl;
    std::cout << "矩形相交: " << (rect1.intersects(rect2) ? "是" : "否") << std::endl;
    std::cout << "点 " << p3 << " 在矩形1内: " << (rect1.contains_point(p3) ? "是" : "否") << std::endl;
    
    // 创建三角形
    Polygon triangle({Point(0.0, 0.0), Point(4.0, 0.0), Point(2.0, 3.0)});
    
    std::cout << "\n三角形: " << triangle << std::endl;
    std::cout << "三角形面积: " << std::fixed << std::setprecision(2) 
              << triangle.area() << std::endl;
    std::cout << "三角形是凸多边形: " << (triangle.is_convex() ? "是" : "否") << std::endl;
}

void demonstrate_sharp_angle_detection() {
    print_separator("面试题1: 尖角检测算法");
    
    // 创建包含尖角的多边形
    std::vector<Polygon> test_polygons = {
        // 标准三角形
        Polygon({Point(0.0, 0.0), Point(4.0, 0.0), Point(2.0, 3.0)}),
        
        // 带尖角的多边形
        Polygon({Point(0.0, 0.0), Point(10.0, 0.0), Point(1.0, 1.0), Point(0.0, 10.0)}),
        
        // L形状
        Polygon({Point(0.0, 0.0), Point(3.0, 0.0), Point(3.0, 1.0), 
                Point(1.0, 1.0), Point(1.0, 3.0), Point(0.0, 3.0)}),
        
        // 带非常尖锐角度的多边形
        Polygon({Point(5.0, 5.0), Point(15.0, 5.1), Point(6.0, 8.0)})
    };
    
    std::vector<std::string> polygon_names = {
        "标准三角形", "尖角多边形", "L形多边形", "锐角三角形"
    };
    
    double threshold_degrees = 45.0;
    std::cout << "尖角阈值: " << threshold_degrees << "度" << std::endl;
    
    for (size_t i = 0; i < test_polygons.size(); ++i) {
        std::cout << "\n--- " << polygon_names[i] << " ---" << std::endl;
        
        auto sharp_angles = test_polygons[i].get_sharp_angles(threshold_degrees);
        std::cout << "发现 " << sharp_angles.size() << " 个尖角" << std::endl;
        
        if (!sharp_angles.empty()) {
            std::cout << "尖角位置和角度:" << std::endl;
            for (size_t vertex_idx : sharp_angles) {
                double angle = test_polygons[i].vertex_angle(vertex_idx);
                std::cout << "  顶点 " << vertex_idx << ": " 
                         << std::fixed << std::setprecision(1) << angle << "°" << std::endl;
            }
        }
        
        // 显示所有角度
        auto all_angles = test_polygons[i].all_vertex_angles();
        std::cout << "所有顶点角度: ";
        for (size_t j = 0; j < all_angles.size(); ++j) {
            if (j > 0) std::cout << ", ";
            std::cout << std::fixed << std::setprecision(1) << all_angles[j] << "°";
        }
        std::cout << std::endl;
    }
}

void demonstrate_narrow_distance_detection() {
    print_separator("面试题2: 窄距离检测算法");
    
    // 创建测试多边形对
    std::vector<std::pair<Polygon, Polygon>> test_pairs = {
        // 距离较近的矩形
        {Polygon({Point(0, 0), Point(5, 0), Point(5, 3), Point(0, 3)}),
         Polygon({Point(6, 0), Point(11, 0), Point(11, 3), Point(6, 3)})},
        
        // 重叠的多边形
        {Polygon({Point(0, 0), Point(8, 0), Point(8, 5), Point(0, 5)}),
         Polygon({Point(6, 2), Point(14, 2), Point(14, 7), Point(6, 7)})},
        
        // 距离很近的三角形
        {Polygon({Point(0, 0), Point(3, 0), Point(1.5, 2)}),
         Polygon({Point(3.2, 0), Point(6.2, 0), Point(4.7, 2)})},
    };
    
    std::vector<std::string> pair_names = {
        "相邻矩形", "重叠矩形", "相邻三角形"
    };
    
    double threshold_distance = 2.0;
    std::cout << "窄距离阈值: " << threshold_distance << " 单位" << std::endl;
    
    for (size_t i = 0; i < test_pairs.size(); ++i) {
        std::cout << "\n--- " << pair_names[i] << " ---" << std::endl;
        
        const auto& poly1 = test_pairs[i].first;
        const auto& poly2 = test_pairs[i].second;
        
        // 计算最小距离
        double min_distance = poly1.distance_to(poly2);
        std::cout << "最小距离: " << std::fixed << std::setprecision(3) 
                 << min_distance << " 单位" << std::endl;
        
        // 查找窄距离区域
        auto narrow_regions = poly1.find_narrow_regions(poly2, threshold_distance);
        std::cout << "窄距离区域数量: " << narrow_regions.size() << std::endl;
        
        if (!narrow_regions.empty()) {
            std::cout << "窄距离详情:" << std::endl;
            for (size_t j = 0; j < narrow_regions.size(); ++j) {
                auto [point1, point2, distance] = narrow_regions[j];
                std::cout << "  区域 " << (j+1) << ": " << point1 << " 到 " << point2 
                         << ", 距离 " << std::fixed << std::setprecision(3) << distance << std::endl;
            }
        }
        
        // 检查是否相交
        if (poly1.intersects(poly2)) {
            std::cout << "⚠️  多边形相交!" << std::endl;
            auto intersections = poly1.intersection_points(poly2);
            std::cout << "交点数量: " << intersections.size() << std::endl;
        }
    }
}

void demonstrate_quadtree_intersection_detection() {
    print_separator("面试题3: 四叉树优化的边相交检测");
    
    // 创建多个测试组件
    std::vector<Rectangle> rectangles = {
        Rectangle(10, 10, 5, 5),    // 组件1
        Rectangle(20, 20, 8, 6),    // 组件2
        Rectangle(50, 50, 12, 8),   // 组件3
        Rectangle(75, 25, 6, 10),   // 组件4
        Rectangle(15, 35, 5, 3),    // 组件5 (可能与其他组件接近)
        Rectangle(21, 35, 5, 3),    // 组件6 (与组件5距离很近)
    };
    
    // 设置世界边界
    Rectangle world_bounds(0, 0, 100, 100);
    
    // 创建四叉树 (使用lambda来提供边界框函数)
    auto rect_quadtree = QuadTree<Rectangle>(
        world_bounds,
        [](const Rectangle& rect) -> Rectangle { return rect; }, // 矩形本身就是边界框
        3,  // 容量
        4   // 最大深度
    );
    
    std::cout << "向四叉树插入 " << rectangles.size() << " 个矩形组件..." << std::endl;
    
    // 插入所有矩形
    auto start_time = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < rectangles.size(); ++i) {
        bool success = rect_quadtree.insert(rectangles[i]);
        std::cout << "  组件 " << (i+1) << ": " << (success ? "✓" : "✗") << std::endl;
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto insert_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    std::cout << "插入耗时: " << insert_duration.count() << " 微秒" << std::endl;
    std::cout << "四叉树大小: " << rect_quadtree.size() << " 个对象" << std::endl;
    
    // 范围查询测试
    std::cout << "\n--- 范围查询测试 ---" << std::endl;
    Rectangle query_region(0, 0, 30, 30);
    
    start_time = std::chrono::high_resolution_clock::now();
    auto objects_in_region = rect_quadtree.query_range(query_region);
    end_time = std::chrono::high_resolution_clock::now();
    auto query_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    std::cout << "查询区域 " << query_region << std::endl;
    std::cout << "找到 " << objects_in_region.size() << " 个对象" << std::endl;
    std::cout << "查询耗时: " << query_duration.count() << " 微秒" << std::endl;
    
    // 点查询测试
    std::cout << "\n--- 点查询测试 ---" << std::endl;
    Point query_point(12, 12);
    
    start_time = std::chrono::high_resolution_clock::now();
    auto containing_objects = rect_quadtree.query_point(query_point);
    end_time = std::chrono::high_resolution_clock::now();
    auto point_query_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    std::cout << "查询点 " << query_point << std::endl;
    std::cout << "找到 " << containing_objects.size() << " 个包含该点的对象" << std::endl;
    std::cout << "点查询耗时: " << point_query_duration.count() << " 微秒" << std::endl;
    
    // 潜在相交检测
    std::cout << "\n--- 潜在相交检测 ---" << std::endl;
    start_time = std::chrono::high_resolution_clock::now();
    auto potential_intersections = rect_quadtree.find_potential_intersections();
    end_time = std::chrono::high_resolution_clock::now();
    auto intersection_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    std::cout << "找到 " << potential_intersections.size() << " 对潜在相交的对象" << std::endl;
    std::cout << "相交检测耗时: " << intersection_duration.count() << " 微秒" << std::endl;
    
    // 实际相交验证
    std::cout << "\n验证实际相交:" << std::endl;
    size_t actual_intersections = 0;
    for (const auto& pair : potential_intersections) {
        if (pair.first.intersects(pair.second)) {
            actual_intersections++;
            std::cout << "  实际相交: " << pair.first << " 与 " << pair.second << std::endl;
        }
    }
    std::cout << "实际相交对数: " << actual_intersections << std::endl;
    
    // 四叉树统计信息
    auto stats = rect_quadtree.get_statistics();
    std::cout << "\n--- 四叉树性能统计 ---" << std::endl;
    std::cout << "总节点数: " << stats.total_nodes << std::endl;
    std::cout << "叶子节点数: " << stats.leaf_nodes << std::endl;
    std::cout << "最大深度: " << stats.max_depth_reached << std::endl;
    std::cout << "平均每叶子节点对象数: " << std::fixed << std::setprecision(2) 
              << stats.average_objects_per_leaf << std::endl;
    std::cout << "树效率: " << std::fixed << std::setprecision(3) 
              << stats.tree_efficiency << std::endl;
}

void demonstrate_eda_design_rules() {
    print_separator("EDA设计规则检查示例");
    
    // 模拟EDA组件
    std::vector<Polygon> eda_components = {
        // 微控制器 (矩形)
        Polygon({Point(30, 40), Point(45, 40), Point(45, 55), Point(30, 55)}),
        
        // 电阻 (小矩形)  
        Polygon({Point(60, 50), Point(66, 50), Point(66, 52), Point(60, 52)}),
        
        // 电容 (小矩形)
        Polygon({Point(70, 65), Point(74, 65), Point(74, 68), Point(70, 68)}),
        
        // 连接器 (带尖角)
        Polygon({Point(110, 20), Point(125, 22), Point(112, 28), Point(108, 24)}),
        
        // 走线 (窄多边形)
        Polygon({Point(45, 47), Point(60, 50), Point(60, 50.2), Point(45, 47.2)}),
        
        // 可能重叠的组件
        Polygon({Point(40, 50), Point(55, 52), Point(53, 65), Point(38, 63)}),
        Polygon({Point(50, 55), Point(65, 57), Point(63, 70), Point(48, 68)})
    };
    
    std::vector<std::string> component_names = {
        "微控制器", "电阻", "电容", "连接器", "走线", "组件A", "组件B"
    };
    
    // 不同制程的设计规则
    struct ProcessRules {
        std::string name;
        double min_spacing;
        double sharp_angle_limit;
    };
    
    std::vector<ProcessRules> processes = {
        {"原型级", 0.1, 20.0},
        {"标准级", 0.15, 30.0},
        {"高精度级", 0.05, 45.0}
    };
    
    std::cout << "分析 " << eda_components.size() << " 个EDA组件..." << std::endl;
    
    for (const auto& process : processes) {
        std::cout << "\n--- " << process.name << " 制程检查 ---" << std::endl;
        std::cout << "最小间距: " << process.min_spacing << ", 尖角限制: " 
                 << process.sharp_angle_limit << "°" << std::endl;
        
        int violations = 0;
        
        // 尖角检查
        std::cout << "\n尖角检查:" << std::endl;
        for (size_t i = 0; i < eda_components.size(); ++i) {
            auto sharp_angles = eda_components[i].get_sharp_angles(process.sharp_angle_limit);
            if (!sharp_angles.empty()) {
                violations += sharp_angles.size();
                std::cout << "  " << component_names[i] << ": " << sharp_angles.size() 
                         << " 个尖角违规" << std::endl;
            }
        }
        
        // 间距检查
        std::cout << "\n间距检查:" << std::endl;
        for (size_t i = 0; i < eda_components.size(); ++i) {
            for (size_t j = i + 1; j < eda_components.size(); ++j) {
                double distance = eda_components[i].distance_to(eda_components[j]);
                if (distance < process.min_spacing) {
                    violations++;
                    std::cout << "  " << component_names[i] << " 与 " << component_names[j] 
                             << ": 距离 " << std::fixed << std::setprecision(3) 
                             << distance << " < " << process.min_spacing << std::endl;
                }
            }
        }
        
        // 相交检查
        std::cout << "\n相交检查:" << std::endl;
        for (size_t i = 0; i < eda_components.size(); ++i) {
            for (size_t j = i + 1; j < eda_components.size(); ++j) {
                if (eda_components[i].intersects(eda_components[j])) {
                    violations++;
                    std::cout << "  " << component_names[i] << " 与 " << component_names[j] 
                             << ": 相交违规" << std::endl;
                }
            }
        }
        
        // 总结
        if (violations == 0) {
            std::cout << "✅ 通过 " << process.name << " 制程检查" << std::endl;
        } else {
            std::cout << "❌ " << violations << " 个违规，不符合 " << process.name 
                     << " 制程要求" << std::endl;
        }
    }
}

int main() {
    std::cout << "ZLayout C++ - 高性能EDA布局处理库" << std::endl;
    std::cout << "版本: " << zlayout::get_version() << std::endl;
    
    // 初始化库
    if (!zlayout::initialize()) {
        std::cerr << "库初始化失败!" << std::endl;
        return 1;
    }
    
    try {
        // 演示基础几何功能
        demonstrate_basic_geometry();
        
        // 演示面试题解决方案
        demonstrate_sharp_angle_detection();
        demonstrate_narrow_distance_detection();
        demonstrate_quadtree_intersection_detection();
        
        // 演示EDA应用
        demonstrate_eda_design_rules();
        
        print_separator("演示完成");
        std::cout << "✅ 所有功能演示成功完成!" << std::endl;
        std::cout << "\n核心算法性能:" << std::endl;
        std::cout << "  - 尖角检测: O(n) per polygon" << std::endl;
        std::cout << "  - 窄距离检测: O(n²) 优化为边界框预过滤" << std::endl;
        std::cout << "  - 四叉树相交检测: O(log n) 平均查询复杂度" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    // 清理资源
    zlayout::cleanup();
    
    return 0;
}