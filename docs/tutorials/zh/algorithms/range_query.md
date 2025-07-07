# 范围查询操作

*多边形重叠检测的高效空间搜索*

## 概述

范围查询操作查找与给定矩形区域重叠的所有多边形。这是EDA工具中碰撞检测、区域选择和邻近分析的基础操作。范围查询的效率直接影响交互性能。

## 问题定义

### 查询类型

- **点查询**：查找包含特定点的多边形
- **矩形查询**：查找与矩形区域重叠的多边形  
- **圆形查询**：查找在圆形半径内的多边形
- **多边形查询**：查找与任意多边形重叠的多边形

### 数学基础

对于多边形P和查询矩形Q：
```
重叠(P, Q) = true 当：
  包围盒(P) ∩ 包围盒(Q) ≠ ∅ 且
  详细相交(P, Q) = true
```

两阶段过滤：
1. **粗筛选**：包围盒相交测试（快速）
2. **细筛选**：详细几何相交（精确）

---

## 数据结构比较

### 1. 四叉树实现

```cpp
class QuadTree {
private:
    struct Node {
        Rectangle bounds;
        std::vector<int> polygon_ids;
        std::array<std::unique_ptr<Node>, 4> children;
        bool is_leaf = true;
        
        static constexpr int MAX_POLYGONS = 10;
        static constexpr int MAX_DEPTH = 12;
    };
    
    std::unique_ptr<Node> root;
    std::vector<Polygon> polygons;

public:
    QuadTree(const Rectangle& world_bounds) {
        root = std::make_unique<Node>();
        root->bounds = world_bounds;
    }
    
    void insert(const Polygon& polygon) {
        int poly_id = polygons.size();
        polygons.push_back(polygon);
        insert_recursive(root.get(), poly_id, 0);
    }
    
    std::vector<int> range_query(const Rectangle& query_rect) const {
        std::vector<int> results;
        range_query_recursive(root.get(), query_rect, results);
        
        // 细筛选：详细相交测试
        std::vector<int> filtered_results;
        for (int poly_id : results) {
            if (polygons[poly_id].intersects(query_rect)) {
                filtered_results.push_back(poly_id);
            }
        }
        
        return filtered_results;
    }

private:
    void insert_recursive(Node* node, int poly_id, int depth) {
        if (node->is_leaf) {
            node->polygon_ids.push_back(poly_id);
            
            // 必要时分裂
            if (node->polygon_ids.size() > Node::MAX_POLYGONS && 
                depth < Node::MAX_DEPTH) {
                split_node(node, depth);
            }
        } else {
            // 插入到适当的子节点
            Rectangle poly_bbox = polygons[poly_id].bounding_box();
            
            for (int i = 0; i < 4; ++i) {
                if (node->children[i]->bounds.intersects(poly_bbox)) {
                    insert_recursive(node->children[i].get(), poly_id, depth + 1);
                }
            }
        }
    }
    
    void range_query_recursive(const Node* node, const Rectangle& query_rect,
                              std::vector<int>& results) const {
        if (!node->bounds.intersects(query_rect)) {
            return;  // 与节点边界无相交
        }
        
        if (node->is_leaf) {
            // 添加此叶节点中的所有多边形（粗筛选）
            results.insert(results.end(), 
                          node->polygon_ids.begin(), 
                          node->polygon_ids.end());
        } else {
            // 递归查询子节点
            for (const auto& child : node->children) {
                if (child) {
                    range_query_recursive(child.get(), query_rect, results);
                }
            }
        }
    }
    
    void split_node(Node* node, int depth) {
        node->is_leaf = false;
        
        // 创建四个象限
        double mid_x = (node->bounds.min_x + node->bounds.max_x) / 2.0;
        double mid_y = (node->bounds.min_y + node->bounds.max_y) / 2.0;
        
        node->children[0] = std::make_unique<Node>();  // 西北
        node->children[0]->bounds = Rectangle(
            node->bounds.min_x, mid_y, mid_x, node->bounds.max_y);
            
        node->children[1] = std::make_unique<Node>();  // 东北
        node->children[1]->bounds = Rectangle(
            mid_x, mid_y, node->bounds.max_x, node->bounds.max_y);
            
        node->children[2] = std::make_unique<Node>();  // 西南
        node->children[2]->bounds = Rectangle(
            node->bounds.min_x, node->bounds.min_y, mid_x, mid_y);
            
        node->children[3] = std::make_unique<Node>();  // 东南
        node->children[3]->bounds = Rectangle(
            mid_x, node->bounds.min_y, node->bounds.max_x, mid_y);
        
        // 重新分配多边形
        for (int poly_id : node->polygon_ids) {
            for (int i = 0; i < 4; ++i) {
                Rectangle poly_bbox = polygons[poly_id].bounding_box();
                if (node->children[i]->bounds.intersects(poly_bbox)) {
                    insert_recursive(node->children[i].get(), poly_id, depth + 1);
                }
            }
        }
        
        node->polygon_ids.clear();
    }
};
```

