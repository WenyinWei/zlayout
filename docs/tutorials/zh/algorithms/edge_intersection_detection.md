# 边相交检测算法

*几何完整性检查的核心算法*

## 概述

边相交检测用于识别多边形边缘的交叉位置，这对于检测布局错误、自相交多边形和重叠组件至关重要。该算法是EDA工具几何验证的基础。

## 问题定义

### 相交类型

- **自相交**：同一多边形内的边相互交叉
- **多边形间相交**：不同多边形的边相交
- **退化情况**：重合边、端点接触
- **数值精度**：由于浮点数误差导致的近似相交

### 数学基础

对于两个线段P₁P₂和P₃P₄：
```
存在相交当：
- 线段不平行
- 交点位于两个线段内
- 参数解：P = P₁ + t(P₂ - P₁) = P₃ + s(P₄ - P₃)
```

---

## 核心算法

### 算法1：暴力搜索 O(n²)

```cpp
class EdgeIntersectionDetector {
public:
    struct Intersection {
        Point location;
        int edge1_polygon_id, edge1_index;
        int edge2_polygon_id, edge2_index;
        bool is_proper;  // 真相交（穿越）还是仅接触
    };
    
    std::vector<Intersection> detectIntersections(
        const std::vector<Polygon>& polygons
    ) {
        std::vector<Intersection> intersections;
        
        // 检查所有多边形间的所有边对
        for (size_t poly1_id = 0; poly1_id < polygons.size(); ++poly1_id) {
            for (size_t poly2_id = poly1_id; poly2_id < polygons.size(); ++poly2_id) {
                auto poly_intersections = checkPolygonPair(
                    polygons[poly1_id], polygons[poly2_id], 
                    poly1_id, poly2_id
                );
                intersections.insert(intersections.end(),
                                   poly_intersections.begin(),
                                   poly_intersections.end());
            }
        }
        
        return intersections;
    }

private:
    std::vector<Intersection> checkPolygonPair(
        const Polygon& poly1, const Polygon& poly2,
        int poly1_id, int poly2_id
    ) {
        std::vector<Intersection> intersections;
        auto edges1 = poly1.edges();
        auto edges2 = poly2.edges();
        
        for (size_t i = 0; i < edges1.size(); ++i) {
            for (size_t j = 0; j < edges2.size(); ++j) {
                // 跳过同一多边形中相邻的边
                if (poly1_id == poly2_id && areAdjacent(i, j, edges1.size())) {
                    continue;
                }
                
                auto intersection = segmentIntersection(
                    edges1[i].first, edges1[i].second,
                    edges2[j].first, edges2[j].second
                );
                
                if (intersection.exists) {
                    intersections.push_back({
                        intersection.point,
                        poly1_id, static_cast<int>(i),
                        poly2_id, static_cast<int>(j),
                        intersection.is_proper
                    });
                }
            }
        }
        
        return intersections;
    }
    
    struct IntersectionResult {
        bool exists;
        Point point;
        bool is_proper;
    };
    
    IntersectionResult segmentIntersection(
        const Point& p1, const Point& p2,
        const Point& p3, const Point& p4
    ) {
        // 向量表示
        double dx1 = p2.x - p1.x, dy1 = p2.y - p1.y;
        double dx2 = p4.x - p3.x, dy2 = p4.y - p3.y;
        double dx3 = p1.x - p3.x, dy3 = p1.y - p3.y;
        
        // 叉积计算行列式
        double denominator = dx1 * dy2 - dy1 * dx2;
        
        // 检查直线是否平行
        if (std::abs(denominator) < 1e-10) {
            return {false, Point(), false};
        }
        
        // 计算参数
        double t = (dx2 * dy3 - dy2 * dx3) / denominator;
        double s = (dx1 * dy3 - dy1 * dx3) / denominator;
        
        // 检查交点是否在两个线段内
        bool within_seg1 = (t >= 0.0 && t <= 1.0);
        bool within_seg2 = (s >= 0.0 && s <= 1.0);
        
        if (within_seg1 && within_seg2) {
            Point intersection_point(
                p1.x + t * dx1,
                p1.y + t * dy1
            );
            
            // 检查是否为真相交（不只是接触）
            bool is_proper = (t > 1e-10 && t < 1.0 - 1e-10) &&
                            (s > 1e-10 && s < 1.0 - 1e-10);
            
            return {true, intersection_point, is_proper};
        }
        
        return {false, Point(), false};
    }
};
```

