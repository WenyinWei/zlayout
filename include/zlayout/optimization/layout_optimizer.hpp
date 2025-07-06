/**
 * @file layout_optimizer.hpp
 * @brief Advanced EDA layout optimization algorithms
 * 
 * This module focuses on the real challenges of EDA layout optimization:
 * - Component placement with complex connectivity constraints
 * - Multi-objective optimization (area, timing, power, thermal)
 * - Sophisticated algorithms like simulated annealing, force-directed placement
 * - Hierarchical optimization for billion-scale layouts
 */

#pragma once

#include <zlayout/geometry/point.hpp>
#include <zlayout/geometry/rectangle.hpp>
#include <zlayout/spatial/advanced_spatial.hpp>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <random>

namespace zlayout {
namespace optimization {

/**
 * @brief Circuit component with connectivity information
 */
struct Component {
    std::string name;
    geometry::Rectangle shape;
    geometry::Point position;
    std::vector<std::string> input_pins;
    std::vector<std::string> output_pins;
    
    // Physical properties
    double power_consumption;
    double thermal_coefficient;
    bool is_fixed;  // Cannot be moved (e.g., I/O pads)
    
    Component(const std::string& name, const geometry::Rectangle& shape)
        : name(name), shape(shape), power_consumption(0.0), 
          thermal_coefficient(1.0), is_fixed(false) {}
};

/**
 * @brief Net (electrical connection) between components
 */
struct Net {
    std::string name;
    std::string driver_component;  // Component that drives this net
    std::string driver_pin;        // Pin that drives this net
    std::vector<std::pair<std::string, std::string>> sinks;  // (component, pin) pairs
    
    double criticality;  // Timing criticality (0.0 = non-critical, 1.0 = critical path)
    double weight;       // Optimization weight
    
    Net(const std::string& name) : name(name), criticality(0.0), weight(1.0) {}
};

/**
 * @brief Layout optimization objectives and constraints
 */
struct OptimizationConfig {
    // Objectives (weights should sum to 1.0)
    double area_weight = 0.3;
    double wirelength_weight = 0.4;
    double timing_weight = 0.2;
    double power_weight = 0.1;
    
    // Constraints
    double max_utilization = 0.8;  // Maximum area utilization
    double min_spacing = 0.15;     // Minimum spacing between components
    double max_aspect_ratio = 2.0; // Maximum chip aspect ratio
    
    // Algorithm parameters
    double initial_temperature = 1000.0;
    double cooling_rate = 0.95;
    double final_temperature = 0.1;
    size_t max_iterations = 100000;
    
    // Multi-level optimization
    bool enable_hierarchical = true;
    size_t max_components_per_block = 10000;
};

/**
 * @brief Cost evaluation result
 */
struct CostResult {
    double total_cost;
    double area_cost;
    double wirelength_cost;
    double timing_cost;
    double power_cost;
    double constraint_violations;
    
    bool is_feasible() const {
        return constraint_violations < 1e-6;
    }
};

/**
 * @brief Force-directed placement algorithm
 * 
 * Models components as particles connected by springs (nets)
 * and uses physical simulation to find optimal positions.
 */
class ForceDirectedPlacer {
private:
    std::vector<Component*> components;
    std::vector<Net> nets;
    geometry::Rectangle placement_area;
    
    // Force calculation parameters
    double spring_constant = 1.0;
    double repulsion_constant = 1000.0;
    double damping_factor = 0.9;
    double time_step = 0.01;
    
public:
    ForceDirectedPlacer(const geometry::Rectangle& area) : placement_area(area) {}
    
    void add_component(Component* comp) { components.push_back(comp); }
    void add_net(const Net& net) { nets.push_back(net); }
    
    /**
     * @brief Run force-directed placement
     */
    bool optimize(size_t max_iterations = 1000);
    
private:
    geometry::Point calculate_net_force(const Component& comp) const;
    geometry::Point calculate_repulsion_force(const Component& comp) const;
    geometry::Point calculate_boundary_force(const Component& comp) const;
    bool is_converged() const;
};

/**
 * @brief Simulated Annealing optimizer
 * 
 * The gold standard for EDA placement optimization.
 * Handles the complex, highly-coupled nature of layout optimization.
 */
class SimulatedAnnealingOptimizer {
private:
    std::vector<Component> components;
    std::vector<Net> nets;
    std::unordered_map<std::string, size_t> component_index;
    
    geometry::Rectangle placement_area;
    OptimizationConfig config;
    
    // State management
    std::mt19937 rng;
    double current_temperature;
    CostResult current_cost;
    CostResult best_cost;
    std::vector<geometry::Point> best_positions;
    
    // Statistics
    size_t total_moves = 0;
    size_t accepted_moves = 0;
    size_t improved_moves = 0;
    
public:
    SimulatedAnnealingOptimizer(const geometry::Rectangle& area, 
                               const OptimizationConfig& config = OptimizationConfig{})
        : placement_area(area), config(config), rng(std::random_device{}()) {}
    
    void add_component(const Component& comp);
    void add_net(const Net& net);
    
    /**
     * @brief Run simulated annealing optimization
     */
    CostResult optimize();
    
    /**
     * @brief Get optimized component positions
     */
    std::vector<geometry::Point> get_positions() const { return best_positions; }
    