### 2. R树实现

```cpp
class RTree {
private:
    struct Node {
        Rectangle bounding_box;
        std::vector<std::pair<Rectangle, int>> entries;  // (包围盒, 多边形ID)
        std::vector<std::unique_ptr<Node>> children;
        bool is_leaf = true;
        
        static constexpr int MIN_ENTRIES = 2;
        static constexpr int MAX_ENTRIES = 8;
    };
    
    std::unique_ptr<Node> root;
    std::vector<Polygon> polygons;

public:
    RTree() {
        root = std::make_unique<Node>();
    }
    
    void insert(const Polygon& polygon) {
        int poly_id = polygons.size();
        polygons.push_back(polygon);
        
        Rectangle poly_bbox = polygon.bounding_box();
        insert_recursive(root.get(), poly_bbox, poly_id);
    }
    
    std::vector<int> range_query(const Rectangle& query_rect) const {
        std::vector<int> results;
        range_query_recursive(root.get(), query_rect, results);
        return results;
    }

private:
    void insert_recursive(Node* node, const Rectangle& bbox, int poly_id) {
        if (node->is_leaf) {
            node->entries.emplace_back(bbox, poly_id);
            node->bounding_box = node->bounding_box.union_with(bbox);
            
            if (node->entries.size() > Node::MAX_ENTRIES) {
                split_node(node);
            }
        } else {
            // 选择面积扩张最小的子树
            int best_child = choose_subtree(node, bbox);
            insert_recursive(node->children[best_child].get(), bbox, poly_id);
            
            // 更新包围盒
            node->bounding_box = node->bounding_box.union_with(bbox);
        }
    }
    
    void range_query_recursive(const Node* node, const Rectangle& query_rect,
                              std::vector<int>& results) const {
        if (!node->bounding_box.intersects(query_rect)) {
            return;
        }
        
        if (node->is_leaf) {
            for (const auto& [bbox, poly_id] : node->entries) {
                if (bbox.intersects(query_rect)) {
                    // 详细相交测试
                    if (polygons[poly_id].intersects(query_rect)) {
                        results.push_back(poly_id);
                    }
                }
            }
        } else {
            for (const auto& child : node->children) {
                range_query_recursive(child.get(), query_rect, results);
            }
        }
    }
    
    int choose_subtree(Node* node, const Rectangle& bbox) {
        int best_child = 0;
        double min_enlargement = std::numeric_limits<double>::max();
        
        for (size_t i = 0; i < node->children.size(); ++i) {
            Rectangle union_bbox = node->children[i]->bounding_box.union_with(bbox);
            double enlargement = union_bbox.area() - 
                               node->children[i]->bounding_box.area();
            
            if (enlargement < min_enlargement) {
                min_enlargement = enlargement;
                best_child = static_cast<int>(i);
            }
        }
        
        return best_child;
    }
};
```

