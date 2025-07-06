/**
 * @file digital_components.hpp
 * @brief Digital Logic Circuit Components
 * 
 * This file contains implementations of digital logic components
 * including gates, flip-flops, counters, and other digital building blocks.
 */

#pragma once

#include <zlayout/components/component.hpp>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <map>

namespace zlayout {
namespace components {

/**
 * @brief Logic family types
 */
enum class LogicFamily {
    TTL,         // Transistor-Transistor Logic
    CMOS,        // Complementary Metal-Oxide-Semiconductor  
    ECL,         // Emitter-Coupled Logic
    LVTTL,       // Low-Voltage TTL
    LVCMOS,      // Low-Voltage CMOS
    LVDS,        // Low-Voltage Differential Signaling
    Custom       // Custom logic levels
};

/**
 * @brief Base class for digital logic components
 */
class DigitalComponent : public Component {
public:
    DigitalComponent(const std::string& name, ComponentCategory category);
    
    // Logic family
    LogicFamily getLogicFamily() const { return logic_family_; }
    void setLogicFamily(LogicFamily family) { logic_family_ = family; }
    
    // Logic levels
    double getVil() const { return vil_; }  // Input Low Voltage
    void setVil(double vil) { vil_ = vil; }
    
    double getVih() const { return vih_; }  // Input High Voltage
    void setVih(double vih) { vih_ = vih; }
    
    double getVol() const { return vol_; }  // Output Low Voltage
    void setVol(double vol) { vol_ = vol; }
    
    double getVoh() const { return voh_; }  // Output High Voltage
    void setVoh(double voh) { voh_ = voh; }
    
    // Signal evaluation
    virtual void evaluateLogic() = 0;
    virtual void propagateSignals();
    
    // Simulation
    void simulate(double time_step) override;
    
protected:
    LogicFamily logic_family_ = LogicFamily::CMOS;
    double vil_ = 0.8;  // V
    double vih_ = 2.0;  // V  
    double vol_ = 0.4;  // V
    double voh_ = 2.4;  // V
    
    // Helper functions
    SignalState voltageToSignalState(double voltage) const;
    double signalStateToVoltage(SignalState state) const;
};

/**
 * @brief Basic Logic Gates
 */
class LogicGate : public DigitalComponent {
public:
    enum class GateType {
        AND, OR, NOT, NAND, NOR, XOR, XNOR, BUFFER
    };
    
    LogicGate(const std::string& name, GateType type, size_t input_count = 2);
    
    GateType getGateType() const { return gate_type_; }
    size_t getInputCount() const { return input_count_; }
    
    // Logic evaluation
    void evaluateLogic() override;
    std::string getDescription() const override;
    
private:
    GateType gate_type_;
    size_t input_count_;
    
    // Gate-specific logic functions
    SignalState evaluateAnd(const std::vector<SignalState>& inputs) const;
    SignalState evaluateOr(const std::vector<SignalState>& inputs) const;
    SignalState evaluateNot(SignalState input) const;
    SignalState evaluateNand(const std::vector<SignalState>& inputs) const;
    SignalState evaluateNor(const std::vector<SignalState>& inputs) const;
    SignalState evaluateXor(const std::vector<SignalState>& inputs) const;
    SignalState evaluateXnor(const std::vector<SignalState>& inputs) const;
};

/**
 * @brief D Flip-Flop
 */
class DFlipFlop : public DigitalComponent {
public:
    DFlipFlop(const std::string& name, bool has_reset = true, bool has_set = false);
    
    // State
    SignalState getQ() const { return q_; }
    SignalState getQbar() const { return qbar_; }
    
    // Control
    bool hasReset() const { return has_reset_; }
    bool hasSet() const { return has_set_; }
    
    // Clock edge detection
    enum class EdgeType { Rising, Falling, Both };
    EdgeType getEdgeType() const { return edge_type_; }
    void setEdgeType(EdgeType type) { edge_type_ = type; }
    
    // Logic evaluation
    void evaluateLogic() override;
    void reset() override;
    std::string getDescription() const override;
    
private:
    SignalState q_ = SignalState::Low;
    SignalState qbar_ = SignalState::High;
    SignalState last_clock_ = SignalState::Low;
    bool has_reset_;
    bool has_set_;
    EdgeType edge_type_ = EdgeType::Rising;
    
    bool isClockEdge(SignalState current_clock) const;
};

/**
 * @brief JK Flip-Flop
 */
class JKFlipFlop : public DigitalComponent {
public:
    JKFlipFlop(const std::string& name, bool has_reset = true, bool has_set = false);
    
    // State
    SignalState getQ() const { return q_; }
    SignalState getQbar() const { return qbar_; }
    
    // Control
    bool hasReset() const { return has_reset_; }
    bool hasSet() const { return has_set_; }
    
