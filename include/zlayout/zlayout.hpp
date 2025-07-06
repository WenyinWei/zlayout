/**
 * @file zlayout.hpp
 * @brief ZLayout - Advanced Electronic Design Automation Layout Library
 * @author ZLayout Team
 * @version 1.0.0
 * 
 * High-performance C++ library for EDA layout processing with quadtree spatial indexing,
 * efficient polygon processing, and advanced geometric analysis for electronic circuit
 * layout optimization.
 */

#pragma once

#include <zlayout/geometry/point.hpp>
#include <zlayout/geometry/rectangle.hpp>
#include <zlayout/geometry/polygon.hpp>
#include <zlayout/spatial/quadtree.hpp>
#include <zlayout/spatial/spatial_index.hpp>
#include <zlayout/analysis/polygon_analyzer.hpp>
#include <zlayout/analysis/geometry_processor.hpp>

/**
 * @namespace zlayout
 * @brief Main namespace for ZLayout library
 */
namespace zlayout {

    /**
     * @brief Library version information
     */
    struct Version {
        static constexpr int MAJOR = 1;
        static constexpr int MINOR = 0;
        static constexpr int PATCH = 0;
        static constexpr const char* STRING = "1.0.0";
    };

    /**
     * @brief Get library version string
     * @return Version string
     */
    const char* get_version();

    /**
     * @brief Initialize ZLayout library
     * @param enable_openmp Enable OpenMP parallelization
     * @return true if successful
     */
    bool initialize(bool enable_openmp = true);

    /**
     * @brief Cleanup ZLayout library resources
     */
    void cleanup();

} // namespace zlayout