/**
 * @file ip_blocks.hpp
 * @brief Standard IP Blocks and Complex Digital Systems
 * 
 * This file contains implementations of standard IP blocks used in modern
 * digital systems, including processors, memory controllers, and communication interfaces.
 */

#pragma once

#include <zlayout/components/component.hpp>
#include <zlayout/components/digital_components.hpp>
#include <memory>
#include <string>
#include <vector>
#include <map>

namespace zlayout {
namespace components {

/**
 * @brief CPU Architecture types
 */
enum class CPUArchitecture {
    ARM_Cortex_A53,    // ARM Cortex-A53 (64-bit)
    ARM_Cortex_A72,    // ARM Cortex-A72 (64-bit)
    ARM_Cortex_A78,    // ARM Cortex-A78 (64-bit) 
    ARM_Cortex_M4,     // ARM Cortex-M4 (32-bit)
    ARM_Cortex_M7,     // ARM Cortex-M7 (32-bit)
    RISC_V_RV32I,      // RISC-V 32-bit Integer
    RISC_V_RV64I,      // RISC-V 64-bit Integer
    x86_64,            // x86-64 Architecture
    Custom             // Custom architecture
};

/**
 * @brief Memory hierarchy types
 */
enum class MemoryHierarchy {
    L1_Cache,          // L1 Cache
    L2_Cache,          // L2 Cache
    L3_Cache,          // L3 Cache
    Main_Memory,       // Main Memory (DDR)
    Storage            // Storage (Flash/SSD)
};

/**
 * @brief Communication interface types
 */
enum class InterfaceType {
    SPI,               // Serial Peripheral Interface
    I2C,               // Inter-Integrated Circuit
    UART,              // Universal Asynchronous Receiver-Transmitter
    USB,               // Universal Serial Bus
    PCIe,              // PCI Express
    Ethernet,          // Ethernet
    WiFi,              // Wi-Fi
    Bluetooth,         // Bluetooth
    CAN,               // Controller Area Network
    Custom             // Custom interface
};

/**
 * @brief Processor Core IP Block
 */
class ProcessorCore : public Component {
public:
    ProcessorCore(const std::string& name, CPUArchitecture arch, size_t core_count = 1);
    
    // Architecture
    CPUArchitecture getArchitecture() const { return architecture_; }
    size_t getCoreCount() const { return core_count_; }
    
    // Performance characteristics
    double getMaxFrequency() const { return max_frequency_; }
    void setMaxFrequency(double freq) { max_frequency_ = freq; }
    
    double getCurrentFrequency() const { return current_frequency_; }
    void setCurrentFrequency(double freq) { current_frequency_ = freq; }
    
    // Cache configuration
    struct CacheConfig {
        size_t size;           // bytes
        size_t associativity;  // ways
        size_t line_size;      // bytes
        double hit_latency;    // cycles
        double miss_latency;   // cycles
    };
    
    void setL1Config(const CacheConfig& config) { l1_config_ = config; }
    void setL2Config(const CacheConfig& config) { l2_config_ = config; }
    void setL3Config(const CacheConfig& config) { l3_config_ = config; }
    
    const CacheConfig& getL1Config() const { return l1_config_; }
    const CacheConfig& getL2Config() const { return l2_config_; }
    const CacheConfig& getL3Config() const { return l3_config_; }
    
    // Pipeline configuration
    size_t getPipelineStages() const { return pipeline_stages_; }
    void setPipelineStages(size_t stages) { pipeline_stages_ = stages; }
    
    // Instruction set features
    bool hasFloatingPoint() const { return has_fpu_; }
    void setFloatingPoint(bool has_fpu) { has_fpu_ = has_fpu; }
    
    bool hasVectorUnit() const { return has_vector_unit_; }
    void setVectorUnit(bool has_vector) { has_vector_unit_ = has_vector; }
    
    // Performance metrics
    double getIPC() const { return ipc_; }  // Instructions Per Cycle
    void setIPC(double ipc) { ipc_ = ipc; }
    
    double getDMIPS() const { return dmips_; }  // Dhrystone MIPS
    void setDMIPS(double dmips) { dmips_ = dmips; }
    
    // Simulation
    void simulate(double time_step) override;
    std::string getDescription() const override;
    
private:
    CPUArchitecture architecture_;
    size_t core_count_;
    double max_frequency_;     // MHz
    double current_frequency_; // MHz
    
    CacheConfig l1_config_;
    CacheConfig l2_config_;
    CacheConfig l3_config_;
    
    size_t pipeline_stages_;
    bool has_fpu_;
    bool has_vector_unit_;
    
    double ipc_;               // Instructions Per Cycle
    double dmips_;             // Dhrystone MIPS
    
