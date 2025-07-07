/**
 * @file zlayout.cpp
 * @brief Main ZLayout library implementation
 */

#include <zlayout/zlayout.hpp>
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <algorithm>
#include <string>
#include <cstdio>
#include <cstdlib>

#ifdef ZLAYOUT_OPENMP
#include <omp.h>
#endif

namespace zlayout {

namespace {
    bool g_initialized = false;
    bool g_openmp_enabled = false;
}

const char* get_version() {
    return Version::STRING;
}

bool initialize(bool enable_openmp) {
    if (g_initialized) {
        std::cerr << "Warning: ZLayout already initialized" << std::endl;
        return true;
    }
    
    try {
        // Initialize OpenMP if requested and available
        g_openmp_enabled = false;
        if (enable_openmp) {
#ifdef ZLAYOUT_OPENMP
            // Set OpenMP number of threads
            int num_threads = omp_get_max_threads();
            std::cout << "ZLayout: OpenMP enabled with " << num_threads << " threads" << std::endl;
            g_openmp_enabled = true;
#else
            std::cout << "ZLayout: OpenMP not available in this build" << std::endl;
#endif
        }
        
        // Perform any other initialization
        std::cout << "ZLayout v" << get_version() << " initialized successfully" << std::endl;
        std::cout << "Features enabled:" << std::endl;
        std::cout << "  - High-precision geometry (tolerance: " << geometry::Point::TOLERANCE << ")" << std::endl;
        std::cout << "  - Quadtree spatial indexing" << std::endl;
        std::cout << "  - Sharp angle detection" << std::endl;
        std::cout << "  - Edge intersection detection" << std::endl;
        std::cout << "  - Narrow distance analysis" << std::endl;
        
        if (g_openmp_enabled) {
            std::cout << "  - OpenMP parallel processing" << std::endl;
        }
        
        g_initialized = true;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error initializing ZLayout: " << e.what() << std::endl;
        return false;
    }
}

void cleanup() {
    if (!g_initialized) {
        return;
    }
    
    // Perform cleanup operations
    g_initialized = false;
    g_openmp_enabled = false;
    
    std::cout << "ZLayout cleanup completed" << std::endl;
}

bool is_initialized() {
    return g_initialized;
}

bool is_openmp_enabled() {
    return g_openmp_enabled;
}

// Performance utilities

struct PerformanceTimer {
    std::chrono::high_resolution_clock::time_point start_time;
    
    PerformanceTimer() {
        start_time = std::chrono::high_resolution_clock::now();
    }
    
    double elapsed_ms() const {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return duration.count() / 1000.0;
    }
    
    double elapsed_us() const {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return static_cast<double>(duration.count());
    }
};

// Library information functions

struct SystemInfo {
    std::string compiler;
    std::string build_type;
    std::string platform;
    bool openmp_support;
    size_t max_threads;
};

// Helper function for MSVC 2012 compatibility
namespace {
    std::string int_to_string(int value) {
#if defined(_MSC_VER) && _MSC_VER < 1800  // MSVC 2012 and earlier
        char buffer[32];
        sprintf(buffer, "%d", value);
        return std::string(buffer);
#else
        return std::to_string(value);
#endif
    }
}

SystemInfo get_system_info() {
    SystemInfo info;
    
    // Compiler information
#ifdef __GNUC__
    info.compiler = "GCC " + int_to_string(__GNUC__) + "." + int_to_string(__GNUC_MINOR__);
#elif defined(__clang__)
    info.compiler = "Clang " + int_to_string(__clang_major__) + "." + int_to_string(__clang_minor__);
#elif defined(_MSC_VER)
    info.compiler = "MSVC " + int_to_string(_MSC_VER);
#else
    info.compiler = "Unknown compiler";
#endif
    
    // Build type
#ifdef DEBUG
    info.build_type = "Debug";
#else
    info.build_type = "Release";
#endif
    
    // Platform
#ifdef _WIN32
    info.platform = "Windows";
#elif defined(__linux__)
    info.platform = "Linux";
#elif defined(__APPLE__)
    info.platform = "macOS";
#else
    info.platform = "Unknown";
#endif
    
    // OpenMP support
#ifdef ZLAYOUT_OPENMP
    info.openmp_support = true;
    info.max_threads = omp_get_max_threads();
#else
    info.openmp_support = false;
    info.max_threads = 1;
#endif
    
    return info;
}

void print_system_info() {
    auto info = get_system_info();
    
    std::cout << "\n=== ZLayout System Information ===" << std::endl;
    std::cout << "Version: " << get_version() << std::endl;
    std::cout << "Compiler: " << info.compiler << std::endl;
    std::cout << "Build Type: " << info.build_type << std::endl;
    std::cout << "Platform: " << info.platform << std::endl;
    std::cout << "OpenMP Support: " << (info.openmp_support ? "Yes" : "No") << std::endl;
    std::cout << "Max Threads: " << info.max_threads << std::endl;
    std::cout << "Geometry Tolerance: " << geometry::Point::TOLERANCE << std::endl;
    std::cout << "=================================" << std::endl;
}

// Error handling utilities

class ZLayoutException : public std::runtime_error {
public:
    explicit ZLayoutException(const std::string& message)
        : std::runtime_error("ZLayout Error: " + message) {}
};

void assert_initialized() {
    if (!g_initialized) {
        throw ZLayoutException("Library not initialized. Call zlayout::initialize() first.");
    }
}

// Memory and performance monitoring

struct MemoryInfo {
    size_t allocated_bytes;
    size_t peak_bytes;
    size_t allocation_count;
};

// Simple memory tracker (for debug builds)
#ifdef DEBUG
namespace {
    MemoryInfo g_memory_info = {0, 0, 0};
}

void* tracked_malloc(size_t size) {
    void* ptr = std::malloc(size);
    if (ptr) {
        g_memory_info.allocated_bytes += size;
        g_memory_info.peak_bytes = std::max(g_memory_info.peak_bytes, g_memory_info.allocated_bytes);
        g_memory_info.allocation_count++;
    }
    return ptr;
}

void tracked_free(void* ptr, size_t size) {
    if (ptr) {
        std::free(ptr);
        g_memory_info.allocated_bytes -= size;
    }
}

MemoryInfo get_memory_info() {
    return g_memory_info;
}
#else
MemoryInfo get_memory_info() {
    return {0, 0, 0}; // Not tracked in release builds
}
#endif

} // namespace zlayout