### 算法2：扫描线 O((n+k) log n)

```cpp
class SweepLineIntersectionDetector {
private:
    struct Event {
        double x;
        enum Type { START, END, INTERSECTION } type;
        int edge_id;
        Point point;
        
        bool operator<(const Event& other) const {
            if (std::abs(x - other.x) > 1e-10) return x < other.x;
            return type < other.type;  // START在END之前处理
        }
    };
    
    struct Edge {
        Point start, end;
        int polygon_id, edge_index;
        
        double y_at_x(double x) const {
            if (std::abs(end.x - start.x) < 1e-10) return start.y;
            double t = (x - start.x) / (end.x - start.x);
            return start.y + t * (end.y - start.y);
        }
    };

public:
    std::vector<Intersection> detectIntersectionsSweepLine(
        const std::vector<Polygon>& polygons
    ) {
        std::vector<Event> events;
        std::vector<Edge> edges;
        
        // 为所有边创建事件
        int edge_id = 0;
        for (size_t poly_id = 0; poly_id < polygons.size(); ++poly_id) {
            auto poly_edges = polygons[poly_id].edges();
            
            for (size_t edge_idx = 0; edge_idx < poly_edges.size(); ++edge_idx) {
                const auto& edge = poly_edges[edge_idx];
                
                // 确保从左到右的顺序
                Point left = edge.first, right = edge.second;
                if (left.x > right.x) std::swap(left, right);
                
                edges.push_back({left, right, 
                               static_cast<int>(poly_id), 
                               static_cast<int>(edge_idx)});
                
                events.push_back({left.x, Event::START, edge_id, left});
                events.push_back({right.x, Event::END, edge_id, right});
                
                edge_id++;
            }
        }
        
        // 按x坐标排序事件
        std::sort(events.begin(), events.end());
        
        // 扫描线状态结构（按y坐标排序的集合）
        std::set<int, decltype([this](int a, int b) {
            return this->compareEdgesByY(a, b);
        })> active_edges(
            [this](int a, int b) { return compareEdgesByY(a, b); }
        );
        
        std::vector<Intersection> intersections;
        
        // 处理事件
        for (const auto& event : events) {
            if (event.type == Event::START) {
                // 将边添加到活跃集合
                auto [iter, inserted] = active_edges.insert(event.edge_id);
                
                // 检查与邻居的相交
                if (iter != active_edges.begin()) {
                    auto prev = std::prev(iter);
                    checkAndAddIntersection(*prev, event.edge_id, 
                                          intersections, edges);
                }
                
                auto next = std::next(iter);
                if (next != active_edges.end()) {
                    checkAndAddIntersection(event.edge_id, *next,
                                          intersections, edges);
                }
                
            } else if (event.type == Event::END) {
                // 从活跃集合中移除边
                auto iter = active_edges.find(event.edge_id);
                if (iter != active_edges.end()) {
                    // 检查邻居现在是否相交
                    auto prev = (iter != active_edges.begin()) ? 
                               std::prev(iter) : active_edges.end();
                    auto next = std::next(iter);
                    
                    active_edges.erase(iter);
                    
                    if (prev != active_edges.end() && next != active_edges.end()) {
                        checkAndAddIntersection(*prev, *next,
                                              intersections, edges);
                    }
                }
            }
        }
        
        return intersections;
    }

private:
    std::vector<Edge> edges_storage;
    double current_sweep_x;
    
    bool compareEdgesByY(int edge_a, int edge_b) {
        double y_a = edges_storage[edge_a].y_at_x(current_sweep_x);
        double y_b = edges_storage[edge_b].y_at_x(current_sweep_x);
        
        if (std::abs(y_a - y_b) > 1e-10) return y_a < y_b;
        return edge_a < edge_b;  // 打破平局
    }
    
    void checkAndAddIntersection(int edge1_id, int edge2_id,
                                std::vector<Intersection>& intersections,
                                const std::vector<Edge>& edges) {
        const auto& edge1 = edges[edge1_id];
        const auto& edge2 = edges[edge2_id];
        
        auto intersection = segmentIntersection(
            edge1.start, edge1.end,
            edge2.start, edge2.end
        );
        
        if (intersection.exists) {
            intersections.push_back({
                intersection.point,
                edge1.polygon_id, edge1.edge_index,
                edge2.polygon_id, edge2.edge_index,
                intersection.is_proper
            });
        }
    }
};
```