    void initializeArchitecture();
    void updatePerformanceMetrics();
};

/**
 * @brief Memory Controller IP Block
 */
class MemoryController : public Component {
public:
    enum class MemoryType {
        DDR3,              // DDR3 SDRAM
        DDR4,              // DDR4 SDRAM
        DDR5,              // DDR5 SDRAM
        LPDDR4,            // Low Power DDR4
        LPDDR5,            // Low Power DDR5
        HBM2,              // High Bandwidth Memory 2
        HBM3,              // High Bandwidth Memory 3
        GDDR6,             // Graphics DDR6
        SRAM,              // Static RAM
        Custom             // Custom memory type
    };
    
    MemoryController(const std::string& name, MemoryType type, size_t channels = 1);
    
    // Memory configuration
    MemoryType getMemoryType() const { return memory_type_; }
    size_t getChannelCount() const { return channel_count_; }
    
    size_t getDataWidth() const { return data_width_; }
    void setDataWidth(size_t width) { data_width_ = width; }
    
    double getFrequency() const { return frequency_; }
    void setFrequency(double freq) { frequency_ = freq; }
    
    // Capacity
    size_t getCapacityPerChannel() const { return capacity_per_channel_; }
    void setCapacityPerChannel(size_t capacity) { capacity_per_channel_ = capacity; }
    
    size_t getTotalCapacity() const { return capacity_per_channel_ * channel_count_; }
    
    // Timing parameters
    struct TimingParams {
        double tCL;        // CAS Latency
        double tRCD;       // RAS to CAS Delay
        double tRP;        // Row Precharge time
        double tRAS;       // Row Active time
        double tRC;        // Row Cycle time
        double tWR;        // Write Recovery time
        double tRFC;       // Refresh Cycle time
    };
    
    void setTimingParams(const TimingParams& params) { timing_params_ = params; }
    const TimingParams& getTimingParams() const { return timing_params_; }
    
    // Performance metrics
    double getBandwidth() const; // GB/s
    double getLatency() const;   // ns
    double getPowerConsumption() const; // W
    
    // Simulation
    void simulate(double time_step) override;
    std::string getDescription() const override;
    
private:
    MemoryType memory_type_;
    size_t channel_count_;
    size_t data_width_;        // bits
    double frequency_;         // MHz
    size_t capacity_per_channel_; // bytes
    
    TimingParams timing_params_;
    
    void initializeMemoryType();
    void calculatePerformanceMetrics();
};

/**
 * @brief GPU Core IP Block
 */
class GPUCore : public Component {
public:
    enum class GPUArchitecture {
        Mali_G78,          // ARM Mali-G78
        Mali_G710,         // ARM Mali-G710
        Adreno_640,        // Qualcomm Adreno 640
        Adreno_730,        // Qualcomm Adreno 730
        PowerVR_GT7600,    // Imagination PowerVR GT7600
        RDNA2,             // AMD RDNA2
        Ampere,            // NVIDIA Ampere
        Custom             // Custom GPU
    };
    
    GPUCore(const std::string& name, GPUArchitecture arch, size_t compute_units = 8);
    
    // Architecture
    GPUArchitecture getArchitecture() const { return architecture_; }
    size_t getComputeUnits() const { return compute_units_; }
    
    // Performance characteristics
    double getMaxFrequency() const { return max_frequency_; }
    void setMaxFrequency(double freq) { max_frequency_ = freq; }
    
    size_t getShaderCores() const { return shader_cores_; }
    void setShaderCores(size_t cores) { shader_cores_ = cores; }
    
    // Memory interface
    size_t getMemoryBusWidth() const { return memory_bus_width_; }
    void setMemoryBusWidth(size_t width) { memory_bus_width_ = width; }
    
    double getMemoryBandwidth() const { return memory_bandwidth_; }
    void setMemoryBandwidth(double bandwidth) { memory_bandwidth_ = bandwidth; }
    
    // Performance metrics
    double getGFLOPS() const { return gflops_; }
    void setGFLOPS(double gflops) { gflops_ = gflops; }
    
    double getTexelFillRate() const { return texel_fill_rate_; }
    void setTexelFillRate(double rate) { texel_fill_rate_ = rate; }
    
    // Simulation
    void simulate(double time_step) override;
    std::string getDescription() const override;
    
private:
    GPUArchitecture architecture_;
    size_t compute_units_;
    double max_frequency_;     // MHz
    size_t shader_cores_;
    size_t memory_bus_width_;  // bits
    double memory_bandwidth_;  // GB/s
    double gflops_;            // GFLOPS
    double texel_fill_rate_;   // Gtexel/s
    
    void initializeArchitecture();
};

/**
 * @brief Digital Signal Processor (DSP) IP Block
 */
class DSPCore : public Component {
public:
    enum class DSPArchitecture {
        TI_C6000,          // Texas Instruments C6000
        TI_C7000,          // Texas Instruments C7000
        Qualcomm_Hexagon,  // Qualcomm Hexagon
        Cadence_Tensilica, // Cadence Tensilica
        Custom             // Custom DSP
    };
    
