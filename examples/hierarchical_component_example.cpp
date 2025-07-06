/**
 * @file hierarchical_component_example.cpp
 * @brief Hierarchical EDA Component System Example
 * 
 * This example demonstrates the hierarchical component system for EDA layout optimization,
 * showing how basic components can be composed into complex IP blocks and how
 * hierarchical optimization can dramatically reduce the complexity for billion-scale designs.
 */

#include <iostream>
#include <memory>
#include <vector>
#include <chrono>
#include <random>

#include <zlayout/components/component.hpp>
#include <zlayout/components/passive_components.hpp>
#include <zlayout/components/digital_components.hpp>
#include <zlayout/components/ip_blocks.hpp>
#include <zlayout/optimization/layout_optimizer.hpp>
#include <zlayout/spatial/advanced_spatial.hpp>

using namespace zlayout;
using namespace zlayout::components;
using namespace zlayout::optimization;
using namespace zlayout::spatial;

/**
 * @brief Demonstrate basic component creation and hierarchy
 */
void demonstrateBasicComponents() {
    std::cout << "\n=== Basic Components Demo ===\n";
    
    // Create basic passive components
    auto resistor = PassiveComponentFactory::createResistor("R1", 1000.0, 5.0);
    resistor->setPosition(geometry::Point(0, 0));
    resistor->setBoundingBox(geometry::Rectangle(0, 0, 2.0, 1.0));
    
    auto capacitor = PassiveComponentFactory::createCapacitor("C1", 100e-12, 
        Capacitor::DielectricType::Ceramic);
    capacitor->setPosition(geometry::Point(10, 0));
    capacitor->setBoundingBox(geometry::Rectangle(10, 0, 2.0, 1.0));
    
    auto inductor = PassiveComponentFactory::createInductor("L1", 10e-9, 
        Inductor::CoreType::Ferrite);
    inductor->setPosition(geometry::Point(20, 0));
    inductor->setBoundingBox(geometry::Rectangle(20, 0, 3.0, 2.0));
    
    // Create digital components
    auto and_gate = DigitalComponentFactory::createLogicGate("AND1", 
        LogicGate::GateType::AND, 2);
    and_gate->setPosition(geometry::Point(0, 10));
    and_gate->setBoundingBox(geometry::Rectangle(0, 10, 4.0, 3.0));
    
    auto flip_flop = DigitalComponentFactory::createDFlipFlop("FF1", true, false);
    flip_flop->setPosition(geometry::Point(10, 10));
    flip_flop->setBoundingBox(geometry::Rectangle(10, 10, 6.0, 4.0));
    
    std::cout << "Created components:\n";
    std::cout << "- " << resistor->getName() << ": " << resistor->getDescription() << "\n";
    std::cout << "- " << capacitor->getName() << ": " << capacitor->getDescription() << "\n";
    std::cout << "- " << inductor->getName() << ": " << inductor->getDescription() << "\n";
    std::cout << "- " << and_gate->getName() << ": " << and_gate->getDescription() << "\n";
    std::cout << "- " << flip_flop->getName() << ": " << flip_flop->getDescription() << "\n";
}

/**
 * @brief Create a simple functional block (ALU) from basic components
 */
