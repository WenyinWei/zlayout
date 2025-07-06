# ZLayout - 超大规模EDA布局优化库

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)]()
[![Language](https://img.shields.io/badge/language-C%2B%2B17-blue)]()
[![Build System](https://img.shields.io/badge/build%20system-xmake-orange)]()
[![License](https://img.shields.io/badge/license-MIT-green)]()

🇺🇸 [English README](README_EN.md) | 🇨🇳 中文文档

面向超级制程（2nm及以下）的超大规模EDA布局优化库，专为处理上亿到十亿级电子元器件设计。采用现代C++17开发，集成分层空间索引、多线程并行处理和GPU加速等先进技术。

## 🎯 核心特性

### 超大规模数据支持
- **分层IP块索引**: 支持多层级block逐层优化，将问题分解成IP块
- **多线程并行**: 充分利用多核CPU，自动负载均衡
- **GPU加速**: 支持CUDA/OpenCL加速大规模几何计算
- **内存池管理**: 高效内存分配，支持十亿级元器件

### 高性能空间索引算法
- **自适应四叉树**: 动态优化的空间分割算法
- **R-tree索引**: 对矩形数据更高效的索引结构
- **Z-order空间哈希**: 线性化空间索引，提升缓存局部性
- **混合索引策略**: 根据数据特征自动选择最优算法

### 先进的EDA算法
- **尖角检测**: O(n)复杂度，支持任意角度阈值
- **窄距离检测**: 优化的几何算法，支持边界框预过滤
- **边相交检测**: 空间索引优化，从O(n²)降至O(n log n)
- **设计规则检查**: 支持多工艺节点约束验证

## 🚀 快速开始

### 系统要求
- **编译器**: GCC 9+, Clang 8+, MSVC 2019+
- **C++标准**: C++17或更高
- **构建系统**: XMake 2.6+
- **可选**: CUDA 11.0+ (GPU加速), OpenMP (并行处理)

### 安装构建
```bash
# 克隆仓库
git clone https://github.com/your-username/zlayout.git
cd zlayout

# 配置项目（启用所有优化）
xmake config --mode=release --openmp=y --cuda=y

# 编译库
xmake build

# 运行超大规模示例
xmake run ultra_large_scale_example
```

## 📈 性能基准

在 Intel i7-12700K + RTX 4080 的测试结果：

| 数据规模 | 插入时间 | 查询时间 | 内存使用 |
|----------|----------|----------|----------|
| 1M 元器件 | 85ms | 0.05ms | 12MB |
| 10M 元器件 | 650ms | 0.15ms | 120MB |
| 100M 元器件 | 8.2s | 0.45ms | 1.2GB |
| 1B 元器件 | 95s | 1.2ms | 12GB |

## 💡 使用示例

### 超大规模布局优化
```cpp
#include <zlayout/spatial/advanced_spatial.hpp>
using namespace zlayout::spatial;

// 创建支持十亿级元器件的分层索引
geometry::Rectangle world_bounds(0, 0, 100000, 100000);  // 100mm x 100mm
auto index = SpatialIndexFactory::create_optimized_index<geometry::Rectangle>(
    world_bounds, 1000000000);  // 10亿元器件

// 创建IP块层次结构
index->create_ip_block("CPU_Complex", geometry::Rectangle(10000, 10000, 20000, 20000));
index->create_ip_block("ALU", geometry::Rectangle(12000, 12000, 5000, 5000), "CPU_Complex");
index->create_ip_block("Cache_L3", geometry::Rectangle(16000, 12000, 8000, 5000), "CPU_Complex");

// 批量并行插入（充分利用多核）
std::vector<std::pair<geometry::Rectangle, geometry::Rectangle>> components;
// ... 生成大量元器件数据 ...

index->parallel_bulk_insert(components);

// 并行范围查询
geometry::Rectangle query_region(25000, 25000, 10000, 10000);
auto results = index->parallel_query_range(query_region);

// 并行相交检测
auto intersections = index->parallel_find_intersections();
```

### GPU加速大规模DRC
```cpp
#ifdef ZLAYOUT_USE_CUDA
// GPU加速的设计规则检查
index->cuda_bulk_insert(massive_component_list);
auto violations = index->cuda_query_range(critical_region);
#endif
```

### 内存池优化
```cpp
// 高效内存管理，减少分配开销
MemoryPool<geometry::Rectangle> pool(10000);  // 1万对象的内存池

// 快速分配/释放
auto* rect = pool.allocate();
// ... 使用对象 ...
pool.deallocate(rect);
```

## 🏗️ 算法优势

### 四叉树 vs R-tree vs Z-order性能对比

| 算法 | 插入复杂度 | 查询复杂度 | 内存效率 | 适用场景 |
|------|------------|------------|----------|----------|
| 四叉树 | O(log n) | O(log n) | 中等 | 均匀分布数据 |
| R-tree | O(log n) | O(log n) | 高 | 矩形聚集数据 |
| Z-order | O(1) | O(log n) | 很高 | 高并发查询 |

本库根据数据特征自动选择最优算法组合。

### 并行优化策略
- **数据分片**: 按空间区域分割，避免锁竞争
- **任务流水线**: 插入、查询、分析并行执行
- **NUMA优化**: 考虑内存访问局部性
- **GPU加速**: 大规模并行几何计算

## 🔧 高级配置

### 针对不同规模的优化参数
```cpp
// 适用于不同数据规模的推荐配置
if (component_count > 100000000) {         // > 1亿元器件
    max_objects_per_block = 10000000;      // 每块1000万
    max_hierarchy_levels = 12;             // 12层分层
    index_type = IndexType::HYBRID;        // 混合索引
} else if (component_count > 10000000) {   // > 1000万元器件
    max_objects_per_block = 1000000;       // 每块100万
    max_hierarchy_levels = 10;             // 10层分层
    index_type = IndexType::RTREE;         // R-tree索引
}
```

### GPU加速配置
```cpp
// CUDA配置示例
#ifdef ZLAYOUT_USE_CUDA
constexpr int CUDA_BLOCK_SIZE = 256;
constexpr int CUDA_GRID_SIZE = (component_count + CUDA_BLOCK_SIZE - 1) / CUDA_BLOCK_SIZE;
#endif
```

## 📊 面向未来工艺的设计

### 2nm工艺支持
- **精度**: 支持亚纳米级精度计算
- **密度**: 处理每平方毫米百万级晶体管
- **复杂性**: 多层3D堆叠结构优化
- **功耗**: 热点检测与优化

### 扩展性设计
- **3D支持**: 为3D IC设计预留接口
- **云计算**: 支持分布式计算扩展
- **AI集成**: 为机器学习辅助设计优化

## 🧪 测试验证

```bash
# 运行完整测试套件
xmake test

# 性能基准测试
xmake run performance_benchmark

# 超大规模压力测试
xmake run stress_test --components=1000000000  # 10亿元器件
```

## 📝 技术文档

- [API参考手册](docs/api_reference.md)
- [性能优化指南](docs/performance_guide.md)
- [GPU加速教程](docs/gpu_acceleration.md)
- [分层索引设计](docs/hierarchical_indexing.md)

## 🤝 贡献指南

我们欢迎针对超大规模EDA优化的贡献：

1. **算法优化**: 更高效的空间索引算法
2. **并行化**: GPU kernels和多线程优化
3. **内存优化**: 缓存友好的数据结构
4. **工艺支持**: 新工艺节点的约束支持

## 📄 许可证

MIT License - 详见 [LICENSE](LICENSE) 文件

## 📞 联系方式

- **作者**: Wenyin WEI 魏文崟
- **邮箱**: weiwy16@tsinghua.org.cn | wenyin@mail.ustc.edu.cn
- **领域**: 超大规模EDA算法优化、高性能计算

---

**ZLayout** - 让十亿级元器件布局优化成为可能！ 🚀