---

## Python实现

```python
class EdgeIntersectionDetector:
    """针对EDA布局优化的边相交检测器"""
    
    @staticmethod
    def detect_intersections(polygons, include_touching=False):
        """
        检测多边形集合中的所有边相交
        
        参数:
            polygons: 多边形对象列表
            include_touching: 是否包含端点接触作为相交
            
        返回:
            相交信息字典列表
        """
        intersections = []
        
        # 使用空间索引提高效率
        spatial_index = SpatialIndex()
        edge_data = []
        
        # 索引所有边
        for poly_id, polygon in enumerate(polygons):
            edges = polygon.edges
            for edge_id, (start, end) in enumerate(edges):
                bbox = BoundingBox.from_points([start, end])
                edge_info = {
                    'polygon_id': poly_id,
                    'edge_id': edge_id,
                    'start': start,
                    'end': end,
                    'bbox': bbox
                }
                spatial_index.insert(bbox, len(edge_data))
                edge_data.append(edge_info)
        
        # 检查相交
        checked_pairs = set()
        
        for i, edge1 in enumerate(edge_data):
            # 查询附近的边
            candidates = spatial_index.query(edge1['bbox'])
            
            for j in candidates:
                if i >= j:  # 避免重复检查
                    continue
                    
                edge2 = edge_data[j]
                
                # 跳过同一多边形中的相邻边
                if (edge1['polygon_id'] == edge2['polygon_id'] and 
                    abs(edge1['edge_id'] - edge2['edge_id']) <= 1):
                    continue
                
                pair = (i, j)
                if pair in checked_pairs:
                    continue
                checked_pairs.add(pair)
                
                # 检查相交
                intersection = EdgeIntersectionDetector._segment_intersection(
                    edge1['start'], edge1['end'],
                    edge2['start'], edge2['end']
                )
                
                if intersection['exists']:
                    if include_touching or intersection['is_proper']:
                        intersections.append({
                            'point': intersection['point'],
                            'polygon1_id': edge1['polygon_id'],
                            'edge1_id': edge1['edge_id'],
                            'polygon2_id': edge2['polygon_id'], 
                            'edge2_id': edge2['edge_id'],
                            'is_proper': intersection['is_proper']
                        })
        
        return intersections
    
    @staticmethod
    def _segment_intersection(p1, p2, p3, p4):
        """计算两个线段之间的相交"""
        # 方向向量
        d1 = (p2[0] - p1[0], p2[1] - p1[1])
        d2 = (p4[0] - p3[0], p4[1] - p3[1])
        d3 = (p1[0] - p3[0], p1[1] - p3[1])
        
        # 叉积计算行列式
        denominator = d1[0] * d2[1] - d1[1] * d2[0]
        
        # 检查平行线
        if abs(denominator) < 1e-10:
            return {'exists': False, 'point': None, 'is_proper': False}
        
        # 计算参数
        t = (d2[0] * d3[1] - d2[1] * d3[0]) / denominator
        s = (d1[0] * d3[1] - d1[1] * d3[0]) / denominator
        
        # 检查交点是否在两个线段内
        if 0.0 <= t <= 1.0 and 0.0 <= s <= 1.0:
            intersection_point = (
                p1[0] + t * d1[0],
                p1[1] + t * d1[1]
            )
            
            # 判断是否为真相交
            epsilon = 1e-10
            is_proper = (epsilon < t < 1.0 - epsilon and 
                        epsilon < s < 1.0 - epsilon)
            
            return {
                'exists': True,
                'point': intersection_point,
                'is_proper': is_proper
            }
        
        return {'exists': False, 'point': None, 'is_proper': False}
```