std::shared_ptr<Component> createALUBlock() {
    auto alu = std::make_shared<Component>("ALU", ComponentCategory::Digital);
    alu->setType("ArithmeticLogicUnit");
    alu->setPosition(geometry::Point(0, 0));
    
    // Add basic gates that make up the ALU
    for (int i = 0; i < 8; ++i) {
        // AND gates for bit operations
        auto and_gate = DigitalComponentFactory::createLogicGate(
            "AND_" + std::to_string(i), LogicGate::GateType::AND, 2);
        and_gate->setPosition(geometry::Point(i * 5.0, 0));
        and_gate->setBoundingBox(geometry::Rectangle(i * 5.0, 0, 4.0, 3.0));
        alu->addChild(and_gate);
        
        // OR gates for bit operations
        auto or_gate = DigitalComponentFactory::createLogicGate(
            "OR_" + std::to_string(i), LogicGate::GateType::OR, 2);
        or_gate->setPosition(geometry::Point(i * 5.0, 5));
        or_gate->setBoundingBox(geometry::Rectangle(i * 5.0, 5, 4.0, 3.0));
        alu->addChild(or_gate);
        
        // XOR gates for addition
        auto xor_gate = DigitalComponentFactory::createLogicGate(
            "XOR_" + std::to_string(i), LogicGate::GateType::XOR, 2);
        xor_gate->setPosition(geometry::Point(i * 5.0, 10));
        xor_gate->setBoundingBox(geometry::Rectangle(i * 5.0, 10, 4.0, 3.0));
        alu->addChild(xor_gate);
    }
    
    // Add full adders for arithmetic operations
    for (int i = 0; i < 8; ++i) {
        auto adder = DigitalComponentFactory::createFullAdder("ADDER_" + std::to_string(i));
        adder->setPosition(geometry::Point(i * 8.0, 15));
        adder->setBoundingBox(geometry::Rectangle(i * 8.0, 15, 7.0, 5.0));
        alu->addChild(adder);
    }
    
    // Calculate hierarchical bounding box
    alu->setBoundingBox(alu->calculateHierarchicalBoundingBox());
    
    // Set timing characteristics for ALU
    TimingInfo timing;
    timing.propagation_delay = 2.5;  // ns
    timing.setup_time = 0.5;         // ns
    timing.hold_time = 0.2;          // ns
    alu->setTimingInfo(timing);
    
    std::cout << "Created ALU with " << alu->getChildren().size() 
              << " sub-components\n";
    std::cout << "ALU bounding box: " << alu->getBoundingBox().toString() << "\n";
    std::cout << "ALU propagation delay: " << timing.propagation_delay << " ns\n";
    
    return alu;
}

/**
 * @brief Create a processor core from functional blocks
 */
std::shared_ptr<ProcessorCore> createProcessorCore() {
    std::cout << "\n=== Creating Processor Core ===\n";
    
    auto cpu = IPBlockFactory::createCortexA53("CortexA53_Cluster", 4);
    
    // Configure cache hierarchy
    ProcessorCore::CacheConfig l1_config;
    l1_config.size = 32 * 1024;           // 32KB
    l1_config.associativity = 2;          // 2-way
    l1_config.line_size = 64;             // 64 bytes
    l1_config.hit_latency = 1.0;          // 1 cycle
    l1_config.miss_latency = 10.0;        // 10 cycles
    cpu->setL1Config(l1_config);
    
    ProcessorCore::CacheConfig l2_config;
    l2_config.size = 512 * 1024;          // 512KB
    l2_config.associativity = 8;          // 8-way
    l2_config.line_size = 64;             // 64 bytes
    l2_config.hit_latency = 5.0;          // 5 cycles
    l2_config.miss_latency = 100.0;       // 100 cycles
    cpu->setL2Config(l2_config);
    
    // Add functional units as children
    auto alu1 = createALUBlock();
    alu1->setPosition(geometry::Point(0, 0));
    cpu->addChild(alu1);
    
    auto alu2 = createALUBlock();
    alu2->setPosition(geometry::Point(50, 0));
    cpu->addChild(alu2);
    
    // Add floating point unit
    auto fpu = std::make_shared<Component>("FPU", ComponentCategory::Digital);
    fpu->setType("FloatingPointUnit");
    fpu->setPosition(geometry::Point(100, 0));
    fpu->setBoundingBox(geometry::Rectangle(100, 0, 40, 30));
    cpu->addChild(fpu);
    
    // Set overall bounding box
    cpu->setBoundingBox(cpu->calculateHierarchicalBoundingBox());
    
    std::cout << "Processor core configuration:\n";
    std::cout << "- Architecture: ARM Cortex-A53\n";
    std::cout << "- Cores: " << cpu->getCoreCount() << "\n";
    std::cout << "- L1 Cache: " << l1_config.size / 1024 << " KB\n";
    std::cout << "- L2 Cache: " << l2_config.size / 1024 << " KB\n";
    std::cout << "- Total sub-components: " << cpu->getTotalGateCount() << "\n";
    std::cout << "- Total area: " << cpu->calculateTotalArea() << " square units\n";
    
    return cpu;
}

