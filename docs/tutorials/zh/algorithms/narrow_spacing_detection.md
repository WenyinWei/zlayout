# 窄间距检测算法

*制造约束验证的核心算法*

## 概述

窄间距检测用于识别几何特征间距小于制造公差的区域。该算法是设计规则检查(DRC)的关键组成部分，确保先进半导体工艺的制造可靠性。

## 问题定义

### 制造挑战

- **蚀刻分辨率**：过于接近的特征可能在蚀刻过程中合并
- **光刻限制**：光刻工艺的光学分辨率约束
- **工艺变化**：制造公差要求最小间距
- **电气问题**：寄生耦合和串扰

### 数学基础

对于两个线段或多边形边：
```
最小距离 = min(点到线距离(p1, 线段2), 
              点到线距离(p2, 线段1),
              线段到线段距离(线段1, 线段2))
```

---

## 核心算法

### 方法1：暴力搜索 O(n²)

```cpp
class NarrowSpacingDetector {
public:
    struct SpacingViolation {
        Point point1, point2;
        double distance;
        int polygon1_id, polygon2_id;
    };
    
    std::vector<SpacingViolation> detectNarrowSpacing(
        const std::vector<Polygon>& polygons,
        double min_spacing = 0.1
    ) {
        std::vector<SpacingViolation> violations;
        
        // 检查不同多边形之间的间距
        for (size_t i = 0; i < polygons.size(); ++i) {
            for (size_t j = i + 1; j < polygons.size(); ++j) {
                auto poly_violations = checkPolygonPair(
                    polygons[i], polygons[j], min_spacing, i, j
                );
                violations.insert(violations.end(), 
                                poly_violations.begin(), 
                                poly_violations.end());
            }
        }
        
        return violations;
    }

private:
    std::vector<SpacingViolation> checkPolygonPair(
        const Polygon& poly1, const Polygon& poly2,
        double min_spacing, int id1, int id2
    ) {
        std::vector<SpacingViolation> violations;
        
        auto edges1 = poly1.edges();
        auto edges2 = poly2.edges();
        
        for (const auto& edge1 : edges1) {
            for (const auto& edge2 : edges2) {
                double dist = segmentToSegmentDistance(
                    edge1.first, edge1.second,
                    edge2.first, edge2.second
                );
                
                if (dist < min_spacing) {
                    auto closest_points = findClosestPoints(edge1, edge2);
                    violations.push_back({
                        closest_points.first,
                        closest_points.second,
                        dist, id1, id2
                    });
                }
            }
        }
        
        return violations;
    }
    
    double segmentToSegmentDistance(
        const Point& p1, const Point& p2,
        const Point& p3, const Point& p4
    ) {
        // 使用最近点算法实现
        return calculateMinimumDistance(p1, p2, p3, p4);
    }
};
```

### 方法2：空间索引优化 O(n log n)

```python
class OptimizedSpacingDetector:
    def __init__(self, world_bounds):
        self.spatial_index = zlayout.QuadTree(world_bounds)
        self.polygon_cache = {}
    
    def detect_narrow_spacing(self, polygons, min_spacing=0.1):
        """使用空间索引的优化间距检测"""
        violations = []
        
        # 构建空间索引
        for poly_id, polygon in enumerate(polygons):
            bbox = polygon.bounding_box()
            expanded_bbox = bbox.expand(min_spacing)
            self.spatial_index.insert(expanded_bbox, poly_id)
            self.polygon_cache[poly_id] = polygon
        
        # 只检查附近的多边形对
        checked_pairs = set()
        
        for poly_id, polygon in enumerate(polygons):
            bbox = polygon.bounding_box()
            search_bbox = bbox.expand(min_spacing * 1.5)
            
            nearby_ids = self.spatial_index.query_range(search_bbox)
            
            for nearby_id in nearby_ids:
                if nearby_id != poly_id:
                    pair = tuple(sorted([poly_id, nearby_id]))
                    
                    if pair not in checked_pairs:
                        checked_pairs.add(pair)
                        
                        nearby_polygon = self.polygon_cache[nearby_id]
                        pair_violations = self._check_polygon_pair(
                            polygon, nearby_polygon, 
                            min_spacing, poly_id, nearby_id
                        )
                        violations.extend(pair_violations)
        
        return violations
    
    def _check_polygon_pair(self, poly1, poly2, min_spacing, id1, id2):
        """检查两个多边形之间的间距"""
        violations = []
        
        # 快速包围盒检查
        bbox1 = poly1.bounding_box()
        bbox2 = poly2.bounding_box()
        
        if bbox1.distance_to(bbox2) >= min_spacing:
            return violations  # 距离太远
        
        # 详细的边到边检查
        for edge1 in poly1.edges:
            for edge2 in poly2.edges:
                dist = self._segment_distance(edge1, edge2)
                
                if dist < min_spacing:
                    closest_pts = self._closest_points_on_segments(edge1, edge2)
                    violations.append({
                        'point1': closest_pts[0],
                        'point2': closest_pts[1], 
                        'distance': dist,
                        'polygon1_id': id1,
                        'polygon2_id': id2
                    })
        
        return violations
```