---

## 复杂度分析

### 时间复杂度比较

| 算法 | 预处理 | 检测 | 总计 | 空间复杂度 |
|------|--------|------|------|------------|
| 暴力搜索 | 无 | O(n²m²) | O(n²m²) | O(1) |
| 扫描线 | O(nm log(nm)) | O((nm+k)log(nm)) | O((nm+k)log(nm)) | O(nm) |
| 空间索引 | O(nm log(nm)) | O(nm log(nm) + k) | O(nm log(nm) + k) | O(nm) |

其中：
- `n` = 多边形数量
- `m` = 每个多边形的平均边数
- `k` = 发现的相交数量

### 性能基准测试

```python
def benchmark_intersection_algorithms():
    """比较不同相交算法的性能"""
    
    polygon_counts = [10, 50, 100, 500]
    edge_counts = [4, 8, 16, 32]  # 每个多边形的平均边数
    
    results = {}
    
    for n_polys in polygon_counts:
        for avg_edges in edge_counts:
            test_polygons = generate_complex_layout(n_polys, avg_edges)
            
            # 暴力方法
            start = time.perf_counter()
            bf_detector = BruteForceDetector()
            bf_results = bf_detector.detect_intersections(test_polygons)
            bf_time = time.perf_counter() - start
            
            # 扫描线方法
            start = time.perf_counter()
            sl_detector = SweepLineDetector()
            sl_results = sl_detector.detect_intersections(test_polygons)
            sl_time = time.perf_counter() - start
            
            # 空间索引方法
            start = time.perf_counter()
            si_detector = EdgeIntersectionDetector()
            si_results = si_detector.detect_intersections(test_polygons)
            si_time = time.perf_counter() - start
            
            key = f"{n_polys}p_{avg_edges}e"
            results[key] = {
                'brute_force': bf_time,
                'sweep_line': sl_time,
                'spatial_index': si_time,
                'intersections_found': len(si_results)
            }
            
            print(f"{key}: 暴力={bf_time:.3f}s, 扫描线={sl_time:.3f}s, 空间索引={si_time:.3f}s")
    
    return results
```

---

## 交互式教程

### 教程1：基础相交检测

```python
import zlayout
import matplotlib.pyplot as plt

# 创建包含已知相交的测试用例
polygons = [
    # 自相交的蝴蝶结多边形
    zlayout.Polygon([
        (0, 0), (4, 4), (4, 0), (0, 4)  # 创建X形相交
    ]),
    
    # 两个重叠的矩形
    zlayout.Rectangle(5, 1, 3, 2),  # 矩形1
    zlayout.Rectangle(6, 0, 3, 3),  # 矩形2 - 与矩形1重叠
    
    # 与矩形相交的三角形
    zlayout.Polygon([
        (10, 0), (13, 0), (11.5, 3)   # 三角形
    ]),
    zlayout.Rectangle(11, 1, 2, 1),   # 与三角形相交的矩形
]

detector = zlayout.EdgeIntersectionDetector()
intersections = detector.detect_intersections(polygons)

print(f"发现 {len(intersections)} 个边相交:")
for i, intersection in enumerate(intersections):
    print(f"相交 {i+1}:")
    print(f"  位置: ({intersection['point'][0]:.2f}, {intersection['point'][1]:.2f})")
    print(f"  之间: 多边形 {intersection['polygon1_id']} 边 {intersection['edge1_id']}")
    print(f"        多边形 {intersection['polygon2_id']} 边 {intersection['edge2_id']}")
    print(f"  类型: {'真相交' if intersection['is_proper'] else '端点接触'}")
```

