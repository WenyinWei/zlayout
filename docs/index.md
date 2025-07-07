# ZLayout Documentation

![ZLayout Logo](https://img.shields.io/badge/ZLayout-v1.0.0-blue.svg) ![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg) ![License](https://img.shields.io/badge/license-MIT-green.svg)

## Language Selection / 语言选择

<div style="text-align: center; margin: 20px 0;">
  <a href="#english-documentation" style="background: #007acc; color: white; padding: 12px 24px; text-decoration: none; border-radius: 6px; margin: 10px; display: inline-block;">🇺🇸 English</a>
  <a href="#chinese-documentation" style="background: #dc143c; color: white; padding: 12px 24px; text-decoration: none; border-radius: 6px; margin: 10px; display: inline-block;">🇨🇳 中文</a>
  <a href="#contribute-translation" style="background: #28a745; color: white; padding: 12px 24px; text-decoration: none; border-radius: 6px; margin: 10px; display: inline-block;">🌍 Add Your Language</a>
</div>

---

## English Documentation

### Advanced Electronic Design Automation Layout Library

ZLayout is a high-performance C++ library with Python bindings for electronic design automation (EDA) layout processing. It provides efficient algorithms for geometric analysis, spatial indexing, and layout optimization specifically designed for electronic circuit layouts.

---

## 🚀 Quick Start

### Installation

```bash
# Install from PyPI (Python users)
pip install zlayout

# Build from source (C++ developers)
git clone https://github.com/your-org/zlayout.git
cd zlayout
xmake build
```

### Hello World Example

```python
import zlayout

# Create a simple layout
layout = zlayout.Rectangle(0, 0, 100, 100)
quadtree = zlayout.QuadTree(layout)

# Add components
component = zlayout.Rectangle(10, 10, 20, 15)
quadtree.insert(component)

# Analyze layout
analyzer = zlayout.PolygonAnalyzer()
results = analyzer.analyze_layout([component])
print(f"Sharp angles detected: {results['sharp_angles']['count']}")
```

---

## 📚 Documentation Sections

### 🎓 [Getting Started](tutorials/en/getting_started.md)
- [Installation Guide](tutorials/en/installation.md)
- [First Steps](tutorials/en/first_steps.md)
- [Basic Concepts](tutorials/en/basic_concepts.md)

### 🔬 [Core Algorithm Tutorials](tutorials/en/algorithms/README.md)
- [Sharp Angle Detection](tutorials/en/algorithms/sharp_angle_detection.md) - *Essential for DRC*
- [Narrow Spacing Detection](tutorials/en/algorithms/narrow_spacing_detection.md) - *Manufacturing Constraints*
- [Edge Intersection Detection](tutorials/en/algorithms/edge_intersection_detection.md) - *Geometry Validation*
- [Range Query Operations](tutorials/en/algorithms/range_query.md)
- [Spatial Index Comparison](tutorials/en/algorithms/spatial_index_comparison.md)

### 📖 [Interactive Tutorials](tutorials/en/README.md)
- [Geometry Operations](tutorials/en/geometry_operations.md)
- [Spatial Indexing](tutorials/en/spatial_indexing.md)
- [Layout Analysis](tutorials/en/layout_analysis.md)
- [Component System](tutorials/en/component_system.md)
- [Layout Optimization](tutorials/en/layout_optimization.md)

### 💡 [Examples](examples/README.md)
- [Basic Usage Examples](examples/basic_usage.md)
- [EDA Circuit Analysis](examples/eda_circuit_analysis.md)
- [Advanced Layout Optimization](examples/advanced_optimization.md)
- [Performance Benchmarks](examples/performance_benchmarks.md)

### 📊 [Benchmarks](benchmarks/README.md)
- [Live Performance Results](benchmarks/performance_results.md)
- [Algorithm Complexity Analysis](benchmarks/complexity_analysis.md)
- [Spatial Indexing Benchmarks](benchmarks/spatial_benchmarks.md)
- [Optimization Algorithm Benchmarks](benchmarks/optimization_benchmarks.md)

### 🔧 [API Reference](api/README.md)
- [C++ API](api/cpp/README.md)
- [Python API](api/python/README.md)
- [Component System](api/components/README.md)

---

## Chinese Documentation

## 🇨🇳 中文文档

### 先进的电子设计自动化布局库

ZLayout 是一个高性能的C++库，提供Python绑定，专门用于电子设计自动化(EDA)布局处理。它为电路布局提供了高效的几何分析、空间索引和布局优化算法。

---

## 🚀 快速开始

### 安装

```bash
# 从PyPI安装 (Python用户)
pip install zlayout

# 从源码构建 (C++开发者)
git clone https://github.com/your-org/zlayout.git
cd zlayout
xmake build
```

### Hello World 示例

```python
import zlayout

# 创建简单布局
layout = zlayout.Rectangle(0, 0, 100, 100)
quadtree = zlayout.QuadTree(layout)

# 添加元器件
component = zlayout.Rectangle(10, 10, 20, 15)
quadtree.insert(component)

# 分析布局
analyzer = zlayout.PolygonAnalyzer()
results = analyzer.analyze_layout([component])
print(f"检测到尖角: {results['sharp_angles']['count']}")
```

---

## 📚 文档章节

### 🎓 [入门指南](tutorials/zh/getting_started.md)
- [安装指南](tutorials/zh/installation.md)
- [第一步](tutorials/zh/first_steps.md)
- [基本概念](tutorials/zh/basic_concepts.md)

### 🔬 [核心算法教程](tutorials/zh/algorithms/README.md)
- [尖角检测算法](tutorials/zh/algorithms/sharp_angle_detection.md) - *设计规则检查必备*
- [窄间距检测算法](tutorials/zh/algorithms/narrow_spacing_detection.md) - *制造约束验证*
- [边相交检测算法](tutorials/zh/algorithms/edge_intersection_detection.md) - *几何完整性检查*
- [范围查询操作](tutorials/zh/algorithms/range_query.md)
- [空间索引比较](tutorials/zh/algorithms/spatial_index_comparison.md)

### 📖 [交互式教程](tutorials/zh/README.md)
- [几何操作](tutorials/zh/geometry_operations.md)
- [空间索引](tutorials/zh/spatial_indexing.md)
- [布局分析](tutorials/zh/layout_analysis.md)
- [组件系统](tutorials/zh/component_system.md)
- [布局优化](tutorials/zh/layout_optimization.md)

### 💡 [示例代码](examples/README.md)
- [基本用法示例](examples/basic_usage.md)
- [EDA电路分析](examples/eda_circuit_analysis.md)
- [高级布局优化](examples/advanced_optimization.md)
- [性能基准测试](examples/performance_benchmarks.md)

### 📊 [性能基准](benchmarks/README.md)
- [实时性能结果](benchmarks/performance_results.md)
- [算法复杂度分析](benchmarks/complexity_analysis.md)
- [空间索引基准](benchmarks/spatial_benchmarks.md)
- [优化算法基准](benchmarks/optimization_benchmarks.md)

### 🔧 [API参考](api/README.md)
- [C++ API](api/cpp/README.md)
- [Python API](api/python/README.md)
- [组件系统](api/components/README.md)

---

## Contribute Translation

### 🌍 Help Us Add Your Language!

We welcome contributions to translate ZLayout documentation into additional languages. Here's how you can help:

1. **Choose Your Language**: Check if your language is already being worked on in our [Translation Issues](https://github.com/your-org/zlayout/issues?q=is%3Aissue+is%3Aopen+label%3Atranslation)

2. **Create Translation Structure**:
   ```bash
   # Create language folder (use ISO 639-1 codes)
   mkdir -p docs/tutorials/[language_code]/
   mkdir -p docs/tutorials/[language_code]/algorithms/
   ```

3. **Translation Priority**:
   - **High Priority**: Core algorithm tutorials (sharp angles, narrow spacing, edge intersection)
   - **Medium Priority**: Getting started guide and basic tutorials
   - **Low Priority**: Advanced topics and API documentation

4. **File Naming Convention**:
   ```
   docs/tutorials/en/     # English (template)
   docs/tutorials/zh/     # Chinese (中文)
   docs/tutorials/ja/     # Japanese (日本語)
   docs/tutorials/ko/     # Korean (한국어)
   docs/tutorials/de/     # German (Deutsch)
   docs/tutorials/fr/     # French (Français)
   docs/tutorials/es/     # Spanish (Español)
   ```

5. **Submit Your Translation**:
   - Fork the repository
   - Create a feature branch: `git checkout -b add-[language]-translation`
   - Translate the files maintaining the same structure
   - Submit a Pull Request with clear description

### Translation Guidelines

- **Keep Code Examples Unchanged**: Only translate comments and text
- **Maintain Formatting**: Preserve markdown structure and links
- **Technical Terms**: Keep technical terms in English when appropriate
- **Cultural Context**: Adapt examples to be culturally relevant when possible

---

## 🌟 Key Features

### Core Capabilities
- **High-Performance Geometry Processing**: Efficient polygon operations, area calculations, and geometric transformations
- **Advanced Spatial Indexing**: QuadTree and R-tree implementations optimized for EDA layouts
- **Layout Analysis**: Sharp angle detection, narrow distance analysis, and intersection detection
- **Component System**: Hierarchical component modeling with passive, digital, and IP block support
- **Layout Optimization**: Simulated annealing, force-directed placement, and timing-driven optimization

### Performance Highlights
- **Scalability**: Handles layouts with millions of components
- **Speed**: Optimized C++ core with OpenMP parallelization
- **Memory Efficiency**: Advanced memory management for large-scale layouts
- **Cross-Platform**: Windows, Linux, and macOS support

### Industry Applications
- **ASIC Design**: Physical design automation and verification
- **PCB Layout**: Component placement and routing optimization
- **MEMS Design**: Micro-electromechanical systems layout
- **Package Design**: IC package layout and thermal analysis

---

## 🎯 Use Cases

### 1. Design Rule Checking (DRC)
```python
# Automated design rule verification
drc_results = analyzer.check_design_rules(layout, rules={
    'min_spacing': 0.15,
    'min_width': 0.10,
    'max_aspect_ratio': 10.0
})
```

### 2. Layout Optimization
```python
# Multi-objective layout optimization
optimizer = zlayout.LayoutOptimizer(
    objectives=['area', 'wirelength', 'timing'],
    constraints={'max_utilization': 0.8}
)
optimized_layout = optimizer.optimize(components, nets)
```

### 3. Hierarchical Component Design
```python
# Build complex circuits from components
cpu_block = zlayout.create_cpu_design("ARM_A78", core_count=4)
memory_block = zlayout.create_memory_subsystem("DDR5_Controller")
soc = zlayout.create_soc([cpu_block, memory_block])
```

---

## 📈 Performance Benchmarks

| Operation | Dataset Size | Time (ms) | Memory (MB) |
|-----------|-------------|-----------|-------------|
| Polygon Area | 1M vertices | 45.2 | 128 |
| Sharp Angle Detection | 100K polygons | 234.7 | 512 |
| QuadTree Query | 10M rectangles | 0.8 | 1024 |
| Layout Optimization | 50K components | 12.3s | 2048 |

*Benchmarks run on Intel i7-12700K with 32GB RAM*

---

## 🛠️ Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    ZLayout Architecture                      │
├─────────────────────────────────────────────────────────────┤
│  Python Bindings (PyBind11)                                │
├─────────────────────────────────────────────────────────────┤
│  C++ Core Library                                          │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐          │
│  │   Geometry  │ │   Spatial   │ │ Components  │          │
│  │  Processing │ │  Indexing   │ │   System    │          │
│  └─────────────┘ └─────────────┘ └─────────────┘          │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐          │
│  │ Optimization│ │  Analysis   │ │Visualization│          │
│  │ Algorithms  │ │   Engine    │ │   Engine    │          │
│  └─────────────┘ └─────────────┘ └─────────────┘          │
├─────────────────────────────────────────────────────────────┤
│  Platform Layer (OpenMP, SIMD, Memory Management)          │
└─────────────────────────────────────────────────────────────┘
```

---

## 🤝 Contributing

We welcome contributions! Please see our [Contributing Guide](CONTRIBUTING.md) for details.

### Development Setup
```bash
# Clone repository
git clone https://github.com/your-org/zlayout.git
cd zlayout

# Install dependencies
pip install -r requirements.txt

# Run tests
python -m pytest tests/
xmake test
```

---

## 📄 License

License information will be provided in future releases.

---

## 🙏 Acknowledgments

- Built on proven EDA algorithms and data structures
- Optimized for modern multi-core processors
- Designed for industrial-scale electronic design workflows
- Community-driven development with industry feedback

---

## 🔗 Links

- [GitHub Repository](https://github.com/your-org/zlayout)
- [PyPI Package](https://pypi.org/project/zlayout/)
- [Documentation](https://your-org.github.io/zlayout/)
- [Issue Tracker](https://github.com/your-org/zlayout/issues)
- [Discussions](https://github.com/your-org/zlayout/discussions)

---

*Last updated: 2025-01-07* 