### 3. Z序空间哈希

```cpp
class ZOrderIndex {
private:
    struct ZOrderEntry {
        uint64_t z_value;
        int polygon_id;
        Rectangle bbox;
        
        bool operator<(const ZOrderEntry& other) const {
            return z_value < other.z_value;
        }
    };
    
    std::vector<ZOrderEntry> entries;
    std::vector<Polygon> polygons;
    Rectangle world_bounds;
    bool is_sorted = false;

public:
    ZOrderIndex(const Rectangle& bounds) : world_bounds(bounds) {}
    
    void insert(const Polygon& polygon) {
        int poly_id = polygons.size();
        polygons.push_back(polygon);
        
        Rectangle bbox = polygon.bounding_box();
        uint64_t z_value = compute_z_order(bbox.center());
        
        entries.push_back({z_value, poly_id, bbox});
        is_sorted = false;
    }
    
    std::vector<int> range_query(const Rectangle& query_rect) const {
        if (!is_sorted) {
            std::sort(const_cast<std::vector<ZOrderEntry>&>(entries).begin(),
                     const_cast<std::vector<ZOrderEntry>&>(entries).end());
            const_cast<ZOrderIndex*>(this)->is_sorted = true;
        }
        
        std::vector<int> results;
        
        // 计算查询矩形的Z序范围
        auto [z_min, z_max] = compute_z_range(query_rect);
        
        // 二分搜索范围
        auto lower = std::lower_bound(entries.begin(), entries.end(),
                                    ZOrderEntry{z_min, -1, Rectangle()});
        auto upper = std::upper_bound(entries.begin(), entries.end(),
                                    ZOrderEntry{z_max, -1, Rectangle()});
        
        // 检查Z序范围内的候选项
        for (auto it = lower; it != upper; ++it) {
            if (it->bbox.intersects(query_rect)) {
                if (polygons[it->polygon_id].intersects(query_rect)) {
                    results.push_back(it->polygon_id);
                }
            }
        }
        
        return results;
    }

private:
    uint64_t compute_z_order(const Point& point) const {
        // 将坐标标准化到[0, 1]
        double norm_x = (point.x - world_bounds.min_x) / world_bounds.width();
        double norm_y = (point.y - world_bounds.min_y) / world_bounds.height();
        
        // 转换为整数坐标
        uint32_t int_x = static_cast<uint32_t>(norm_x * UINT32_MAX);
        uint32_t int_y = static_cast<uint32_t>(norm_y * UINT32_MAX);
        
        // 交错位以创建Z序值
        return interleave_bits(int_x, int_y);
    }
    
    uint64_t interleave_bits(uint32_t x, uint32_t y) const {
        uint64_t result = 0;
        for (int i = 0; i < 32; ++i) {
            result |= ((x & (1ULL << i)) << i) | ((y & (1ULL << i)) << (i + 1));
        }
        return result;
    }
    
    std::pair<uint64_t, uint64_t> compute_z_range(const Rectangle& rect) const {
        // 简化：使用最小和最大角点
        uint64_t z_min = compute_z_order(Point(rect.min_x, rect.min_y));
        uint64_t z_max = compute_z_order(Point(rect.max_x, rect.max_y));
        
        return {std::min(z_min, z_max), std::max(z_min, z_max)};
    }
};
```

---

## 性能分析

### 复杂度比较

| 数据结构 | 构建时间 | 查询时间 | 空间复杂度 | 最佳用例 |
|----------|----------|----------|------------|----------|
| 四叉树 | O(n log n) | O(log n + k) | O(n) | 均匀分布 |
| R树 | O(n log n) | O(log n + k) | O(n) | 聚集矩形 |
| Z序 | O(n log n) | O(log n + k) | O(n) | 高维数据 |
| 混合索引 | O(n log n) | O(log n + k) | O(n) | 混合负载 |

