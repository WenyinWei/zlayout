/**
 * @file layout_optimizer.cpp
 * @brief Implementation of advanced EDA layout optimization algorithms
 */

#include <zlayout/optimization/layout_optimizer.hpp>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <iostream>

namespace zlayout {
namespace optimization {

// SimulatedAnnealingOptimizer implementation
void SimulatedAnnealingOptimizer::add_component(const Component& comp) {
    component_index[comp.name] = components.size();
    components.push_back(comp);
    
    // Initialize with random position if not set
    if (components.back().position.x == 0 && components.back().position.y == 0) {
        components.back().position = generate_random_position();
    }
}

void SimulatedAnnealingOptimizer::add_net(const Net& net) {
    nets.push_back(net);
}

CostResult SimulatedAnnealingOptimizer::optimize() {
    // Initialize best positions
    best_positions.clear();
    for (const auto& comp : components) {
        best_positions.push_back(comp.position);
    }
    
    // Initial cost evaluation
    current_cost = evaluate_cost();
    best_cost = current_cost;
    current_temperature = config.initial_temperature;
    
    std::cout << "Starting simulated annealing optimization..." << std::endl;
    std::cout << "Initial cost: " << current_cost.total_cost << std::endl;
    
    // Main optimization loop
    for (size_t iter = 0; iter < config.max_iterations; ++iter) {
        if (make_random_move()) {
            auto new_cost = evaluate_cost();
            double delta_cost = new_cost.total_cost - current_cost.total_cost;
            
            total_moves++;
            
            if (delta_cost < 0 || accept_probability(delta_cost)) {
                // Accept the move
                accept_move();
                current_cost = new_cost;
                accepted_moves++;
                
                if (new_cost.total_cost < best_cost.total_cost) {
                    best_cost = new_cost;
                    for (size_t i = 0; i < components.size(); ++i) {
                        best_positions[i] = components[i].position;
                    }
                    improved_moves++;
                }
            } else {
                // Reject the move
                reject_move();
            }
        }
        
        // Cool down
        current_temperature *= config.cooling_rate;
        
        // Progress reporting
        if (iter % 10000 == 0) {
            double acceptance_rate = static_cast<double>(accepted_moves) / total_moves;
            std::cout << "Iteration " << iter 
                     << ", Temperature: " << current_temperature
                     << ", Cost: " << current_cost.total_cost
                     << ", Acceptance: " << acceptance_rate * 100 << "%"
                     << std::endl;
        }
        
        // Termination check
        if (current_temperature < config.final_temperature) {
            std::cout << "Reached final temperature, stopping..." << std::endl;
            break;
        }
    }
    
    // Restore best solution
    for (size_t i = 0; i < components.size(); ++i) {
        components[i].position = best_positions[i];
    }
    
    std::cout << "Optimization completed!" << std::endl;
    std::cout << "Final cost: " << best_cost.total_cost << std::endl;
    std::cout << "Improvement: " << (current_cost.total_cost - best_cost.total_cost) / current_cost.total_cost * 100 << "%" << std::endl;
    
    return best_cost;
}

CostResult SimulatedAnnealingOptimizer::evaluate_cost() const {
    CostResult result = {};
    
    result.wirelength_cost = calculate_wirelength_cost();
    result.timing_cost = calculate_timing_cost();
    result.area_cost = calculate_area_cost();
    result.power_cost = calculate_power_cost();
    result.constraint_violations = calculate_constraint_violations();
    
    result.total_cost = config.wirelength_weight * result.wirelength_cost +
                       config.timing_weight * result.timing_cost +
                       config.area_weight * result.area_cost +
                       config.power_weight * result.power_cost +
                       1000.0 * result.constraint_violations;  // Heavy penalty for violations
    
    return result;
}

double SimulatedAnnealingOptimizer::calculate_wirelength_cost() const {
    double total_wirelength = 0.0;
    
    for (const auto& net : nets) {
        // Find driver position
        auto driver_it = component_index.find(net.driver_component);
        if (driver_it == component_index.end()) continue;
        
        geometry::Point driver_pos = components[driver_it->second].position;
        
        // Calculate total wirelength for this net
        double net_wirelength = 0.0;
        for (const auto& [sink_comp, sink_pin] : net.sinks) {
            auto sink_it = component_index.find(sink_comp);
            if (sink_it == component_index.end()) continue;
            
            geometry::Point sink_pos = components[sink_it->second].position;
            double distance = driver_pos.distance_to(sink_pos);
            net_wirelength += distance;
        }
        
        // Weight by net criticality
        total_wirelength += net_wirelength * net.weight * (1.0 + net.criticality);
    }
    
    return total_wirelength;
}

double SimulatedAnnealingOptimizer::calculate_timing_cost() const {
    double timing_cost = 0.0;
    
    for (const auto& net : nets) {
        if (net.criticality > 0.8) {  // Critical nets
            auto driver_it = component_index.find(net.driver_component);
            if (driver_it == component_index.end()) continue;
            
            geometry::Point driver_pos = components[driver_it->second].position;
            
            for (const auto& [sink_comp, sink_pin] : net.sinks) {
                auto sink_it = component_index.find(sink_comp);
                if (sink_it == component_index.end()) continue;
                
                geometry::Point sink_pos = components[sink_it->second].position;
                double distance = driver_pos.distance_to(sink_pos);
                
                // Timing cost increases quadratically with distance for critical nets
                timing_cost += distance * distance * net.criticality;
            }
        }
    }
    
    return timing_cost;
}

double SimulatedAnnealingOptimizer::calculate_area_cost() const {
    // Calculate bounding box of all components
    if (components.empty()) return 0.0;
    
    double min_x = components[0].position.x;
    double max_x = min_x + components[0].shape.width;
    double min_y = components[0].position.y;
    double max_y = min_y + components[0].shape.height;
    
    for (const auto& comp : components) {
        min_x = std::min(min_x, comp.position.x);
        max_x = std::max(max_x, comp.position.x + comp.shape.width);
        min_y = std::min(min_y, comp.position.y);
        max_y = std::max(max_y, comp.position.y + comp.shape.height);
    }
    
    double total_area = (max_x - min_x) * (max_y - min_y);
    double target_area = placement_area.area();
    
    // Penalty for exceeding placement area
    if (total_area > target_area) {
        return total_area - target_area;
    }
    
    return 0.0;
}

double SimulatedAnnealingOptimizer::calculate_power_cost() const {
    // Simple power cost based on component clustering
    // Components with higher power should be spread out to avoid hot spots
    
    double power_cost = 0.0;
    
    for (size_t i = 0; i < components.size(); ++i) {
        for (size_t j = i + 1; j < components.size(); ++j) {
            const auto& comp1 = components[i];
            const auto& comp2 = components[j];
            
            double distance = comp1.position.distance_to(comp2.position);
            double power_product = comp1.power_consumption * comp2.power_consumption;
            
            // Higher cost for high-power components that are close together
            if (power_product > 0.001 && distance < 10.0) {
                power_cost += power_product / (distance + 1.0);
            }
        }
    }
    
    return power_cost;
}

double SimulatedAnnealingOptimizer::calculate_constraint_violations() const {
    double violations = 0.0;
    
    // Check minimum spacing constraints
    for (size_t i = 0; i < components.size(); ++i) {
        for (size_t j = i + 1; j < components.size(); ++j) {
            const auto& comp1 = components[i];
            const auto& comp2 = components[j];
            
            // Calculate actual distance between component boundaries
            geometry::Rectangle rect1(comp1.position.x, comp1.position.y, 
                                    comp1.shape.width, comp1.shape.height);
            geometry::Rectangle rect2(comp2.position.x, comp2.position.y, 
                                    comp2.shape.width, comp2.shape.height);
            
            double distance = rect1.distance_to(rect2);
            
            if (distance < config.min_spacing) {
                violations += (config.min_spacing - distance);
            }
        }
    }
    
    // Check placement area constraints
    for (const auto& comp : components) {
        geometry::Rectangle comp_rect(comp.position.x, comp.position.y, 
                                    comp.shape.width, comp.shape.height);
        
        if (!placement_area.contains(comp_rect)) {
            violations += 100.0;  // Heavy penalty for out-of-bounds
        }
    }
    
    return violations;
}

bool SimulatedAnnealingOptimizer::make_random_move() {
    if (components.empty()) return false;
    
    // Select random component (skip fixed components)
    std::vector<size_t> movable_components;
    for (size_t i = 0; i < components.size(); ++i) {
        if (!components[i].is_fixed) {
            movable_components.push_back(i);
        }
    }
    
    if (movable_components.empty()) return false;
    
    std::uniform_int_distribution<size_t> comp_dist(0, movable_components.size() - 1);
    size_t selected_idx = movable_components[comp_dist(rng)];
    
    // Store old position for potential rollback
    old_position = components[selected_idx].position;
    selected_component = selected_idx;
    
    // Generate random move
    std::uniform_real_distribution<double> move_dist(-current_temperature, current_temperature);
    geometry::Point new_pos(
        components[selected_idx].position.x + move_dist(rng),
        components[selected_idx].position.y + move_dist(rng)
    );
    
    // Check if new position is valid
    if (is_position_valid(selected_idx, new_pos)) {
        components[selected_idx].position = new_pos;
        return true;
    }
    
    return false;
}

void SimulatedAnnealingOptimizer::accept_move() {
    // Move is already applied, nothing to do
}

void SimulatedAnnealingOptimizer::reject_move() {
    // Restore old position
    if (selected_component < components.size()) {
        components[selected_component].position = old_position;
    }
}

bool SimulatedAnnealingOptimizer::accept_probability(double delta_cost) const {
    if (current_temperature <= 0) return false;
    
    double probability = std::exp(-delta_cost / current_temperature);
    std::uniform_real_distribution<double> uniform(0.0, 1.0);
    
    return uniform(rng) < probability;
}

geometry::Point SimulatedAnnealingOptimizer::generate_random_position() const {
    std::uniform_real_distribution<double> x_dist(placement_area.x, 
                                                 placement_area.x + placement_area.width);
    std::uniform_real_distribution<double> y_dist(placement_area.y, 
                                                 placement_area.y + placement_area.height);
    
    return geometry::Point(x_dist(rng), y_dist(rng));
}

bool SimulatedAnnealingOptimizer::is_position_valid(size_t comp_idx, const geometry::Point& pos) const {
    const auto& comp = components[comp_idx];
    
    // Check if component stays within placement area
    if (pos.x < placement_area.x || 
        pos.y < placement_area.y ||
        pos.x + comp.shape.width > placement_area.x + placement_area.width ||
        pos.y + comp.shape.height > placement_area.y + placement_area.height) {
        return false;
    }
    
    return true;
}

SimulatedAnnealingOptimizer::Statistics SimulatedAnnealingOptimizer::get_statistics() const {
    Statistics stats;
    stats.total_moves = total_moves;
    stats.accepted_moves = accepted_moves;
    stats.improved_moves = improved_moves;
    stats.acceptance_rate = total_moves > 0 ? static_cast<double>(accepted_moves) / total_moves : 0.0;
    stats.improvement_rate = total_moves > 0 ? static_cast<double>(improved_moves) / total_moves : 0.0;
    stats.final_cost = best_cost;
    
    return stats;
}

// ForceDirectedPlacer implementation
bool ForceDirectedPlacer::optimize(size_t max_iterations) {
    std::cout << "Starting force-directed placement..." << std::endl;
    
    std::vector<geometry::Point> velocities(components.size(), geometry::Point(0, 0));
    
    for (size_t iter = 0; iter < max_iterations; ++iter) {
        bool converged = true;
        
        // Calculate forces for each component
        for (size_t i = 0; i < components.size(); ++i) {
            if (components[i]->is_fixed) continue;
            
            geometry::Point total_force(0, 0);
            
            // Net forces (attraction)
            auto net_force = calculate_net_force(*components[i]);
            total_force.x += net_force.x;
            total_force.y += net_force.y;
            
            // Repulsion forces
            auto repulsion_force = calculate_repulsion_force(*components[i]);
            total_force.x += repulsion_force.x;
            total_force.y += repulsion_force.y;
            
            // Boundary forces
            auto boundary_force = calculate_boundary_force(*components[i]);
            total_force.x += boundary_force.x;
            total_force.y += boundary_force.y;
            
            // Update velocity with damping
            velocities[i].x = velocities[i].x * damping_factor + total_force.x * time_step;
            velocities[i].y = velocities[i].y * damping_factor + total_force.y * time_step;
            
            // Update position
            components[i]->position.x += velocities[i].x * time_step;
            components[i]->position.y += velocities[i].y * time_step;
            
            // Check convergence
            if (std::abs(velocities[i].x) > 0.1 || std::abs(velocities[i].y) > 0.1) {
                converged = false;
            }
        }
        
        if (iter % 100 == 0) {
            std::cout << "Iteration " << iter << std::endl;
        }
        
        if (converged) {
            std::cout << "Force-directed placement converged at iteration " << iter << std::endl;
            return true;
        }
    }
    
    std::cout << "Force-directed placement completed (max iterations reached)" << std::endl;
    return false;
}

geometry::Point ForceDirectedPlacer::calculate_net_force(const Component& comp) const {
    geometry::Point force(0, 0);
    
    for (const auto& net : nets) {
        bool is_connected = false;
        
        // Check if this component is connected to the net
        if (net.driver_component == comp.name) {
            is_connected = true;
        } else {
            for (const auto& [sink_comp, sink_pin] : net.sinks) {
                if (sink_comp == comp.name) {
                    is_connected = true;
                    break;
                }
            }
        }
        
        if (!is_connected) continue;
        
        // Calculate center of mass of connected components
        geometry::Point center_of_mass(0, 0);
        int connected_count = 0;
        
        for (const auto& other_comp : components) {
            if (other_comp->name == comp.name) continue;
            
            bool is_other_connected = false;
            if (net.driver_component == other_comp->name) {
                is_other_connected = true;
            } else {
                for (const auto& [sink_comp, sink_pin] : net.sinks) {
                    if (sink_comp == other_comp->name) {
                        is_other_connected = true;
                        break;
                    }
                }
            }
            
            if (is_other_connected) {
                center_of_mass.x += other_comp->position.x;
                center_of_mass.y += other_comp->position.y;
                connected_count++;
            }
        }
        
        if (connected_count > 0) {
            center_of_mass.x /= connected_count;
            center_of_mass.y /= connected_count;
            
            // Apply spring force towards center of mass
            double dx = center_of_mass.x - comp.position.x;
            double dy = center_of_mass.y - comp.position.y;
            
            force.x += spring_constant * dx * net.weight;
            force.y += spring_constant * dy * net.weight;
        }
    }
    
    return force;
}

geometry::Point ForceDirectedPlacer::calculate_repulsion_force(const Component& comp) const {
    geometry::Point force(0, 0);
    
    for (const auto& other_comp : components) {
        if (other_comp->name == comp.name) continue;
        
        double dx = comp.position.x - other_comp->position.x;
        double dy = comp.position.y - other_comp->position.y;
        double distance_sq = dx * dx + dy * dy;
        
        if (distance_sq > 0) {
            double distance = std::sqrt(distance_sq);
            double repulsion = repulsion_constant / distance_sq;
            
            force.x += repulsion * dx / distance;
            force.y += repulsion * dy / distance;
        }
    }
    
    return force;
}

geometry::Point ForceDirectedPlacer::calculate_boundary_force(const Component& comp) const {
    geometry::Point force(0, 0);
    
    // Push components back into placement area
    double left_dist = comp.position.x - placement_area.x;
    double right_dist = (placement_area.x + placement_area.width) - (comp.position.x + comp.shape.width);
    double bottom_dist = comp.position.y - placement_area.y;
    double top_dist = (placement_area.y + placement_area.height) - (comp.position.y + comp.shape.height);
    
    double boundary_strength = 100.0;
    
    if (left_dist < 0) force.x += boundary_strength * (-left_dist);
    if (right_dist < 0) force.x -= boundary_strength * (-right_dist);
    if (bottom_dist < 0) force.y += boundary_strength * (-bottom_dist);
    if (top_dist < 0) force.y -= boundary_strength * (-top_dist);
    
    return force;
}

// OptimizerFactory implementation
std::unique_ptr<SimulatedAnnealingOptimizer> OptimizerFactory::create_sa_optimizer(
    const geometry::Rectangle& area,
    const OptimizationConfig& config) {
    return std::make_unique<SimulatedAnnealingOptimizer>(area, config);
}

std::unique_ptr<HierarchicalOptimizer> OptimizerFactory::create_hierarchical_optimizer(
    const geometry::Rectangle& area,
    const OptimizationConfig& config) {
    return std::make_unique<HierarchicalOptimizer>(area, config);
}

std::unique_ptr<ForceDirectedPlacer> OptimizerFactory::create_force_directed_placer(
    const geometry::Rectangle& area) {
    return std::make_unique<ForceDirectedPlacer>(area);
}

OptimizerFactory::AlgorithmType OptimizerFactory::recommend_algorithm(
    size_t component_count,
    size_t net_count,
    bool timing_critical) {
    
    if (component_count > 100000) {
        return AlgorithmType::HIERARCHICAL;
    } else if (timing_critical) {
        return AlgorithmType::TIMING_DRIVEN;
    } else if (component_count > 1000) {
        return AlgorithmType::SIMULATED_ANNEALING;
    } else {
        return AlgorithmType::FORCE_DIRECTED;
    }
}

} // namespace optimization
} // namespace zlayout 