/**
 * @file passive_components.hpp
 * @brief Passive Electronic Components Implementation
 * 
 * This file contains implementations of basic passive components
 * like resistors, capacitors, and inductors.
 */

#pragma once

#include <zlayout/components/component.hpp>
#include <memory>
#include <string>
#include <vector>

namespace zlayout {
namespace components {

/**
 * @brief Resistor component
 */
class Resistor : public Component {
public:
    Resistor(const std::string& name, double resistance, double tolerance = 5.0);
    
    // Electrical properties
    double getResistance() const { return resistance_; }
    void setResistance(double resistance);
    
    double getTolerance() const { return tolerance_; }
    void setTolerance(double tolerance) { tolerance_ = tolerance; }
    
    double getPowerRating() const { return power_rating_; }
    void setPowerRating(double power) { power_rating_ = power; }
    
    // Temperature coefficient
    double getTempCoeff() const { return temp_coeff_; }
    void setTempCoeff(double coeff) { temp_coeff_ = coeff; }
    
    // Standard values
    static std::vector<double> getStandardValues(); // E12, E24, E48, E96 series
    static double getNearestStandardValue(double value, int series = 24);
    
    // Parasitic effects
    double getParasiticCapacitance() const { return parasitic_capacitance_; }
    double getParasiticInductance() const { return parasitic_inductance_; }
    
    // Simulation
    void simulate(double time_step) override;
    std::string getDescription() const override;
    
private:
    double resistance_;        // Ω
    double tolerance_;         // %
    double power_rating_;      // W
    double temp_coeff_;        // ppm/°C
    double parasitic_capacitance_; // pF
    double parasitic_inductance_;  // nH
};

/**
 * @brief Capacitor component
 */
class Capacitor : public Component {
public:
    enum class DielectricType {
        Ceramic,      // C0G, X7R, Y5V
        Electrolytic, // Aluminum, Tantalum
        Film,         // Polyester, Polypropylene
        Mica,         // Silver mica
        Supercap      // Super capacitor
    };
    
    Capacitor(const std::string& name, double capacitance, DielectricType dielectric);
    
    // Electrical properties
    double getCapacitance() const { return capacitance_; }
    void setCapacitance(double capacitance);
    
    double getTolerance() const { return tolerance_; }
    void setTolerance(double tolerance) { tolerance_ = tolerance; }
    
    double getVoltageRating() const { return voltage_rating_; }
    void setVoltageRating(double voltage) { voltage_rating_ = voltage; }
    
    DielectricType getDielectric() const { return dielectric_; }
    void setDielectric(DielectricType type) { dielectric_ = type; }
    
    // Parasitic effects
    double getESR() const { return esr_; }        // Equivalent Series Resistance
    void setESR(double esr) { esr_ = esr; }
    
    double getESL() const { return esl_; }        // Equivalent Series Inductance
    void setESL(double esl) { esl_ = esl; }
    
    double getLeakageCurrent() const { return leakage_current_; }
    void setLeakageCurrent(double current) { leakage_current_ = current; }
    
    // Temperature characteristics
    double getTempCoeff() const { return temp_coeff_; }
    void setTempCoeff(double coeff) { temp_coeff_ = coeff; }
    
    // Standard values
    static std::vector<double> getStandardValues();
    static double getNearestStandardValue(double value);
    
    // Simulation
    void simulate(double time_step) override;
    std::string getDescription() const override;
    
private:
    double capacitance_;       // F
    double tolerance_;         // %
    double voltage_rating_;    // V
    DielectricType dielectric_;
    double esr_;              // Ω
    double esl_;              // H
    double leakage_current_;  // A
    double temp_coeff_;       // ppm/°C
    
    // State variables for simulation
    double charge_;           // C
    double voltage_;          // V
};

/**
 * @brief Inductor component
 */
class Inductor : public Component {
public:
    enum class CoreType {
        Air,          // Air core
        Ferrite,      // Ferrite core
        Iron,         // Iron core
        Laminated,    // Laminated iron
        Powdered      // Powdered iron
    };
    
    Inductor(const std::string& name, double inductance, CoreType core);
    
    // Electrical properties
    double getInductance() const { return inductance_; }
    void setInductance(double inductance);
    
    double getTolerance() const { return tolerance_; }
    void setTolerance(double tolerance) { tolerance_ = tolerance; }
    
    double getCurrentRating() const { return current_rating_; }
    void setCurrentRating(double current) { current_rating_ = current; }
    
    CoreType getCoreType() const { return core_type_; }
    void setCoreType(CoreType type) { core_type_ = type; }
    
    // Parasitic effects
    double getDCR() const { return dcr_; }        // DC Resistance
    void setDCR(double dcr) { dcr_ = dcr; }
    
    double getSRF() const { return srf_; }        // Self Resonant Frequency
    void setSRF(double srf) { srf_ = srf; }
    
    double getParasiticCapacitance() const { return parasitic_capacitance_; }
    void setParasiticCapacitance(double cap) { parasitic_capacitance_ = cap; }
    
    // Magnetic properties
    double getSaturationCurrent() const { return saturation_current_; }
    void setSaturationCurrent(double current) { saturation_current_ = current; }
    
    double getQualityFactor() const { return quality_factor_; }
    void setQualityFactor(double q) { quality_factor_ = q; }
    
    // Standard values
    static std::vector<double> getStandardValues();
    static double getNearestStandardValue(double value);
    
    // Simulation
    void simulate(double time_step) override;
    std::string getDescription() const override;
    
private:
    double inductance_;            // H
    double tolerance_;             // %
    double current_rating_;        // A
    CoreType core_type_;
    double dcr_;                   // Ω
    double srf_;                   // Hz
    double parasitic_capacitance_; // F
    double saturation_current_;    // A
    double quality_factor_;        // Q
    