其中：
- `n` = 多边形数量
- `k` = 返回的结果数量

### Python基准测试框架

```python
class RangeQueryBenchmark:
    """范围查询算法的综合基准测试"""
    
    def __init__(self, world_bounds):
        self.world_bounds = world_bounds
        self.algorithms = {
            'quadtree': QuadTreeIndex(world_bounds),
            'rtree': RTreeIndex(world_bounds),
            'zorder': ZOrderIndex(world_bounds),
            'hybrid': HybridIndex(world_bounds)
        }
    
    def benchmark_range_queries(self, polygons, query_rects, num_trials=10):
        """基准测试不同的范围查询算法"""
        
        results = {}
        
        for algo_name, index in self.algorithms.items():
            print(f"\n基准测试 {algo_name}...")
            
            # 构建索引
            build_start = time.perf_counter()
            for polygon in polygons:
                index.insert(polygon)
            build_time = time.perf_counter() - build_start
            
            # 查询性能
            query_times = []
            result_counts = []
            
            for trial in range(num_trials):
                for query_rect in query_rects:
                    query_start = time.perf_counter()
                    query_results = index.range_query(query_rect)
                    query_time = time.perf_counter() - query_start
                    
                    query_times.append(query_time)
                    result_counts.append(len(query_results))
            
            results[algo_name] = {
                'build_time': build_time,
                'avg_query_time': np.mean(query_times),
                'avg_results': np.mean(result_counts),
                'memory_usage': index.memory_usage()
            }
        
        return results
    
    def generate_test_scenarios(self, n_polygons):
        """为综合评估生成不同的测试场景"""
        
        scenarios = {}
        
        # 均匀随机分布
        scenarios['uniform'] = self.generate_uniform_polygons(n_polygons)
        
        # 聚集分布
        scenarios['clustered'] = self.generate_clustered_polygons(n_polygons, n_clusters=10)
        
        # 真实EDA布局模式
        scenarios['eda_pattern'] = self.generate_eda_layout(n_polygons)
        
        return scenarios
    
    def generate_query_patterns(self):
        """生成不同的查询模式用于测试"""
        
        queries = {}
        
        # 小面积查询（世界面积的1%）
        small_area = self.world_bounds.area() * 0.01
        side_length = math.sqrt(small_area)
        queries['small'] = [
            Rectangle(x, y, side_length, side_length)
            for x, y in self.random_points(100)
        ]
        
        # 中等面积查询（世界面积的5%）
        medium_area = self.world_bounds.area() * 0.05
        side_length = math.sqrt(medium_area)
        queries['medium'] = [
            Rectangle(x, y, side_length, side_length)
            for x, y in self.random_points(50)
        ]
        
        # 大面积查询（世界面积的20%）
        large_area = self.world_bounds.area() * 0.20
        side_length = math.sqrt(large_area)
        queries['large'] = [
            Rectangle(x, y, side_length, side_length)
            for x, y in self.random_points(20)
        ]
        
        return queries

# 运行综合基准测试
def run_comprehensive_benchmark():
    world_bounds = Rectangle(0, 0, 10000, 10000)
    benchmark = RangeQueryBenchmark(world_bounds)
    
    polygon_counts = [1000, 5000, 10000, 50000]
    
    for n_polys in polygon_counts:
        print(f"\n=== 测试 {n_polys} 个多边形 ===")
        
        scenarios = benchmark.generate_test_scenarios(n_polys)
        query_patterns = benchmark.generate_query_patterns()
        
        for scenario_name, polygons in scenarios.items():
            print(f"\n场景: {scenario_name}")
            
            for query_name, queries in query_patterns.items():
                print(f"查询模式: {query_name}")
                
                results = benchmark.benchmark_range_queries(polygons, queries)
                
                # 打印结果表
                print(f"{'算法':<12} {'构建(ms)':<12} {'查询(μs)':<12} {'结果数':<10} {'内存(MB)':<12}")
                print("-" * 60)
                
                for algo, metrics in results.items():
                    print(f"{algo:<12} {metrics['build_time']*1000:<12.1f} "
                          f"{metrics['avg_query_time']*1e6:<12.1f} "
                          f"{metrics['avg_results']:<10.1f} "
                          f"{metrics['memory_usage']/1e6:<12.1f}")
```