### 教程2：自相交验证

```python
def validate_polygon_integrity(polygon):
    """检查多边形是否有自相交"""
    
    # 创建单个多边形列表用于相交检测
    result = detector.detect_intersections([polygon])
    
    # 只筛选自相交
    self_intersections = [
        inter for inter in result 
        if inter['polygon1_id'] == inter['polygon2_id']
    ]
    
    if self_intersections:
        print(f"多边形有 {len(self_intersections)} 个自相交:")
        for inter in self_intersections:
            print(f"  边 {inter['edge1_id']} 与边 {inter['edge2_id']} 相交")
            print(f"  在点: ({inter['point'][0]:.3f}, {inter['point'][1]:.3f})")
        return False
    else:
        print("多边形有效（无自相交）")
        return True

# 测试各种多边形类型
test_cases = [
    # 有效的简单多边形
    zlayout.Polygon([(0, 0), (4, 0), (4, 3), (0, 3)]),
    
    # 自相交的蝴蝶结
    zlayout.Polygon([(0, 0), (2, 2), (2, 0), (0, 2)]),
    
    # 复杂的有效多边形
    zlayout.Polygon([(0, 0), (3, 0), (4, 1), (3, 3), (1, 4), (0, 2)]),
    
    # 8字形状（自相交）
    zlayout.Polygon([(0, 1), (1, 2), (2, 1), (3, 2), (4, 1), (3, 0), (2, 1), (1, 0)])
]

for i, polygon in enumerate(test_cases):
    print(f"\n=== 测试用例 {i+1} ===")
    is_valid = validate_polygon_integrity(polygon)
```

---

## 实际应用

### 1. 布局验证

```python
class LayoutVerifier:
    def __init__(self):
        self.detector = EdgeIntersectionDetector()
    
    def comprehensive_intersection_check(self, layout):
        """执行全面的相交分析"""
        
        report = {
            'self_intersections': [],
            'component_overlaps': [],
            'critical_intersections': [],
            'total_intersections': 0
        }
        
        # 获取所有相交
        all_intersections = self.detector.detect_intersections(
            layout.components, include_touching=True
        )
        
        report['total_intersections'] = len(all_intersections)
        
        for intersection in all_intersections:
            # 分类相交类型
            if intersection['polygon1_id'] == intersection['polygon2_id']:
                # 自相交
                report['self_intersections'].append(intersection)
            else:
                # 组件重叠
                component1 = layout.components[intersection['polygon1_id']]
                component2 = layout.components[intersection['polygon2_id']]
                
                overlap_info = {
                    'intersection': intersection,
                    'component1_name': component1.name,
                    'component2_name': component2.name,
                    'component1_layer': component1.layer,
                    'component2_layer': component2.layer
                }
                
                # 检查是否为关键相交
                if (component1.layer == component2.layer and 
                    intersection['is_proper']):
                    report['critical_intersections'].append(overlap_info)
                else:
                    report['component_overlaps'].append(overlap_info)
        
        return report
    
    def generate_fix_suggestions(self, intersection_report):
        """为修复相交生成建议"""
        suggestions = []
        
        # 自相交修复
        for self_inter in intersection_report['self_intersections']:
            suggestions.append({
                'type': 'self_intersection',
                'severity': '严重',
                'message': f"多边形有自相交边。考虑简化几何形状。",
                'location': self_inter['point']
            })
        
        # 关键重叠修复
        for critical in intersection_report['critical_intersections']:
            suggestions.append({
                'type': 'component_overlap',
                'severity': '高', 
                'message': f"组件 '{critical['component1_name']}' 和 "
                          f"'{critical['component2_name']}' 在同一层重叠",
                'suggestion': "调整组件位置或修改布线"
            })
        
        return suggestions
```

### 2. 制造约束检查