---

## 复杂度分析

### 时间复杂度比较

| 方法 | 预处理 | 查询 | 总体 | 空间复杂度 |
|------|--------|------|------|------------|
| 暴力搜索 | 无 | O(n²m²) | O(n²m²) | O(1) |
| 空间索引 | O(n log n) | O(k log n) | O(n log n + km²) | O(n) |
| R-tree | O(n log n) | O(log n + k) | O(n log n + km²) | O(n) |

其中：
- `n` = 多边形数量
- `m` = 每个多边形的平均边数  
- `k` = 每次查询的平均邻近多边形数

### 性能扩展性

```python
# 不同方法的基准测试
import time
import numpy as np

def benchmark_spacing_detection():
    polygon_counts = [10, 50, 100, 500, 1000]
    
    results = {
        'brute_force': [],
        'spatial_index': [],
        'rtree': []
    }
    
    for n_polygons in polygon_counts:
        # 生成测试数据
        test_polygons = generate_test_layout(n_polygons)
        
        # 暴力方法
        start = time.perf_counter()
        bf_detector = BruteForceDetector()
        bf_results = bf_detector.detect_narrow_spacing(test_polygons, 0.1)
        bf_time = time.perf_counter() - start
        results['brute_force'].append(bf_time)
        
        # 空间索引方法
        start = time.perf_counter()
        si_detector = OptimizedSpacingDetector(world_bounds)
        si_results = si_detector.detect_narrow_spacing(test_polygons, 0.1)
        si_time = time.perf_counter() - start
        results['spatial_index'].append(si_time)
        
        print(f"N={n_polygons}: 暴力={bf_time:.3f}s, 空间索引={si_time:.3f}s, "
              f"加速比={bf_time/si_time:.1f}x")
    
    return results

# 预期输出显示大数据集的显著加速
benchmark_results = benchmark_spacing_detection()
```

---

## 交互式教程

### 教程1：基础间距检查

```python
import zlayout
import matplotlib.pyplot as plt

# 创建包含间距违规的测试布局
components = [
    # 两个非常接近的矩形
    zlayout.Rectangle(0, 0, 10, 5),    # 组件1
    zlayout.Rectangle(10.05, 0, 10, 5), # 组件2 - 只有0.05间距！
    
    # 另一对具有适当间距的组件
    zlayout.Rectangle(0, 10, 8, 4),     # 组件3  
    zlayout.Rectangle(10, 10, 8, 4),    # 组件4 - 2.0间距
]

detector = zlayout.NarrowSpacingDetector()
violations = detector.detect_narrow_spacing(
    components, 
    min_spacing=0.1  # 100nm最小间距
)

print(f"发现 {len(violations)} 个间距违规")
for i, violation in enumerate(violations):
    print(f"违规 {i+1}: {violation['distance']:.3f} < 0.1")
    print(f"  多边形 {violation['polygon1_id']} 和 {violation['polygon2_id']} 之间")
```

### 教程2：工艺节点验证

```python
# 不同工艺节点有不同的间距要求
process_spacing_rules = {
    "28nm": 0.028,  # 28nm最小间距
    "14nm": 0.014,  # 14nm最小间距
    "7nm":  0.007,  # 7nm最小间距
    "3nm":  0.003,  # 3nm最小间距
}

def validate_process_node(layout, process_node):
    min_spacing = process_spacing_rules[process_node]
    
    violations = detector.detect_narrow_spacing(
        layout.components, 
        min_spacing=min_spacing
    )
    
    critical_violations = [
        v for v in violations 
        if v['distance'] < min_spacing * 0.8  # 小于最小值的80%
    ]
    
    return {
        'total_violations': len(violations),
        'critical_violations': len(critical_violations),
        'pass': len(violations) == 0,
        'min_distance_found': min([v['distance'] for v in violations]) if violations else float('inf')
    }

# 在不同工艺节点上测试
test_layout = generate_cpu_layout()

for process in ["28nm", "14nm", "7nm", "3nm"]:
    result = validate_process_node(test_layout, process)
    status = "通过" if result['pass'] else "失败"
    
    print(f"{process} 工艺: {status}")
    print(f"  违规数: {result['total_violations']}")
    print(f"  严重违规: {result['critical_violations']}")
    print(f"  最小距离: {result['min_distance_found']:.4f}")
```

---

## 实际应用

### 1. 先进工艺DRC