    /**
     * @brief Get optimization statistics
     */
    struct Statistics {
        size_t total_moves;
        size_t accepted_moves;
        size_t improved_moves;
        double acceptance_rate;
        double improvement_rate;
        CostResult final_cost;
    };
    
    Statistics get_statistics() const;
    
private:
    CostResult evaluate_cost() const;
    double calculate_wirelength_cost() const;
    double calculate_timing_cost() const;
    double calculate_area_cost() const;
    double calculate_power_cost() const;
    double calculate_constraint_violations() const;
    
    bool make_random_move();
    void accept_move();
    void reject_move();
    bool accept_probability(double delta_cost) const;
    
    geometry::Point generate_random_position() const;
    bool is_position_valid(size_t comp_idx, const geometry::Point& pos) const;
};

/**
 * @brief Hierarchical layout optimizer for billion-scale designs
 * 
 * Decomposes the problem into manageable IP blocks,
 * optimizes each block separately, then optimizes block placement.
 */
class HierarchicalOptimizer {
private:
    struct IPBlock {
        std::string name;
        std::vector<Component> components;
        std::vector<Net> internal_nets;
        geometry::Rectangle boundary;
        geometry::Point position;
        
        // Block-level optimization result
        CostResult optimization_result;
        bool is_optimized = false;
    };
    
    std::vector<IPBlock> ip_blocks;
    std::vector<Net> inter_block_nets;
    geometry::Rectangle chip_area;
    OptimizationConfig config;
    
public:
    HierarchicalOptimizer(const geometry::Rectangle& chip_area,
                         const OptimizationConfig& config = OptimizationConfig{})
        : chip_area(chip_area), config(config) {}
    
    /**
     * @brief Create an IP block
     */
    void create_ip_block(const std::string& name, const geometry::Rectangle& boundary);
    
    /**
     * @brief Add component to an IP block
     */
    void add_component_to_block(const std::string& block_name, const Component& comp);
    
    /**
     * @brief Add net (automatically determines if inter-block or intra-block)
     */
    void add_net(const Net& net);
    
    /**
     * @brief Run hierarchical optimization
     * 1. Optimize each IP block independently
     * 2. Optimize IP block placement
     * 3. Refine with global optimization
     */
    CostResult optimize();
    
    /**
     * @brief Get final layout result
     */
    std::vector<std::pair<Component, geometry::Point>> get_final_layout() const;
    
private:
    void optimize_ip_block(IPBlock& block);
    void optimize_block_placement();
    void global_refinement();
    
    std::vector<Net> extract_inter_block_nets() const;
    bool components_in_same_block(const std::string& comp1, const std::string& comp2) const;
};

/**
 * @brief Analytical placement using quadratic optimization
 * 
 * Fast initial placement algorithm that models nets as quadratic forces.
 * Good for generating initial solutions for simulated annealing.
 */
class AnalyticalPlacer {
private:
    std::vector<Component> components;
    std::vector<Net> nets;
    geometry::Rectangle placement_area;
    
public:
    AnalyticalPlacer(const geometry::Rectangle& area) : placement_area(area) {}
    
    void add_component(const Component& comp) { components.push_back(comp); }
    void add_net(const Net& net) { nets.push_back(net); }
    
    /**
     * @brief Generate initial placement using quadratic optimization
     */
    std::vector<geometry::Point> generate_initial_placement();
    
private:
    void solve_quadratic_system();
    void legalize_positions(std::vector<geometry::Point>& positions);
};

/**
 * @brief Timing-driven optimization
 * 
 * Specialized optimizer that prioritizes critical path timing.
 */
class TimingDrivenOptimizer {
private:
    std::vector<Component> components;
    std::vector<Net> nets;
    std::unordered_map<std::string, double> component_delays;
    std::unordered_map<std::string, double> net_delays;
    
public:
    void add_component(const Component& comp, double delay);
    void add_net(const Net& net);
    
    /**
     * @brief Calculate critical path and update net criticalities
     */
    void update_timing_criticality();
    
    /**
     * @brief Timing-driven placement optimization
     */
    CostResult optimize_for_timing();
    
private:
    double calculate_path_delay(const std::vector<std::string>& path) const;
    std::vector<std::string> find_critical_path() const;
    void update_net_weights_by_criticality();
};

/**
 * @brief Layout optimization factory
 */
class OptimizerFactory {
public:
    enum class AlgorithmType {
        SIMULATED_ANNEALING,
        FORCE_DIRECTED,
        HIERARCHICAL,
        ANALYTICAL,
        TIMING_DRIVEN
    };
    
    static std::unique_ptr<SimulatedAnnealingOptimizer> create_sa_optimizer(
        const geometry::Rectangle& area,
        const OptimizationConfig& config = OptimizationConfig{});
    
    static std::unique_ptr<HierarchicalOptimizer> create_hierarchical_optimizer(
        const geometry::Rectangle& area,
        const OptimizationConfig& config = OptimizationConfig{});
    
    static std::unique_ptr<ForceDirectedPlacer> create_force_directed_placer(
        const geometry::Rectangle& area);
    
    /**
     * @brief Choose optimal algorithm based on problem characteristics
     */
    static AlgorithmType recommend_algorithm(size_t component_count, 
                                           size_t net_count,
                                           bool timing_critical = false);
};

} // namespace optimization
} // namespace zlayout 