---

## 交互式教程

### 教程1：基础范围查询

```python
import zlayout
import matplotlib.pyplot as plt
import numpy as np

# 创建包含各种多边形类型的测试布局
world_bounds = zlayout.Rectangle(0, 0, 1000, 1000)
index = zlayout.QuadTree(world_bounds)

# 添加不同类型的组件
components = [
    # 常规组件
    zlayout.Rectangle(100, 100, 50, 30),   # 小矩形
    zlayout.Rectangle(200, 150, 80, 60),   # 中矩形
    zlayout.Rectangle(400, 300, 120, 90),  # 大矩形
    
    # 三角形组件
    zlayout.Polygon([(500, 100), (600, 100), (550, 180)]),
    zlayout.Polygon([(700, 200), (800, 220), (750, 300)]),
    
    # 复杂多边形
    zlayout.Polygon([(50, 500), (150, 480), (180, 550), (120, 600), (30, 580)]),
]

# 构建空间索引
for component in components:
    index.insert(component)

# 定义查询区域
query_regions = [
    zlayout.Rectangle(80, 80, 100, 100),    # 与小矩形重叠
    zlayout.Rectangle(450, 250, 200, 200),  # 与大矩形重叠
    zlayout.Rectangle(0, 450, 300, 200),    # 与复杂多边形重叠
]

# 执行范围查询
for i, query_rect in enumerate(query_regions):
    print(f"\n=== 查询 {i+1}: {query_rect} ===")
    
    # 查找重叠组件
    overlapping_ids = index.range_query(query_rect)
    
    print(f"发现 {len(overlapping_ids)} 个重叠组件:")
    for comp_id in overlapping_ids:
        component = components[comp_id]
        overlap_area = component.intersection_area(query_rect)
        print(f"  组件 {comp_id}: 重叠面积 = {overlap_area:.2f}")
```

### 教程2：性能比较

```python
def compare_index_performance():
    """比较不同空间索引方法的性能"""
    
    # 生成测试数据
    world_bounds = zlayout.Rectangle(0, 0, 5000, 5000)
    n_polygons = 10000
    
    # 创建不同的索引类型
    indexes = {
        '四叉树': zlayout.QuadTree(world_bounds),
        'R树': zlayout.RTree(world_bounds),
        'Z序': zlayout.ZOrderIndex(world_bounds)
    }
    
    # 生成测试多边形
    polygons = generate_random_layout(n_polygons, world_bounds)
    
    # 构建索引并测量时间
    build_times = {}
    for name, index in indexes.items():
        start_time = time.perf_counter()
        
        for polygon in polygons:
            index.insert(polygon)
            
        build_times[name] = time.perf_counter() - start_time
        print(f"{name} 构建时间: {build_times[name]:.3f}s")
    
    # 生成查询矩形
    query_rects = [
        zlayout.Rectangle(x, y, 100, 100)
        for x, y in zip(
            np.random.uniform(0, 4900, 1000),
            np.random.uniform(0, 4900, 1000)
        )
    ]
    
    # 测量查询性能
    query_times = {}
    for name, index in indexes.items():
        times = []
        
        for query_rect in query_rects:
            start_time = time.perf_counter()
            results = index.range_query(query_rect)
            end_time = time.perf_counter()
            
            times.append(end_time - start_time)
        
        query_times[name] = {
            'avg': np.mean(times),
            'median': np.median(times),
            'p95': np.percentile(times, 95)
        }
    
    # 打印比较表
    print(f"\n{'索引类型':<12} {'构建(s)':<10} {'平均查询(μs)':<15} {'P95查询(μs)':<15}")
    print("-" * 55)
    
    for name in indexes.keys():
        print(f"{name:<12} {build_times[name]:<10.3f} "
              f"{query_times[name]['avg']*1e6:<15.1f} "
              f"{query_times[name]['p95']*1e6:<15.1f}")

# 运行性能比较
compare_index_performance()
```