    // Logic evaluation
    void evaluateLogic() override;
    void reset() override;
    std::string getDescription() const override;
    
private:
    SignalState q_ = SignalState::Low;
    SignalState qbar_ = SignalState::High;
    SignalState last_clock_ = SignalState::Low;
    bool has_reset_;
    bool has_set_;
};

/**
 * @brief SR Latch
 */
class SRLatch : public DigitalComponent {
public:
    SRLatch(const std::string& name, bool is_gated = false);
    
    // State
    SignalState getQ() const { return q_; }
    SignalState getQbar() const { return qbar_; }
    
    bool isGated() const { return is_gated_; }
    
    // Logic evaluation
    void evaluateLogic() override;
    void reset() override;
    std::string getDescription() const override;
    
private:
    SignalState q_ = SignalState::Low;
    SignalState qbar_ = SignalState::High;
    bool is_gated_;
};

/**
 * @brief Binary Counter
 */
class BinaryCounter : public DigitalComponent {
public:
    BinaryCounter(const std::string& name, size_t width, bool is_up_counter = true);
    
    // Counter properties
    size_t getWidth() const { return width_; }
    bool isUpCounter() const { return is_up_counter_; }
    
    // Count value
    uint64_t getCount() const { return count_; }
    void setCount(uint64_t value);
    
    uint64_t getMaxCount() const { return max_count_; }
    
    // Control
    void enableCount(bool enable) { count_enable_ = enable; }
    bool isCountEnabled() const { return count_enable_; }
    
    // Logic evaluation
    void evaluateLogic() override;
    void reset() override;
    std::string getDescription() const override;
    
private:
    size_t width_;
    bool is_up_counter_;
    uint64_t count_ = 0;
    uint64_t max_count_;
    bool count_enable_ = true;
    SignalState last_clock_ = SignalState::Low;
    
    void updateOutputs();
};

/**
 * @brief Shift Register
 */
class ShiftRegister : public DigitalComponent {
public:
    enum class ShiftType {
        SISO,  // Serial In, Serial Out
        SIPO,  // Serial In, Parallel Out
        PISO,  // Parallel In, Serial Out
        PIPO   // Parallel In, Parallel Out
    };
    
    ShiftRegister(const std::string& name, size_t width, ShiftType type);
    
    // Properties
    size_t getWidth() const { return width_; }
    ShiftType getShiftType() const { return shift_type_; }
    
    // Direction
    enum class Direction { Left, Right };
    Direction getDirection() const { return direction_; }
    void setDirection(Direction dir) { direction_ = dir; }
    
    // Data
    const std::vector<SignalState>& getData() const { return data_; }
    void setData(const std::vector<SignalState>& data);
    
    // Logic evaluation
    void evaluateLogic() override;
    void reset() override;
    std::string getDescription() const override;
    
private:
    size_t width_;
    ShiftType shift_type_;
    Direction direction_ = Direction::Right;
    std::vector<SignalState> data_;
    SignalState last_clock_ = SignalState::Low;
    
    void shiftData();
    void updateOutputs();
};

/**
 * @brief Multiplexer
 */
class Multiplexer : public DigitalComponent {
public:
    Multiplexer(const std::string& name, size_t input_count);
    
    // Properties
    size_t getInputCount() const { return input_count_; }
    size_t getSelectWidth() const { return select_width_; }
    
    // Current selection
    size_t getSelectedInput() const { return selected_input_; }
    
    // Logic evaluation
    void evaluateLogic() override;
    std::string getDescription() const override;
    
private:
    size_t input_count_;
    size_t select_width_;
    size_t selected_input_ = 0;
    
    size_t calculateSelectValue() const;
};

/**
 * @brief Demultiplexer
 */
class Demultiplexer : public DigitalComponent {
public:
    Demultiplexer(const std::string& name, size_t output_count);
    
    // Properties
    size_t getOutputCount() const { return output_count_; }
    size_t getSelectWidth() const { return select_width_; }
    
    // Current selection
    size_t getSelectedOutput() const { return selected_output_; }
    
    // Logic evaluation
    void evaluateLogic() override;
    std::string getDescription() const override;
    
private:
    size_t output_count_;
    size_t select_width_;
    size_t selected_output_ = 0;
    
    size_t calculateSelectValue() const;
};

/**
 * @brief Decoder
 */
class Decoder : public DigitalComponent {
public:
    Decoder(const std::string& name, size_t input_width, bool has_enable = true);
    
    // Properties
    size_t getInputWidth() const { return input_width_; }
    size_t getOutputCount() const { return output_count_; }
    bool hasEnable() const { return has_enable_; }
    
    // Logic evaluation
    void evaluateLogic() override;
    std::string getDescription() const override;
    
private:
    size_t input_width_;
    size_t output_count_;
    bool has_enable_;
    
    size_t calculateInputValue() const;
};

/**
 * @brief Encoder
 */
class Encoder : public DigitalComponent {
public:
    Encoder(const std::string& name, size_t input_count, bool is_priority = false);
    
    // Properties
    size_t getInputCount() const { return input_count_; }
    size_t getOutputWidth() const { return output_width_; }
    bool isPriority() const { return is_priority_; }
    