```python
class AdvancedDRCChecker:
    def __init__(self, process_rules):
        self.rules = process_rules
        self.detectors = {}
        
    def comprehensive_spacing_check(self, layout):
        """多层间距验证"""
        all_violations = {}
        
        # 不同层有不同的间距规则
        layer_rules = {
            'metal1': self.rules['metal1_spacing'],
            'metal2': self.rules['metal2_spacing'], 
            'via': self.rules['via_spacing'],
            'poly': self.rules['poly_spacing']
        }
        
        for layer_name, min_spacing in layer_rules.items():
            layer_components = layout.get_layer_components(layer_name)
            
            violations = self.detect_narrow_spacing(
                layer_components, min_spacing
            )
            
            # 特殊情况的额外检查
            if layer_name == 'via':
                violations.extend(self._check_via_to_via_spacing(
                    layer_components, min_spacing * 1.2
                ))
            
            all_violations[layer_name] = violations
        
        return all_violations
```

### 2. 寄生提取指导

```python
def analyze_coupling_risk(spacing_violations, frequency_mhz=1000):
    """分析间距违规的电气耦合风险"""
    coupling_analysis = []
    
    for violation in spacing_violations:
        # 计算耦合电容
        distance = violation['distance']
        
        # 简单平行板电容器模型
        # C = ε₀ × εᵣ × A / d
        coupling_capacitance = calculate_coupling_capacitance(
            distance, 
            conductor_width=1.0,  # 假设1μm宽度
            dielectric_constant=3.9  # SiO2
        )
        
        # 估算耦合噪声
        coupling_noise_mv = estimate_coupling_noise(
            coupling_capacitance, frequency_mhz
        )
        
        risk_level = "高" if coupling_noise_mv > 50 else \
                    "中" if coupling_noise_mv > 10 else "低"
        
        coupling_analysis.append({
            'violation': violation,
            'coupling_capacitance_ff': coupling_capacitance * 1e15,
            'coupling_noise_mv': coupling_noise_mv,
            'risk_level': risk_level
        })
    
    return coupling_analysis
```

---

## 高级优化

### 大数据集的GPU加速

```python
# 对于极大布局（数百万多边形）
class GPUSpacingDetector:
    def __init__(self, use_cuda=True):
        self.use_cuda = use_cuda and cuda_available()
        
    def detect_narrow_spacing_gpu(self, polygons, min_spacing):
        """GPU加速的间距检测"""
        if not self.use_cuda:
            return self.fallback_cpu_detection(polygons, min_spacing)
        
        # 将多边形转换为GPU友好格式
        gpu_polygons = self._prepare_gpu_data(polygons)
        
        # 启动CUDA内核进行并行距离计算
        violations = self._cuda_spacing_kernel(gpu_polygons, min_spacing)
        
        return self._format_violations(violations)
    
    def _cuda_spacing_kernel(self, gpu_data, min_spacing):
        """用于并行距离计算的CUDA内核"""
        # 伪CUDA代码
        """
        __global__ void spacing_kernel(
            float* polygon_data, 
            int num_polygons,
            float min_spacing,
            int* violations_out
        ) {
            int idx = blockIdx.x * blockDim.x + threadIdx.x;
            // 并行距离计算...
        }
        """
        pass  # 实现细节
```

### 内存高效的流式处理

```python
class StreamingSpacingDetector:
    def __init__(self, chunk_size=1000):
        self.chunk_size = chunk_size
        
    def detect_large_layout(self, layout_iterator, min_spacing):
        """处理无法装入内存的大布局"""
        violations = []
        polygon_chunks = []
        
        # 分块处理
        for chunk in self._chunk_iterator(layout_iterator, self.chunk_size):
            chunk_violations = self._process_chunk(chunk, min_spacing)
            violations.extend(chunk_violations)
            
            # 保留最近的块用于跨块检查
            polygon_chunks.append(chunk)
            if len(polygon_chunks) > 3:  # 在内存中保留3个块
                polygon_chunks.pop(0)
            
            # 检查块之间的交互
            if len(polygon_chunks) > 1:
                cross_violations = self._check_chunk_boundaries(
                    polygon_chunks[-2], polygon_chunks[-1], min_spacing
                )
                violations.extend(cross_violations)
        
        return violations
```

---

## 性能总结

窄间距检测性能特征：

**算法复杂度：**
- **暴力搜索**：O(n²m²) - 对大布局来说过于昂贵
- **空间索引**：O(n log n + km²) - 生产环境实用
- **GPU加速**：O(n log n) 配合大规模并行计算

**内存使用：**
- **基础**：O(n) 用于多边形存储
- **空间索引**：额外O(n) 用于索引结构
- **流式处理**：O(chunk_size) 适用于任意大小的布局

**实际性能：**
- 1K多边形：~10ms（空间索引）
- 10K多边形：~100ms（空间索引）  
- 100K多边形：~1s（空间索引）
- 1M+多边形：推荐GPU加速

**优化指南：**
1. 超过100个多边形时使用空间索引
2. 考虑包围盒预筛选
3. 超过100K多边形时使用GPU加速
4. 内存受限环境下使用流式处理

该算法是现代EDA流程的核心，在保持合理计算性能的同时，确保先进工艺节点的可靠制造。 