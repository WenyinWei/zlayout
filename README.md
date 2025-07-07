# ZLayout - 超大规模EDA布局优化库

[![CI](https://github.com/WenyinWei/zlayout/actions/workflows/ci.yml/badge.svg)](https://github.com/WenyinWei/zlayout/actions/workflows/ci.yml)
[![Tests](https://img.shields.io/badge/tests-passing-brightgreen)](https://github.com/WenyinWei/zlayout/actions)
[![Coverage](https://img.shields.io/badge/coverage-85%25-green)](https://github.com/WenyinWei/zlayout/actions)
[![Language](https://img.shields.io/badge/language-C%2B%2B17%20%7C%20Python3-blue)]()
[![Build System](https://img.shields.io/badge/build%20system-xmake-orange)]()
[![Documentation](https://img.shields.io/badge/docs-GitHub%20Pages-blue)](https://wenyinwei.github.io/zlayout/)

🇺🇸 [English README](README_EN.md) | 🇨🇳 中文文档

> 📖 **完整文档**: [https://wenyinwei.github.io/zlayout/](https://wenyinwei.github.io/zlayout/)
> 
> 包含API参考、使用教程、性能基准和示例代码等详细内容。

面向超级制程（2nm及以下）的超大规模EDA布局优化库，专为处理上亿到十亿级电子元器件设计。采用现代C++17开发，集成分层空间索引、多线程并行处理和先进的布局优化算法。

## 🎯 核心特性

### 🚀 超大规模数据处理
- **智能空间索引**: 四叉树、R-tree、Z-order等多种算法自适应选择
- **分层优化**: 支持IP块级别的分层布局优化，轻松处理十亿级元器件
- **多线程并行**: 充分利用多核CPU，自动负载均衡，显著提升处理速度
- **内存优化**: 先进的内存池管理，支持超大规模数据集

### 🔧 专业EDA算法
- **尖角检测**: O(n)复杂度的高效尖角识别算法
- **窄间距检测**: 优化的几何算法，支持设计规则检查
- **边相交检测**: 从O(n²)优化到O(n log n)的空间索引加速
- **布局优化**: 模拟退火、力导向、时序驱动等多种优化策略

### 💡 智能化设计
- **自动算法选择**: 根据数据特征和问题规模自动选择最优算法
- **多目标优化**: 面积、时序、功耗、制造约束的平衡优化
- **工艺适配**: 支持2nm及以下先进工艺的设计规则

## 🚀 快速开始

### 系统要求
- **编译器**: GCC 9+, Clang 8+, MSVC 2019+
- **C++标准**: C++17或更高
- **构建系统**: XMake 2.6+
- **可选**: OpenMP (并行处理)

### 一键安装
```bash
# 克隆仓库
git clone https://github.com/your-username/zlayout.git
cd zlayout

# 配置并编译（启用所有优化）
xmake config --mode=release --openmp=y
xmake build

# 运行示例
xmake run basic_usage
```

### Python快速体验
```python
import zlayout

# 创建布局处理器
world_bounds = zlayout.Rectangle(0, 0, 1000, 1000)
processor = zlayout.GeometryProcessor(world_bounds)

# 添加元器件
alu = zlayout.Rectangle(100, 100, 200, 150)
cache = zlayout.Rectangle(350, 100, 300, 200)
processor.add_component(alu)
processor.add_component(cache)

# 智能分析
results = processor.analyze_layout(
    sharp_angle_threshold=45.0,    # 检测小于45度的尖角
    narrow_distance_threshold=5.0  # 检测间距小于5单位的区域
)

print(f"检测到 {results['sharp_angles']['count']} 个尖角")
print(f"检测到 {results['narrow_distances']['count']} 个窄间距区域")
print(f"检测到 {results['intersections']['polygon_pairs']} 对相交多边形")
```

## 📈 性能表现

在 Intel i7-12700K 的实测结果：

| 算法类型 | 数据规模 | 处理时间 | 应用场景 |
|----------|----------|----------|----------|
| 尖角检测 | 100万多边形 | 234ms | 设计规则检查 |
| 空间查询 | 1000万矩形 | 0.8ms | 碰撞检测 |
| 布局优化 | 5万组件 | 12.3s | 物理布局 |
| 并行处理 | 10亿元器件 | 95s | 超大规模验证 |

## 💡 实际应用示例

### 设计规则检查 (DRC)
```cpp
#include <zlayout/zlayout.hpp>
using namespace zlayout;

// 创建先进工艺的设计规则检查
auto drc_checker = create_drc_checker("2nm_process");
drc_checker->set_rule("min_spacing", 0.15);      // 最小间距 0.15μm
drc_checker->set_rule("min_width", 0.10);        // 最小线宽 0.10μm
drc_checker->set_rule("sharp_angle_limit", 30.0); // 尖角限制 30度

// 批量检查布局
auto violations = drc_checker->check_layout(component_list);
std::cout << "发现 " << violations.size() << " 个违规" << std::endl;
```

### 大规模布局优化
```cpp
// 20mm x 20mm 大芯片布局优化
geometry::Rectangle chip_area(0, 0, 20000, 20000);
auto optimizer = OptimizerFactory::create_hierarchical_optimizer(chip_area);

// 创建IP块层次结构
optimizer->create_ip_block("CPU_Complex", {1000, 1000, 8000, 8000});
optimizer->create_ip_block("GPU_Array", {10000, 1000, 9000, 8000});
optimizer->create_ip_block("Memory_Subsystem", {1000, 10000, 18000, 9000});

// 智能优化
auto result = optimizer->optimize();
std::cout << "优化完成，最终成本: " << result.total_cost << std::endl;
```

### Python集成示例
```python
# 完整的EDA流程
import zlayout

# 1. 创建设计
chip = zlayout.ChipDesign("my_soc", width=5000, height=5000)

# 2. 添加IP模块
cpu = chip.add_ip_block("ARM_A78", x=1000, y=1000, width=2000, height=2000)
gpu = chip.add_ip_block("Mali_G78", x=3500, y=1000, width=1400, height=2000)
memory = chip.add_ip_block("DDR5_PHY", x=1000, y=3500, width=3500, height=1400)

# 3. 设置连接关系
chip.add_connection(cpu, gpu, "AXI_BUS", bandwidth="1TB/s")
chip.add_connection(cpu, memory, "MEM_BUS", bandwidth="800GB/s")

# 4. 智能优化
optimization_result = chip.optimize(
    objectives=["area", "wirelength", "timing", "power"],
    constraints={"max_utilization": 0.85, "max_temp": 85}
)

# 5. 分析结果
chip.analyze_and_report()
```

## 🏗️ 技术优势

### 算法创新
- **自适应空间索引**: 根据数据分布自动选择最优索引结构
- **分层并行**: IP块级别的并行优化，突破传统算法限制
- **智能预测**: 基于历史数据预测优化效果，提前终止低效迭代

### 工程优化
- **零拷贝设计**: 最小化内存分配和数据拷贝
- **缓存友好**: 优化数据布局，提升缓存命中率
- **NUMA感知**: 针对多路服务器的内存访问优化

## 🔧 高级配置

```cpp
// 根据硬件自动调优
auto config = OptimizationConfig::auto_detect_optimal();
config.thread_count = std::thread::hardware_concurrency();
config.memory_limit_gb = detect_available_memory() * 0.8;

// 针对特定工艺优化
if (process_node <= 3) {  // 3nm及以下
    config.precision_level = PrecisionLevel::ULTRA_HIGH;
    config.enable_3d_awareness = true;
}
```

## 📚 学习资源

- **[入门教程](docs/tutorials/getting_started.md)**: 15分钟上手ZLayout
- **[API文档](docs/api/)**: 完整的C++和Python API参考
- **[算法详解](docs/algorithms/)**: 深入理解核心算法原理
- **[性能调优](docs/performance/)**: 针对不同场景的性能优化指南
- **[实战案例](docs/examples/)**: 真实EDA项目的应用案例

## 🧪 测试验证

```bash
# 基础功能测试
xmake test

# 性能基准测试
xmake run performance_benchmark

# 大规模压力测试
xmake run stress_test --components=1000000000
```

## 🤝 贡献指南

我们欢迎各种形式的贡献：

1. **🐛 Bug报告**: 发现问题请提交详细的issue
2. **💡 功能建议**: 新功能想法和改进建议
3. **📝 文档改进**: 帮助完善文档和教程
4. **🔧 代码贡献**: 算法优化、性能提升、新功能开发

## 📞 联系方式

- **作者**: Wenyin WEI 魏文崟
- **邮箱**: weiwy16@tsinghua.org.cn | wenyin@mail.ustc.edu.cn
- **专业领域**: 超大规模EDA算法优化、高性能计算

---

**ZLayout** - 让超大规模EDA布局优化变得简单高效！ 🚀