---

## 实际应用

### 1. 交互式选择工具

```python
class InteractiveSelector:
    """EDA GUI的交互式多边形选择器"""
    
    def __init__(self, spatial_index):
        self.index = spatial_index
        self.selection_cache = {}
    
    def select_in_region(self, selection_rect, selection_mode='replace'):
        """在矩形区域内选择多边形"""
        
        # 对重复查询使用缓存
        cache_key = (selection_rect.to_tuple(), selection_mode)
        if cache_key in self.selection_cache:
            return self.selection_cache[cache_key]
        
        # 执行范围查询
        candidates = self.index.range_query(selection_rect)
        
        selected_polygons = []
        for poly_id in candidates:
            polygon = self.index.get_polygon(poly_id)
            
            # 不同的选择条件
            if selection_mode == 'intersect':
                if polygon.intersects(selection_rect):
                    selected_polygons.append(poly_id)
            elif selection_mode == 'contain':
                if selection_rect.contains(polygon):
                    selected_polygons.append(poly_id)
            elif selection_mode == 'center':
                if selection_rect.contains(polygon.center()):
                    selected_polygons.append(poly_id)
        
        # 缓存结果
        self.selection_cache[cache_key] = selected_polygons
        return selected_polygons
    
    def incremental_select(self, current_selection, new_rect, operation='add'):
        """基于操作增量更新选择"""
        
        new_selection = self.select_in_region(new_rect)
        
        if operation == 'add':
            return list(set(current_selection) | set(new_selection))
        elif operation == 'subtract':
            return list(set(current_selection) - set(new_selection))
        elif operation == 'intersect':
            return list(set(current_selection) & set(new_selection))
        else:  # replace
            return new_selection
```

### 2. 碰撞检测系统

```python
class CollisionDetector:
    """组件放置的实时碰撞检测"""
    
    def __init__(self, spatial_index):
        self.index = spatial_index
        self.collision_threshold = 0.1  # 最小间隙
    
    def check_placement(self, new_component, position):
        """检查组件是否可以在位置上放置而不发生碰撞"""
        
        # 将组件变换到新位置
        positioned_component = new_component.translate(position)
        component_bbox = positioned_component.bounding_box()
        
        # 按碰撞阈值扩展包围盒
        expanded_bbox = component_bbox.expand(self.collision_threshold)
        
        # 查找附近组件
        nearby_components = self.index.range_query(expanded_bbox)
        
        collisions = []
        for comp_id in nearby_components:
            existing_component = self.index.get_polygon(comp_id)
            
            # 检查实际碰撞
            distance = positioned_component.distance_to(existing_component)
            if distance < self.collision_threshold:
                collisions.append({
                    'component_id': comp_id,
                    'distance': distance,
                    'overlap_area': positioned_component.intersection_area(existing_component)
                })
        
        return {
            'can_place': len(collisions) == 0,
            'collisions': collisions,
            'collision_count': len(collisions)
        }
    
    def find_valid_positions(self, component, search_region, grid_spacing=10):
        """在搜索区域内找到组件的所有有效位置"""
        
        valid_positions = []
        
        x_positions = np.arange(search_region.min_x, search_region.max_x, grid_spacing)
        y_positions = np.arange(search_region.min_y, search_region.max_y, grid_spacing)
        
        for x in x_positions:
            for y in y_positions:
                position = Point(x, y)
                result = self.check_placement(component, position)
                
                if result['can_place']:
                    valid_positions.append(position)
        
        return valid_positions
```