    DSPCore(const std::string& name, DSPArchitecture arch);
    
    // Architecture
    DSPArchitecture getArchitecture() const { return architecture_; }
    
    // Performance
    double getMaxFrequency() const { return max_frequency_; }
    void setMaxFrequency(double freq) { max_frequency_ = freq; }
    
    size_t getMACUnits() const { return mac_units_; }
    void setMACUnits(size_t units) { mac_units_ = units; }
    
    // Specialized units
    bool hasFFTUnit() const { return has_fft_unit_; }
    void setFFTUnit(bool has_fft) { has_fft_unit_ = has_fft; }
    
    bool hasViterbiUnit() const { return has_viterbi_unit_; }
    void setViterbiUnit(bool has_viterbi) { has_viterbi_unit_ = has_viterbi; }
    
    // Performance metrics
    double getGMACS() const { return gmacs_; }  // Giga MAC/s
    void setGMACS(double gmacs) { gmacs_ = gmacs; }
    
    // Simulation
    void simulate(double time_step) override;
    std::string getDescription() const override;
    
private:
    DSPArchitecture architecture_;
    double max_frequency_;     // MHz
    size_t mac_units_;         // Multiply-Accumulate units
    bool has_fft_unit_;
    bool has_viterbi_unit_;
    double gmacs_;             // Giga MAC/s
    
    void initializeArchitecture();
};

/**
 * @brief Communication Interface IP Block
 */
class CommunicationInterface : public Component {
public:
    CommunicationInterface(const std::string& name, InterfaceType type);
    
    // Interface type
    InterfaceType getInterfaceType() const { return interface_type_; }
    
    // Common parameters
    double getMaxDataRate() const { return max_data_rate_; }
    void setMaxDataRate(double rate) { max_data_rate_ = rate; }
    
    double getCurrentDataRate() const { return current_data_rate_; }
    void setCurrentDataRate(double rate) { current_data_rate_ = rate; }
    
    // Protocol specific parameters
    void setProtocolParameter(const std::string& param, const std::string& value);
    std::string getProtocolParameter(const std::string& param) const;
    
    // Connection status
    bool isConnected() const { return is_connected_; }
    void setConnected(bool connected) { is_connected_ = connected; }
    
    // Simulation
    void simulate(double time_step) override;
    std::string getDescription() const override;
    
private:
    InterfaceType interface_type_;
    double max_data_rate_;     // Mbps
    double current_data_rate_; // Mbps
    bool is_connected_;
    
    std::map<std::string, std::string> protocol_params_;
    
    void initializeInterface();
};

/**
 * @brief Power Management Unit (PMU) IP Block
 */
class PowerManagementUnit : public Component {
public:
    enum class PowerState {
        Active,            // Full power
        Idle,              // Low power idle
        Standby,           // Standby mode
        Sleep,             // Sleep mode
        DeepSleep,         // Deep sleep mode
        Shutdown           // Power off
    };
    
    PowerManagementUnit(const std::string& name);
    
    // Power domains
    void addPowerDomain(const std::string& domain, double voltage, double current);
    void removePowerDomain(const std::string& domain);
    
    // Voltage regulation
    double getVoltage(const std::string& domain) const;
    void setVoltage(const std::string& domain, double voltage);
    
    // Current monitoring
    double getCurrent(const std::string& domain) const;
    double getTotalPower() const;
    
    // Power states
    PowerState getPowerState() const { return power_state_; }
    void setPowerState(PowerState state);
    
    // Dynamic voltage and frequency scaling
    void enableDVFS(bool enable) { dvfs_enabled_ = enable; }
    bool isDVFSEnabled() const { return dvfs_enabled_; }
    
    // Simulation
    void simulate(double time_step) override;
    std::string getDescription() const override;
    
private:
    struct PowerDomain {
        double voltage;        // V
        double current;        // A
        bool is_enabled;
    };
    
    std::map<std::string, PowerDomain> power_domains_;
    PowerState power_state_;
    bool dvfs_enabled_;
    
    void updatePowerMetrics();
};

/**
 * @brief System-on-Chip (SoC) IP Block
 */
class SoC : public Component {
public:
    SoC(const std::string& name, const std::string& part_number);
    
    // Core components
    void addProcessorCore(std::shared_ptr<ProcessorCore> core);
    void addMemoryController(std::shared_ptr<MemoryController> controller);
    void addGPUCore(std::shared_ptr<GPUCore> gpu);
    void addDSPCore(std::shared_ptr<DSPCore> dsp);
    void addInterface(std::shared_ptr<CommunicationInterface> interface);
    void addPMU(std::shared_ptr<PowerManagementUnit> pmu);
    