```python
def check_manufacturing_constraints(layout, process_rules):
    """检查违反制造规则的相交"""
    
    violations = []
    intersections = detector.detect_intersections(layout.components)
    
    for intersection in intersections:
        comp1 = layout.components[intersection['polygon1_id']]
        comp2 = layout.components[intersection['polygon2_id']]
        
        # 检查同层相交
        if comp1.layer == comp2.layer:
            layer_rules = process_rules.get(comp1.layer, {})
            
            if intersection['is_proper']:
                # 同层的真相交总是违规
                violations.append({
                    'type': 'same_layer_overlap',
                    'severity': 'error',
                    'layer': comp1.layer,
                    'location': intersection['point'],
                    'components': [comp1.name, comp2.name]
                })
            
        # 检查层间违规
        else:
            # 某些层组合不允许相交
            forbidden_combinations = process_rules.get('forbidden_intersections', [])
            layer_pair = tuple(sorted([comp1.layer, comp2.layer]))
            
            if layer_pair in forbidden_combinations:
                violations.append({
                    'type': 'forbidden_layer_intersection',
                    'severity': 'warning',
                    'layers': layer_pair,
                    'location': intersection['point']
                })
    
    return violations
```

---

## 高级优化

### Bentley-Ottmann算法

```python
class BentleyOttmannDetector:
    """最优的O((n+k) log n)相交检测"""
    
    def detect_intersections_optimal(self, polygons):
        """Bentley-Ottmann扫描线算法"""
        
        events = []
        edges = []
        
        # 创建初始事件
        for poly_id, polygon in enumerate(polygons):
            poly_edges = polygon.edges
            for edge_id, (start, end) in enumerate(poly_edges):
                # 确保从左到右的顺序
                if start[0] > end[0] or (start[0] == end[0] and start[1] > end[1]):
                    start, end = end, start
                
                edge_data = {
                    'id': len(edges),
                    'polygon_id': poly_id,
                    'edge_id': edge_id,
                    'start': start,
                    'end': end
                }
                edges.append(edge_data)
                
                events.append(Event(start[0], 'start', len(edges) - 1))
                events.append(Event(end[0], 'end', len(edges) - 1))
        
        # 排序事件
        events.sort(key=lambda e: (e.x, e.type == 'end'))
        
        # 扫描线状态
        active_edges = BalancedBST()  # Y序的活跃边
        intersections = []
        
        for event in events:
            if event.type == 'start':
                edge = edges[event.edge_id]
                
                # 插入边并查找邻居
                node = active_edges.insert(edge)
                
                # 检查与邻居的相交
                if node.predecessor:
                    self._check_intersection(
                        edges[node.predecessor.edge_id], edge, intersections
                    )
                if node.successor:
                    self._check_intersection(
                        edge, edges[node.successor.edge_id], intersections
                    )
                    
            elif event.type == 'end':
                edge = edges[event.edge_id]
                node = active_edges.find(edge)
                
                # 检查前驱和后继现在是否相交
                if node.predecessor and node.successor:
                    self._check_intersection(
                        edges[node.predecessor.edge_id],
                        edges[node.successor.edge_id], 
                        intersections
                    )
                
                active_edges.delete(node)
        
        return intersections
```

---

## 性能总结

边相交检测特征：

**算法选择：**
- **暴力搜索**：边总数<100时使用
- **空间索引**：最佳通用算法
- **扫描线**：稀疏相交时最优
- **Bentley-Ottmann**：最佳理论复杂度

**性能扩展：**
- 小数据集（<1K边）：所有算法都可接受
- 中等数据集（1K-10K边）：推荐空间索引
- 大数据集（>10K边）：扫描线或Bentley-Ottmann

**内存要求：**
- 暴力搜索：O(1)工作内存
- 空间索引：O(n)用于索引结构
- 扫描线：O(n)用于事件队列和活跃集合

**实际考虑：**
1. 空间索引具有最佳平均情况性能
2. 扫描线在相交密度低时表现优异
3. 数值精度对稳健结果至关重要
4. 验证用例可能提前终止

该算法是几何验证的基础，能够可靠检测布局错误并确保EDA工作流中的几何完整性。 