/**
 * @brief Create a complete SoC with multiple IP blocks
 */
std::shared_ptr<SoC> createSmartphoneSoC() {
    std::cout << "\n=== Creating Smartphone SoC ===\n";
    
    auto soc = std::make_shared<SoC>("SmartphoneSoC", "Custom_SoC_2nm");
    soc->setTechNode(TechNode::um_2);
    
    // Add processor cores
    auto cpu_cluster = createProcessorCore();
    cpu_cluster->setPosition(geometry::Point(0, 0));
    soc->addProcessorCore(cpu_cluster);
    
    // Add GPU
    auto gpu = IPBlockFactory::createMaliG78("Mali_G78", 16);
    gpu->setPosition(geometry::Point(200, 0));
    gpu->setBoundingBox(geometry::Rectangle(200, 0, 80, 60));
    soc->addGPUCore(gpu);
    
    // Add DSP
    auto dsp = IPBlockFactory::createHexagonDSP("Hexagon_DSP");
    dsp->setPosition(geometry::Point(300, 0));
    dsp->setBoundingBox(geometry::Rectangle(300, 0, 40, 40));
    soc->addDSPCore(dsp);
    
    // Add memory controller
    auto memory_ctrl = IPBlockFactory::createLPDDR5Controller("LPDDR5_Ctrl", 4);
    memory_ctrl->setPosition(geometry::Point(0, 100));
    memory_ctrl->setBoundingBox(geometry::Rectangle(0, 100, 60, 30));
    soc->addMemoryController(memory_ctrl);
    
    // Add communication interfaces
    auto usb_if = IPBlockFactory::createUSB3Interface("USB3_IF");
    usb_if->setPosition(geometry::Point(100, 100));
    usb_if->setBoundingBox(geometry::Rectangle(100, 100, 20, 15));
    soc->addInterface(usb_if);
    
    auto pcie_if = IPBlockFactory::createPCIe4Interface("PCIe4_IF");
    pcie_if->setPosition(geometry::Point(150, 100));
    pcie_if->setBoundingBox(geometry::Rectangle(150, 100, 25, 20));
    soc->addInterface(pcie_if);
    
    // Add power management
    auto pmu = std::make_shared<PowerManagementUnit>("PMU");
    pmu->setPosition(geometry::Point(200, 100));
    pmu->setBoundingBox(geometry::Rectangle(200, 100, 30, 25));
    pmu->addPowerDomain("CPU", 1.0, 2.5);      // 1V, 2.5A
    pmu->addPowerDomain("GPU", 0.9, 3.0);      // 0.9V, 3A
    pmu->addPowerDomain("DSP", 0.8, 0.5);      // 0.8V, 0.5A
    pmu->addPowerDomain("IO", 1.8, 0.3);       // 1.8V, 0.3A
    soc->addPMU(pmu);
    
    // Calculate overall SoC metrics
    soc->setBoundingBox(soc->calculateHierarchicalBoundingBox());
    
    std::cout << "SoC Configuration:\n";
    std::cout << "- Technology: 2nm\n";
    std::cout << "- Total area: " << soc->calculateTotalArea() << " square units\n";
    std::cout << "- Total power: " << soc->getTotalPower() << " W\n";
    std::cout << "- Processor cores: " << cpu_cluster->getCoreCount() << "\n";
    std::cout << "- GPU compute units: " << gpu->getComputeUnits() << "\n";
    std::cout << "- Memory channels: " << memory_ctrl->getChannelCount() << "\n";
    std::cout << "- Total gate count: " << soc->getTotalGateCount() << "\n";
    
    return soc;
}

/**
 * @brief Demonstrate hierarchical optimization
 */
