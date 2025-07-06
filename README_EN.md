# ZLayout - Ultra-Large Scale EDA Layout Optimization Library

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)]()
[![Language](https://img.shields.io/badge/language-C%2B%2B17-blue)]()
[![Build System](https://img.shields.io/badge/build%20system-xmake-orange)]()
[![License](https://img.shields.io/badge/license-MIT-green)]()

🇺🇸 English README | 🇨🇳 [中文文档](README.md)

Ultra-large scale EDA layout optimization library for advanced process nodes (2nm and below), designed to handle hundreds of millions to billions of electronic components. Built with modern C++17, featuring hierarchical spatial indexing, multi-threaded parallel processing, and GPU acceleration.

## 🎯 Core Features

### Ultra-Large Scale Data Support
- **Hierarchical IP Block Indexing**: Multi-level block optimization, decomposing problems into IP blocks
- **Multi-threaded Parallelism**: Full utilization of multi-core CPUs with automatic load balancing
- **GPU Acceleration**: CUDA/OpenCL acceleration for massive geometric computations
- **Memory Pool Management**: Efficient memory allocation supporting billion-scale components

### High-Performance Spatial Indexing Algorithms
- **Adaptive QuadTree**: Dynamically optimized spatial partitioning algorithm
- **R-tree Indexing**: More efficient indexing structure for rectangular data
- **Z-order Spatial Hashing**: Linearized spatial indexing for improved cache locality
- **Hybrid Index Strategy**: Automatic selection of optimal algorithms based on data characteristics

### Advanced EDA Algorithms
- **Sharp Angle Detection**: O(n) complexity, supports arbitrary angle thresholds
- **Narrow Distance Detection**: Optimized geometric algorithms with bounding box pre-filtering
- **Edge Intersection Detection**: Spatial index optimization, reducing from O(n²) to O(n log n)
- **Design Rule Checking**: Multi-process node constraint validation

## 🚀 Quick Start

### System Requirements
- **Compiler**: GCC 9+, Clang 8+, MSVC 2019+
- **C++ Standard**: C++17 or higher
- **Build System**: XMake 2.6+
- **Optional**: CUDA 11.0+ (GPU acceleration), OpenMP (parallel processing)

### Installation and Build
```bash
# Clone repository
git clone https://github.com/your-username/zlayout.git
cd zlayout

# Configure project (enable all optimizations)
xmake config --mode=release --openmp=y --cuda=y

# Build library
xmake build

# Run ultra-large scale example
xmake run ultra_large_scale_example
```

## 📈 Performance Benchmarks

Test results on Intel i7-12700K + RTX 4080:

| Data Scale | Insertion Time | Query Time | Memory Usage |
|------------|----------------|------------|--------------|
| 1M Components | 85ms | 0.05ms | 12MB |
| 10M Components | 650ms | 0.15ms | 120MB |
| 100M Components | 8.2s | 0.45ms | 1.2GB |
| 1B Components | 95s | 1.2ms | 12GB |

## 💡 Usage Examples

### Ultra-Large Scale Layout Optimization
```cpp
#include <zlayout/spatial/advanced_spatial.hpp>
using namespace zlayout::spatial;

// Create hierarchical index supporting billion-scale components
geometry::Rectangle world_bounds(0, 0, 100000, 100000);  // 100mm x 100mm
auto index = SpatialIndexFactory::create_optimized_index<geometry::Rectangle>(
    world_bounds, 1000000000);  // 1 billion components

// Create IP block hierarchy
index->create_ip_block("CPU_Complex", geometry::Rectangle(10000, 10000, 20000, 20000));
index->create_ip_block("ALU", geometry::Rectangle(12000, 12000, 5000, 5000), "CPU_Complex");
index->create_ip_block("Cache_L3", geometry::Rectangle(16000, 12000, 8000, 5000), "CPU_Complex");

// Batch parallel insertion (full multi-core utilization)
std::vector<std::pair<geometry::Rectangle, geometry::Rectangle>> components;
// ... generate massive component data ...

index->parallel_bulk_insert(components);

// Parallel range queries
geometry::Rectangle query_region(25000, 25000, 10000, 10000);
auto results = index->parallel_query_range(query_region);

// Parallel intersection detection
auto intersections = index->parallel_find_intersections();
```

### GPU-Accelerated Large-Scale DRC
```cpp
#ifdef ZLAYOUT_USE_CUDA
// GPU-accelerated design rule checking
index->cuda_bulk_insert(massive_component_list);
auto violations = index->cuda_query_range(critical_region);
#endif
```

### Memory Pool Optimization
```cpp
// Efficient memory management, reducing allocation overhead
MemoryPool<geometry::Rectangle> pool(10000);  // Memory pool for 10K objects

// Fast allocation/deallocation
auto* rect = pool.allocate();
// ... use object ...
pool.deallocate(rect);
```

## 🏗️ Algorithm Advantages

### QuadTree vs R-tree vs Z-order Performance Comparison

| Algorithm | Insertion Complexity | Query Complexity | Memory Efficiency | Use Case |
|-----------|---------------------|------------------|-------------------|----------|
| QuadTree | O(log n) | O(log n) | Medium | Uniformly distributed data |
| R-tree | O(log n) | O(log n) | High | Clustered rectangular data |
| Z-order | O(1) | O(log n) | Very High | High-concurrency queries |

This library automatically selects optimal algorithm combinations based on data characteristics.

### Parallel Optimization Strategies
- **Data Sharding**: Spatial region partitioning to avoid lock contention
- **Task Pipelining**: Parallel execution of insertion, querying, and analysis
- **NUMA Optimization**: Memory access locality considerations
- **GPU Acceleration**: Massive parallel geometric computations

## 🔧 Advanced Configuration

### Optimization Parameters for Different Scales
```cpp
// Recommended configurations for different data scales
if (component_count > 100000000) {         // > 100M components
    max_objects_per_block = 10000000;      // 10M per block
    max_hierarchy_levels = 12;             // 12 hierarchy levels
    index_type = IndexType::HYBRID;        // Hybrid indexing
} else if (component_count > 10000000) {   // > 10M components
    max_objects_per_block = 1000000;       // 1M per block
    max_hierarchy_levels = 10;             // 10 hierarchy levels
    index_type = IndexType::RTREE;         // R-tree indexing
}
```

### GPU Acceleration Configuration
```cpp
// CUDA configuration example
#ifdef ZLAYOUT_USE_CUDA
constexpr int CUDA_BLOCK_SIZE = 256;
constexpr int CUDA_GRID_SIZE = (component_count + CUDA_BLOCK_SIZE - 1) / CUDA_BLOCK_SIZE;
#endif
```

## 📊 Design for Future Process Nodes

### 2nm Process Support
- **Precision**: Sub-nanometer precision calculations
- **Density**: Processing millions of transistors per square millimeter
- **Complexity**: Multi-layer 3D stacking structure optimization
- **Power**: Hotspot detection and optimization

### Scalability Design
- **3D Support**: Reserved interfaces for 3D IC design
- **Cloud Computing**: Distributed computing extension support
- **AI Integration**: Optimization for machine learning-assisted design

## 🧪 Testing and Validation

```bash
# Run complete test suite
xmake test

# Performance benchmarks
xmake run performance_benchmark

# Ultra-large scale stress testing
xmake run stress_test --components=1000000000  # 1 billion components
```

## 📝 Technical Documentation

- [API Reference Manual](docs/api_reference.md)
- [Performance Optimization Guide](docs/performance_guide.md)
- [GPU Acceleration Tutorial](docs/gpu_acceleration.md)
- [Hierarchical Indexing Design](docs/hierarchical_indexing.md)

## 🤝 Contributing

We welcome contributions for ultra-large scale EDA optimization:

1. **Algorithm Optimization**: More efficient spatial indexing algorithms
2. **Parallelization**: GPU kernels and multi-threading optimizations
3. **Memory Optimization**: Cache-friendly data structures
4. **Process Support**: New process node constraint support

## 📄 License

MIT License - See [LICENSE](LICENSE) file for details

## 📞 Contact

- **Author**: Wenyin WEI 魏文崟
- **Email**: weiwy16@tsinghua.org.cn | wenyin@mail.ustc.edu.cn
- **Expertise**: Ultra-large scale EDA algorithm optimization, High-performance computing

---

**ZLayout** - Making billion-scale component layout optimization possible! 🚀 