    // Configuration
    const std::string& getPartNumber() const { return part_number_; }
    
    // Performance analysis
    double getTotalPower() const;
    double getMaxPerformance() const;
    std::vector<std::string> getCriticalPaths() const;
    
    // Hierarchical optimization
    geometry::Rectangle calculateHierarchicalBoundingBox() const override;
    double calculateTotalArea() const override;
    size_t getTotalGateCount() const override;
    
    // Simulation
    void simulate(double time_step) override;
    std::string getDescription() const override;
    
private:
    std::string part_number_;
    
    std::vector<std::shared_ptr<ProcessorCore>> processor_cores_;
    std::vector<std::shared_ptr<MemoryController>> memory_controllers_;
    std::vector<std::shared_ptr<GPUCore>> gpu_cores_;
    std::vector<std::shared_ptr<DSPCore>> dsp_cores_;
    std::vector<std::shared_ptr<CommunicationInterface>> interfaces_;
    std::vector<std::shared_ptr<PowerManagementUnit>> pmus_;
    
    void analyzeInterconnect();
    void optimizeFloorplan();
};

/**
 * @brief Standard IP Block Factory
 */
class IPBlockFactory {
public:
    // Processor cores
    static std::shared_ptr<ProcessorCore> createCortexA53(const std::string& name, size_t cores = 4);
    static std::shared_ptr<ProcessorCore> createCortexA72(const std::string& name, size_t cores = 4);
    static std::shared_ptr<ProcessorCore> createCortexA78(const std::string& name, size_t cores = 4);
    static std::shared_ptr<ProcessorCore> createCortexM4(const std::string& name);
    static std::shared_ptr<ProcessorCore> createCortexM7(const std::string& name);
    static std::shared_ptr<ProcessorCore> createRISCVCore(const std::string& name, CPUArchitecture arch);
    
    // Memory controllers
    static std::shared_ptr<MemoryController> createDDR4Controller(const std::string& name, size_t channels = 2);
    static std::shared_ptr<MemoryController> createDDR5Controller(const std::string& name, size_t channels = 2);
    static std::shared_ptr<MemoryController> createLPDDR5Controller(const std::string& name, size_t channels = 4);
    
    // GPU cores
    static std::shared_ptr<GPUCore> createMaliG78(const std::string& name, size_t compute_units = 16);
    static std::shared_ptr<GPUCore> createAdreno640(const std::string& name);
    static std::shared_ptr<GPUCore> createAdreno730(const std::string& name);
    
    // DSP cores
    static std::shared_ptr<DSPCore> createHexagonDSP(const std::string& name);
    static std::shared_ptr<DSPCore> createTIC6000(const std::string& name);
    
    // Communication interfaces
    static std::shared_ptr<CommunicationInterface> createUSB3Interface(const std::string& name);
    static std::shared_ptr<CommunicationInterface> createPCIe4Interface(const std::string& name);
    static std::shared_ptr<CommunicationInterface> createEthernetInterface(const std::string& name);
    
    // Complete SoCs
    static std::shared_ptr<SoC> createSnapdragon8Gen1(const std::string& name);
    static std::shared_ptr<SoC> createAppleM1(const std::string& name);
    static std::shared_ptr<SoC> createExynos2200(const std::string& name);
    static std::shared_ptr<SoC> createMediaTekDimensity9000(const std::string& name);
    
    // Register all IP blocks in library
    static void registerStandardIPBlocks(ComponentLibrary& library);
};

/**
 * @brief Real-world circuit examples for timing optimization
 */
namespace RealWorldCircuits {
    
    /**
     * @brief Create a realistic CPU design with proper timing constraints
     */
    std::shared_ptr<Component> createCPUDesign(const std::string& name, size_t core_count = 4);
    
    /**
     * @brief Create a memory subsystem with realistic timing
     */
    std::shared_ptr<Component> createMemorySubsystem(const std::string& name);
    
    /**
     * @brief Create a graphics processing unit with compute pipelines
     */
    std::shared_ptr<Component> createGPUDesign(const std::string& name);
    
    /**
     * @brief Create a communication subsystem with multiple interfaces
     */
    std::shared_ptr<Component> createCommunicationSubsystem(const std::string& name);
    
    /**
     * @brief Create a complete smartphone SoC
     */
    std::shared_ptr<Component> createSmartphoneSoC(const std::string& name);
    
    /**
     * @brief Create a server processor with multiple cores
     */
    std::shared_ptr<Component> createServerProcessor(const std::string& name);
    
    /**
     * @brief Create an IoT microcontroller design
     */
    std::shared_ptr<Component> createIoTMicrocontroller(const std::string& name);
}

} // namespace components
} // namespace zlayout 