    // Logic evaluation
    void evaluateLogic() override;
    std::string getDescription() const override;
    
private:
    size_t input_count_;
    size_t output_width_;
    bool is_priority_;
    
    size_t findActiveInput() const;
    size_t findHighestPriorityInput() const;
};

/**
 * @brief Full Adder
 */
class FullAdder : public DigitalComponent {
public:
    FullAdder(const std::string& name);
    
    // Logic evaluation
    void evaluateLogic() override;
    std::string getDescription() const override;
};

/**
 * @brief Ripple Carry Adder
 */
class RippleCarryAdder : public DigitalComponent {
public:
    RippleCarryAdder(const std::string& name, size_t width);
    
    // Properties
    size_t getWidth() const { return width_; }
    
    // Values
    uint64_t getA() const { return a_; }
    uint64_t getB() const { return b_; }
    uint64_t getSum() const { return sum_; }
    bool getCarryOut() const { return carry_out_; }
    
    // Logic evaluation
    void evaluateLogic() override;
    std::string getDescription() const override;
    
private:
    size_t width_;
    uint64_t a_ = 0;
    uint64_t b_ = 0;
    uint64_t sum_ = 0;
    bool carry_out_ = false;
    
    void calculateInputValues();
    void updateOutputs();
};

/**
 * @brief Memory component (RAM/ROM)
 */
class Memory : public DigitalComponent {
public:
    enum class MemoryType {
        RAM,    // Random Access Memory
        ROM,    // Read-Only Memory
        EEPROM, // Electrically Erasable Programmable ROM
        Flash   // Flash Memory
    };
    
    Memory(const std::string& name, size_t address_width, size_t data_width, MemoryType type);
    
    // Properties
    size_t getAddressWidth() const { return address_width_; }
    size_t getDataWidth() const { return data_width_; }
    size_t getMemorySize() const { return memory_size_; }
    MemoryType getMemoryType() const { return memory_type_; }
    
    // Memory operations
    uint64_t readData(uint64_t address) const;
    void writeData(uint64_t address, uint64_t data);
    
    // Initialization
    void loadFromFile(const std::string& filename);
    void saveToFile(const std::string& filename) const;
    
    // Logic evaluation
    void evaluateLogic() override;
    std::string getDescription() const override;
    
private:
    size_t address_width_;
    size_t data_width_;
    size_t memory_size_;
    MemoryType memory_type_;
    std::vector<uint64_t> memory_data_;
    
    uint64_t current_address_ = 0;
    uint64_t current_data_ = 0;
    SignalState last_clock_ = SignalState::Low;
    SignalState last_write_enable_ = SignalState::Low;
    
    void updateCurrentAddress();
    void updateCurrentData();
    void updateOutputs();
};

/**
 * @brief Digital component factory
 */
class DigitalComponentFactory {
public:
    // Basic gates
    static std::shared_ptr<LogicGate> createLogicGate(
        const std::string& name, LogicGate::GateType type, size_t input_count = 2);
    
    // Flip-flops and latches
    static std::shared_ptr<DFlipFlop> createDFlipFlop(
        const std::string& name, bool has_reset = true, bool has_set = false);
    
    static std::shared_ptr<JKFlipFlop> createJKFlipFlop(
        const std::string& name, bool has_reset = true, bool has_set = false);
    
    static std::shared_ptr<SRLatch> createSRLatch(
        const std::string& name, bool is_gated = false);
    
    // Counters and registers
    static std::shared_ptr<BinaryCounter> createBinaryCounter(
        const std::string& name, size_t width, bool is_up_counter = true);
    
    static std::shared_ptr<ShiftRegister> createShiftRegister(
        const std::string& name, size_t width, ShiftRegister::ShiftType type);
    
    // Combinational logic
    static std::shared_ptr<Multiplexer> createMultiplexer(
        const std::string& name, size_t input_count);
    
    static std::shared_ptr<Demultiplexer> createDemultiplexer(
        const std::string& name, size_t output_count);
    
    static std::shared_ptr<Decoder> createDecoder(
        const std::string& name, size_t input_width, bool has_enable = true);
    
    static std::shared_ptr<Encoder> createEncoder(
        const std::string& name, size_t input_count, bool is_priority = false);
    
    // Arithmetic
    static std::shared_ptr<FullAdder> createFullAdder(const std::string& name);
    
    static std::shared_ptr<RippleCarryAdder> createRippleCarryAdder(
        const std::string& name, size_t width);
    
    // Memory
    static std::shared_ptr<Memory> createMemory(
        const std::string& name, size_t address_width, size_t data_width, 
        Memory::MemoryType type);
    
    // Standard digital library components
    static void registerStandardComponents(ComponentLibrary& library);
    
    // Common digital building blocks
    static std::shared_ptr<Component> create74Series(
        const std::string& part_number, const std::string& name);
    
    static std::shared_ptr<Component> create4000Series(
        const std::string& part_number, const std::string& name);
};

} // namespace components
} // namespace zlayout 