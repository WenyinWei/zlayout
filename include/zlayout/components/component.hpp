/**
 * @file component.hpp
 * @brief Hierarchical EDA Component System
 * 
 * This system supports hierarchical composition of electronic components,
 * from basic passive components to complex IP blocks.
 */

#pragma once

#include <memory>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <functional>
#include <zlayout/geometry/rectangle.hpp>
#include <zlayout/geometry/point.hpp>

namespace zlayout {
namespace components {

// Forward declarations
class Component;
class Pin;
class Net;
class ComponentLibrary;

/**
 * @brief Pin types for component connections
 */
enum class PinType {
    Input,       // Signal input
    Output,      // Signal output
    Bidirectional, // Bidirectional signal
    Power,       // Power supply
    Ground,      // Ground
    Clock,       // Clock signal
    Reset,       // Reset signal
    Analog       // Analog signal
};

/**
 * @brief Signal states in digital circuits
 */
enum class SignalState {
    Low = 0,     // Logic 0
    High = 1,    // Logic 1
    Unknown = 2, // Unknown state
    HighZ = 3    // High impedance
};

/**
 * @brief Component categories for organization
 */
enum class ComponentCategory {
    Passive,      // R, L, C
    Active,       // Diodes, Transistors
    Digital,      // Logic gates, Flip-flops
    Analog,       // Op-amps, Comparators
    Mixed,        // ADC, DAC
    Memory,       // SRAM, DRAM, Flash
    Processor,    // CPU, GPU, DSP
    Interface,    // I/O, Communication
    Power,        // Regulators, Converters
    Custom        // User-defined
};

/**
 * @brief Technology nodes for manufacturing
 */
enum class TechNode {
    um_180 = 180,  // 180nm
    um_130 = 130,  // 130nm
    um_90 = 90,    // 90nm
    um_65 = 65,    // 65nm
    um_45 = 45,    // 45nm
    um_32 = 32,    // 32nm
    um_22 = 22,    // 22nm
    um_14 = 14,    // 14nm
    um_10 = 10,    // 10nm
    um_7 = 7,      // 7nm
    um_5 = 5,      // 5nm
    um_3 = 3,      // 3nm
    um_2 = 2       // 2nm
};

/**
 * @brief Pin class representing component connection points
 */
class Pin {
public:
    Pin(const std::string& name, PinType type, const geometry::Point& position);
    
    // Getters
    const std::string& getName() const { return name_; }
    PinType getType() const { return type_; }
    const geometry::Point& getPosition() const { return position_; }
    SignalState getSignalState() const { return signal_state_; }
    
    // Setters
    void setSignalState(SignalState state) { signal_state_ = state; }
    void setPosition(const geometry::Point& pos) { position_ = pos; }
    
    // Connection management
    void connectToNet(std::shared_ptr<Net> net);
    void disconnect();
    std::shared_ptr<Net> getNet() const { return net_; }
    bool isConnected() const { return net_ != nullptr; }
    
private:
    std::string name_;
    PinType type_;
    geometry::Point position_;
    SignalState signal_state_ = SignalState::Unknown;
    std::shared_ptr<Net> net_;
};

/**
 * @brief Net class representing electrical connections
 */
class Net {
public:
    explicit Net(const std::string& name);
    
    const std::string& getName() const { return name_; }
    
    // Pin management
    void addPin(std::shared_ptr<Pin> pin);
    void removePin(std::shared_ptr<Pin> pin);
    const std::vector<std::shared_ptr<Pin>>& getPins() const { return pins_; }
    
    // Signal propagation
    void propagateSignal(SignalState state);
    
private:
    std::string name_;
    std::vector<std::shared_ptr<Pin>> pins_;
};

/**
 * @brief Component timing information
 */
struct TimingInfo {
    double propagation_delay = 0.0;  // ns
    double setup_time = 0.0;         // ns
    double hold_time = 0.0;          // ns
    double clock_to_q = 0.0;         // ns
    double max_frequency = 0.0;      // MHz
};

/**
 * @brief Component power information
 */
struct PowerInfo {
    double static_power = 0.0;       // μW
    double dynamic_power = 0.0;      // μW per MHz
    double leakage_current = 0.0;    // nA
    double supply_voltage = 0.0;     // V
    double threshold_voltage = 0.0;  // V
};

/**
 * @brief Component electrical characteristics
 */
struct ElectricalInfo {
    double resistance = 0.0;         // Ω
    double capacitance = 0.0;        // pF
    double inductance = 0.0;         // nH
    double current_rating = 0.0;     // mA
    double voltage_rating = 0.0;     // V
    double tolerance = 0.0;          // %
};

/**
 * @brief Base class for all EDA components
 */
class Component {
public:
    Component(const std::string& name, ComponentCategory category);
    virtual ~Component() = default;
    
    // Basic information
    const std::string& getName() const { return name_; }
    const std::string& getType() const { return type_; }
    ComponentCategory getCategory() const { return category_; }
    
    void setName(const std::string& name) { name_ = name; }
    void setType(const std::string& type) { type_ = type; }
    