    // State variables for simulation
    double current_;               // A
    double flux_;                  // Wb
};

/**
 * @brief Crystal/Oscillator component
 */
class Crystal : public Component {
public:
    enum class CrystalType {
        Quartz,       // Quartz crystal
        SAW,          // Surface Acoustic Wave
        MEMS,         // Microelectromechanical systems
        Ceramic       // Ceramic resonator
    };
    
    Crystal(const std::string& name, double frequency, CrystalType type);
    
    // Frequency characteristics
    double getFrequency() const { return frequency_; }
    void setFrequency(double freq) { frequency_ = freq; }
    
    double getFrequencyTolerance() const { return frequency_tolerance_; }
    void setFrequencyTolerance(double tolerance) { frequency_tolerance_ = tolerance; }
    
    double getFrequencyStability() const { return frequency_stability_; }
    void setFrequencyStability(double stability) { frequency_stability_ = stability; }
    
    CrystalType getCrystalType() const { return crystal_type_; }
    void setCrystalType(CrystalType type) { crystal_type_ = type; }
    
    // Electrical model
    double getMotionalCapacitance() const { return c1_; }
    void setMotionalCapacitance(double c1) { c1_ = c1; }
    
    double getMotionalInductance() const { return l1_; }
    void setMotionalInductance(double l1) { l1_ = l1; }
    
    double getMotionalResistance() const { return r1_; }
    void setMotionalResistance(double r1) { r1_ = r1; }
    
    double getShuntCapacitance() const { return c0_; }
    void setShuntCapacitance(double c0) { c0_ = c0; }
    
    // Drive level
    double getDriveLevel() const { return drive_level_; }
    void setDriveLevel(double level) { drive_level_ = level; }
    
    // Simulation
    void simulate(double time_step) override;
    std::string getDescription() const override;
    
private:
    double frequency_;            // Hz
    double frequency_tolerance_;  // ppm
    double frequency_stability_;  // ppm
    CrystalType crystal_type_;
    
    // Equivalent circuit model
    double c1_;                   // F (motional capacitance)
    double l1_;                   // H (motional inductance)
    double r1_;                   // Ω (motional resistance)
    double c0_;                   // F (shunt capacitance)
    double drive_level_;          // W
};

/**
 * @brief Transformer component
 */
class Transformer : public Component {
public:
    enum class TransformerType {
        Power,        // Power transformer
        Signal,       // Signal transformer
        Isolation,    // Isolation transformer
        Current,      // Current transformer
        Voltage       // Voltage transformer
    };
    
    Transformer(const std::string& name, double turns_ratio, TransformerType type);
    
    // Turns ratio
    double getTurnsRatio() const { return turns_ratio_; }
    void setTurnsRatio(double ratio) { turns_ratio_ = ratio; }
    
    TransformerType getTransformerType() const { return transformer_type_; }
    void setTransformerType(TransformerType type) { transformer_type_ = type; }
    
    // Electrical parameters
    double getPrimaryInductance() const { return primary_inductance_; }
    void setPrimaryInductance(double l) { primary_inductance_ = l; }
    
    double getSecondaryInductance() const { return secondary_inductance_; }
    void setSecondaryInductance(double l) { secondary_inductance_ = l; }
    
    double getMutualInductance() const { return mutual_inductance_; }
    void setMutualInductance(double m) { mutual_inductance_ = m; }
    
    double getCouplingCoefficient() const { return coupling_coefficient_; }
    void setCouplingCoefficient(double k) { coupling_coefficient_ = k; }
    
    // Resistance
    double getPrimaryResistance() const { return primary_resistance_; }
    void setPrimaryResistance(double r) { primary_resistance_ = r; }
    
    double getSecondaryResistance() const { return secondary_resistance_; }
    void setSecondaryResistance(double r) { secondary_resistance_ = r; }
    
    // Power rating
    double getPowerRating() const { return power_rating_; }
    void setPowerRating(double power) { power_rating_ = power; }
    
    // Simulation
    void simulate(double time_step) override;
    std::string getDescription() const override;
    
private:
    double turns_ratio_;          // N2/N1
    TransformerType transformer_type_;
    double primary_inductance_;   // H
    double secondary_inductance_; // H
    double mutual_inductance_;    // H
    double coupling_coefficient_; // k
    double primary_resistance_;   // Ω
    double secondary_resistance_; // Ω
    double power_rating_;         // W
};

/**
 * @brief Passive component factory
 */
class PassiveComponentFactory {
public:
    // Standard component creation
    static std::shared_ptr<Resistor> createResistor(
        const std::string& name, double resistance, double tolerance = 5.0);
    
    static std::shared_ptr<Capacitor> createCapacitor(
        const std::string& name, double capacitance, 
        Capacitor::DielectricType dielectric = Capacitor::DielectricType::Ceramic);
    
    static std::shared_ptr<Inductor> createInductor(
        const std::string& name, double inductance, 
        Inductor::CoreType core = Inductor::CoreType::Air);
    
    static std::shared_ptr<Crystal> createCrystal(
        const std::string& name, double frequency, 
        Crystal::CrystalType type = Crystal::CrystalType::Quartz);
    
    static std::shared_ptr<Transformer> createTransformer(
        const std::string& name, double turns_ratio, 
        Transformer::TransformerType type = Transformer::TransformerType::Signal);
    
    // Component from specifications
    static std::shared_ptr<Component> createFromSpecs(
        const std::string& component_type, const std::string& name,
        const std::map<std::string, std::string>& specs);
    
    // Standard library components
    static void registerStandardComponents(ComponentLibrary& library);
};

} // namespace components
} // namespace zlayout 