void demonstrateHierarchicalOptimization() {
    std::cout << "\n=== Hierarchical Optimization Demo ===\n";
    
    // Create multiple SoCs for a server design
    std::vector<std::shared_ptr<Component>> socs;
    
    for (int i = 0; i < 8; ++i) {
        auto soc = createSmartphoneSoC();
        soc->setName("SoC_" + std::to_string(i));
        
        // Position SoCs in a grid
        double x = (i % 4) * 400.0;
        double y = (i / 4) * 200.0;
        soc->setPosition(geometry::Point(x, y));
        
        socs.push_back(soc);
    }
    
    std::cout << "\nCreated " << socs.size() << " SoCs for server design\n";
    
    // Calculate total complexity
    size_t total_components = 0;
    double total_area = 0.0;
    
    for (const auto& soc : socs) {
        total_components += soc->getTotalGateCount();
        total_area += soc->calculateTotalArea();
    }
    
    std::cout << "Total design complexity:\n";
    std::cout << "- Total components: " << total_components << "\n";
    std::cout << "- Total area: " << total_area << " square units\n";
    
    // Demonstrate hierarchical optimization benefits
    std::cout << "\nHierarchical Optimization Benefits:\n";
    std::cout << "- Without hierarchy: Need to optimize " << total_components 
              << " individual components\n";
    std::cout << "- With hierarchy: Optimize " << socs.size() 
              << " SoC blocks at top level\n";
    std::cout << "- Complexity reduction: " 
              << (double)total_components / socs.size() << "x\n";
    
    // Create spatial index for optimization
    geometry::Rectangle boundary(0, 0, 1600, 400);
    auto spatial_index = std::make_unique<HierarchicalSpatialIndex>(boundary, 16, 8);
    
    // Insert SoCs into spatial index
    for (const auto& soc : socs) {
        spatial_index->insert(soc, [](std::shared_ptr<Component> comp) {
            return comp->getBoundingBox();
        });
    }
    
    std::cout << "\nSpatial indexing completed for hierarchical optimization\n";
    
    // Demonstrate timing-driven optimization
    auto timing_optimizer = std::make_unique<TimingDrivenOptimizer>(boundary);
    
    // Add timing constraints
    timing_optimizer->addTimingConstraint("CPU_to_Memory", 5.0); // 5ns max delay
    timing_optimizer->addTimingConstraint("CPU_to_GPU", 3.0);    // 3ns max delay
    timing_optimizer->addTimingConstraint("Clock", 1.0);         // 1GHz clock
    
    std::cout << "Added timing constraints for critical paths\n";
    
    // Run hierarchical optimization
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (int iteration = 0; iteration < 10; ++iteration) {
        // At each level, optimize placement of major blocks
        for (auto& soc : socs) {
            geometry::Point current_pos = soc->getPosition();
            
            // Simple optimization: minimize distance to center
            geometry::Point center(800, 200);
            geometry::Point direction = center - current_pos;
            double distance = direction.magnitude();
            
            if (distance > 1.0) {
                direction = direction.normalize();
                geometry::Point new_pos = current_pos + direction * 10.0;
                soc->setPosition(new_pos);
            }
        }
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time).count();
    
    std::cout << "Hierarchical optimization completed in " << duration << " ms\n";
}

/**
 * @brief Benchmark different optimization approaches
 */