    // Hierarchical structure
    void addChild(std::shared_ptr<Component> child);
    void removeChild(const std::string& name);
    std::shared_ptr<Component> getChild(const std::string& name) const;
    const std::vector<std::shared_ptr<Component>>& getChildren() const { return children_; }
    
    std::shared_ptr<Component> getParent() const { return parent_.lock(); }
    void setParent(std::shared_ptr<Component> parent) { parent_ = parent; }
    
    // Geometry
    const geometry::Rectangle& getBoundingBox() const { return bounding_box_; }
    void setBoundingBox(const geometry::Rectangle& bbox) { bounding_box_ = bbox; }
    
    geometry::Point getPosition() const { return position_; }
    void setPosition(const geometry::Point& pos) { position_ = pos; }
    
    double getRotation() const { return rotation_; }
    void setRotation(double angle) { rotation_ = angle; }
    
    // Pin management
    void addPin(std::shared_ptr<Pin> pin);
    void removePin(const std::string& name);
    std::shared_ptr<Pin> getPin(const std::string& name) const;
    const std::vector<std::shared_ptr<Pin>>& getPins() const { return pins_; }
    
    // Properties
    void setProperty(const std::string& key, const std::string& value);
    std::string getProperty(const std::string& key) const;
    const std::map<std::string, std::string>& getProperties() const { return properties_; }
    
    // Characteristics
    void setTimingInfo(const TimingInfo& timing) { timing_ = timing; }
    const TimingInfo& getTimingInfo() const { return timing_; }
    
    void setPowerInfo(const PowerInfo& power) { power_ = power; }
    const PowerInfo& getPowerInfo() const { return power_; }
    
    void setElectricalInfo(const ElectricalInfo& electrical) { electrical_ = electrical; }
    const ElectricalInfo& getElectricalInfo() const { return electrical_; }
    
    void setTechNode(TechNode node) { tech_node_ = node; }
    TechNode getTechNode() const { return tech_node_; }
    
    // Virtual methods for derived classes
    virtual void simulate(double time_step) {}
    virtual void reset() {}
    virtual std::string getDescription() const { return "Generic Component"; }
    
    // Hierarchical optimization support
    virtual geometry::Rectangle calculateHierarchicalBoundingBox() const;
    virtual double calculateTotalArea() const;
    virtual size_t getTotalGateCount() const;
    virtual std::vector<std::shared_ptr<Component>> flattenHierarchy() const;
    
    // Serialization
    virtual std::string serialize() const;
    virtual void deserialize(const std::string& data);
    
protected:
    std::string name_;
    std::string type_;
    ComponentCategory category_;
    
    // Hierarchical structure
    std::vector<std::shared_ptr<Component>> children_;
    std::weak_ptr<Component> parent_;
    
    // Geometry
    geometry::Rectangle bounding_box_;
    geometry::Point position_;
    double rotation_ = 0.0;
    
    // Pins
    std::vector<std::shared_ptr<Pin>> pins_;
    std::unordered_map<std::string, std::shared_ptr<Pin>> pin_map_;
    
    // Properties
    std::map<std::string, std::string> properties_;
    
    // Characteristics
    TimingInfo timing_;
    PowerInfo power_;
    ElectricalInfo electrical_;
    TechNode tech_node_ = TechNode::um_7;
};

/**
 * @brief Component library for managing component definitions
 */
class ComponentLibrary {
public:
    ComponentLibrary(const std::string& name);
    
    // Component management
    void addComponent(std::shared_ptr<Component> component);
    std::shared_ptr<Component> getComponent(const std::string& name) const;
    std::shared_ptr<Component> createComponent(const std::string& type, const std::string& name) const;
    
    // Library information
    const std::string& getName() const { return name_; }
    std::vector<std::string> getComponentTypes() const;
    
    // Serialization
    void saveToFile(const std::string& filename) const;
    void loadFromFile(const std::string& filename);
    
private:
    std::string name_;
    std::unordered_map<std::string, std::shared_ptr<Component>> components_;
    std::unordered_map<std::string, std::function<std::shared_ptr<Component>(const std::string&)>> factories_;
};

/**
 * @brief Utility functions for component hierarchy
 */
namespace ComponentUtils {
    // Hierarchy traversal
    void traversePreOrder(std::shared_ptr<Component> root, 
                         std::function<void(std::shared_ptr<Component>)> visitor);
    void traversePostOrder(std::shared_ptr<Component> root, 
                          std::function<void(std::shared_ptr<Component>)> visitor);
    
    // Component analysis
    size_t countComponents(std::shared_ptr<Component> root, ComponentCategory category);
    double calculateTotalPower(std::shared_ptr<Component> root);
    double calculateCriticalPath(std::shared_ptr<Component> root);
    
    // Hierarchy optimization
    std::vector<std::shared_ptr<Component>> groupComponentsByFunction(
        const std::vector<std::shared_ptr<Component>>& components);
    
    std::shared_ptr<Component> createHierarchicalBlock(
        const std::vector<std::shared_ptr<Component>>& components,
        const std::string& block_name);
}

} // namespace components
} // namespace zlayout 