---

## 优化技术

### 1. 查询结果缓存

```python
class CachedRangeQuery:
    """具有智能缓存的范围查询"""
    
    def __init__(self, spatial_index, cache_size=1000):
        self.index = spatial_index
        self.cache = {}
        self.cache_hits = 0
        self.cache_misses = 0
        self.max_cache_size = cache_size
    
    def range_query(self, query_rect, cache_enabled=True):
        """具有可选缓存的范围查询"""
        
        if not cache_enabled:
            return self.index.range_query(query_rect)
        
        # 检查缓存
        cache_key = self._compute_cache_key(query_rect)
        
        if cache_key in self.cache:
            self.cache_hits += 1
            return self.cache[cache_key]
        
        # 缓存未命中 - 执行实际查询
        self.cache_misses += 1
        results = self.index.range_query(query_rect)
        
        # 添加到缓存（必要时进行驱逐）
        if len(self.cache) >= self.max_cache_size:
            # 简单的LRU驱逐
            oldest_key = next(iter(self.cache))
            del self.cache[oldest_key]
        
        self.cache[cache_key] = results
        return results
    
    def _compute_cache_key(self, query_rect):
        """计算查询矩形的缓存键"""
        # 量化坐标以减少类似查询的缓存未命中
        quantum = 1.0
        return (
            round(query_rect.min_x / quantum) * quantum,
            round(query_rect.min_y / quantum) * quantum,
            round(query_rect.width() / quantum) * quantum,
            round(query_rect.height() / quantum) * quantum
        )
    
    def get_cache_stats(self):
        """获取缓存性能统计"""
        total_queries = self.cache_hits + self.cache_misses
        hit_rate = self.cache_hits / total_queries if total_queries > 0 else 0
        
        return {
            'cache_hits': self.cache_hits,
            'cache_misses': self.cache_misses,
            'hit_rate': hit_rate,
            'cache_size': len(self.cache)
        }
```

### 2. 多线程范围查询

```python
class ParallelRangeQuery:
    """大查询集的并行范围查询处理"""
    
    def __init__(self, spatial_index, num_threads=None):
        self.index = spatial_index
        self.num_threads = num_threads or multiprocessing.cpu_count()
    
    def batch_range_query(self, query_rects):
        """并行处理多个范围查询"""
        
        # 将查询分成批次
        batch_size = max(1, len(query_rects) // self.num_threads)
        query_batches = [
            query_rects[i:i + batch_size]
            for i in range(0, len(query_rects), batch_size)
        ]
        
        # 并行处理批次
        with multiprocessing.Pool(self.num_threads) as pool:
            batch_results = pool.map(self._process_query_batch, query_batches)
        
        # 合并结果
        all_results = []
        for batch_result in batch_results:
            all_results.extend(batch_result)
        
        return all_results
    
    def _process_query_batch(self, query_batch):
        """在单线程中处理一批查询"""
        batch_results = []
        
        for query_rect in query_batch:
            results = self.index.range_query(query_rect)
            batch_results.append(results)
        
        return batch_results
```

---

## 总结

范围查询操作是EDA工具中空间数据处理的基础：

**算法选择指南：**
- **四叉树**：适用于均匀分布数据和交互式应用
- **R树**：适用于聚集矩形数据和批量操作  
- **Z序**：适用于高维数据和并行处理
- **混合索引**：结合多种方法优势，适用于多样化负载

**性能特征：**
- 所有算法都达到O(log n + k)查询复杂度
- 平衡结构的构建时间为O(n log n)
- 内存使用量与对象数量成线性关系O(n)

**优化策略：**
1. 根据数据分布选择适当的数据结构
2. 为重复模式实现查询结果缓存
3. 对批量查询使用并行处理
4. 对复杂几何体应用两阶段过滤

范围查询为交互式EDA工具和自动化布局验证系统提供了高效的空间分析支持。 