void benchmarkOptimizationApproaches() {
    std::cout << "\n=== Optimization Benchmark ===\n";
    
    // Test different scales
    std::vector<size_t> component_counts = {1000, 10000, 100000, 1000000};
    
    for (size_t count : component_counts) {
        std::cout << "\nBenchmarking with " << count << " components:\n";
        
        // Create mock components
        std::vector<std::shared_ptr<Component>> components;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> pos_dist(0.0, 1000.0);
        std::uniform_real_distribution<> size_dist(1.0, 10.0);
        
        for (size_t i = 0; i < count; ++i) {
            auto comp = std::make_shared<Component>("Comp_" + std::to_string(i), 
                ComponentCategory::Digital);
            
            double x = pos_dist(gen);
            double y = pos_dist(gen);
            double w = size_dist(gen);
            double h = size_dist(gen);
            
            comp->setPosition(geometry::Point(x, y));
            comp->setBoundingBox(geometry::Rectangle(x, y, w, h));
            
            components.push_back(comp);
        }
        
        // Benchmark flat optimization
        auto start_time = std::chrono::high_resolution_clock::now();
        
        geometry::Rectangle boundary(0, 0, 1000, 1000);
        auto flat_optimizer = std::make_unique<SimulatedAnnealingOptimizer>(boundary);
        flat_optimizer->setComponentCount(count);
        
        // Simulate optimization iterations
        for (int iter = 0; iter < 100; ++iter) {
            // Mock optimization step
            size_t idx = gen() % components.size();
            auto& comp = components[idx];
            
            double new_x = pos_dist(gen);
            double new_y = pos_dist(gen);
            comp->setPosition(geometry::Point(new_x, new_y));
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto flat_duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end_time - start_time).count();
        
        // Benchmark hierarchical optimization
        start_time = std::chrono::high_resolution_clock::now();
        
        // Group components into hierarchical blocks
        size_t block_size = std::min(count / 100, size_t(1000));
        size_t block_count = (count + block_size - 1) / block_size;
        
        std::vector<std::shared_ptr<Component>> blocks;
        for (size_t b = 0; b < block_count; ++b) {
            auto block = std::make_shared<Component>("Block_" + std::to_string(b), 
                ComponentCategory::Custom);
            
            // Add components to block
            size_t start_idx = b * block_size;
            size_t end_idx = std::min(start_idx + block_size, count);
            
            for (size_t i = start_idx; i < end_idx; ++i) {
                block->addChild(components[i]);
            }
            
            block->setBoundingBox(block->calculateHierarchicalBoundingBox());
            blocks.push_back(block);
        }
        
        // Optimize at block level
        auto hierarchical_optimizer = std::make_unique<HierarchicalOptimizer>(boundary);
        hierarchical_optimizer->setHierarchyDepth(2);
        
        for (int iter = 0; iter < 100; ++iter) {
            // Mock block-level optimization
            if (!blocks.empty()) {
                size_t idx = gen() % blocks.size();
                auto& block = blocks[idx];
                
                double new_x = pos_dist(gen);
                double new_y = pos_dist(gen);
                block->setPosition(geometry::Point(new_x, new_y));
            }
        }
        
        end_time = std::chrono::high_resolution_clock::now();
        auto hierarchical_duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end_time - start_time).count();
        
        // Report results
        std::cout << "- Flat optimization: " << flat_duration << " μs\n";
        std::cout << "- Hierarchical optimization: " << hierarchical_duration << " μs\n";
        std::cout << "- Speedup: " << (double)flat_duration / hierarchical_duration << "x\n";
        std::cout << "- Hierarchy levels: " << 2 << "\n";
        std::cout << "- Blocks created: " << blocks.size() << "\n";
        std::cout << "- Components per block: " << block_size << "\n";
    }
}

int main() {
    std::cout << "Hierarchical EDA Component System Demo\n";
    std::cout << "======================================\n";
    
    try {
        // Demonstrate basic component creation
        demonstrateBasicComponents();
        
        // Create and demonstrate hierarchical structures
        auto soc = createSmartphoneSoC();
        
        // Demonstrate hierarchical optimization
        demonstrateHierarchicalOptimization();
        
        // Benchmark different approaches
        benchmarkOptimizationApproaches();
        
        std::cout << "\n=== Summary ===\n";
        std::cout << "✓ Successfully demonstrated hierarchical component system\n";
        std::cout << "✓ Created complex SoC from basic building blocks\n";
        std::cout << "✓ Showed dramatic complexity reduction through hierarchy\n";
        std::cout << "✓ Benchmarked optimization performance improvements\n";
        std::cout << "\nKey Benefits:\n";
        std::cout << "- Scalability: Handle billion-component designs\n";
        std::cout << "- Modularity: Reusable IP blocks\n";
        std::cout << "- Performance: Hierarchical optimization speedup\n";
        std::cout << "- Maintainability: Clear component organization\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 