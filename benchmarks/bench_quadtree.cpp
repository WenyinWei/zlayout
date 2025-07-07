/**
 * @file bench_quadtree.cpp
 * @brief Benchmark tests for QuadTree performance
 */

#include <benchmark/benchmark.h>
#include <zlayout/spatial/quadtree.hpp>
#include <zlayout/geometry/rectangle.hpp>
#include <random>

using namespace zlayout::geometry;
using namespace zlayout::spatial;

// Benchmark QuadTree insertion performance
static void BM_QuadTreeInsert(benchmark::State& state) {
    Rectangle world_bounds(0, 0, 1000, 1000);
    auto qt = QuadTree<Rectangle>(
        world_bounds,
        [](const Rectangle& rect) -> Rectangle { return rect; },
        10, 6
    );
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 900.0);
    
    for (auto _ : state) {
        Rectangle rect(dis(gen), dis(gen), 50, 50);
        benchmark::DoNotOptimize(qt.insert(rect));
    }
    
    state.SetComplexityN(state.iterations());
}
BENCHMARK(BM_QuadTreeInsert)->Range(8, 8<<10)->Complexity();

// Benchmark QuadTree query performance
static void BM_QuadTreeQuery(benchmark::State& state) {
    Rectangle world_bounds(0, 0, 1000, 1000);
    auto qt = QuadTree<Rectangle>(
        world_bounds,
        [](const Rectangle& rect) -> Rectangle { return rect; },
        10, 6
    );
    
    // Pre-populate quadtree
    std::random_device rd;
    std::mt19937 gen(42); // Fixed seed for consistency
    std::uniform_real_distribution<> dis(0.0, 900.0);
    
    for (int i = 0; i < state.range(0); ++i) {
        Rectangle rect(dis(gen), dis(gen), 50, 50);
        qt.insert(rect);
    }
    
    gen.seed(42); // Reset for consistent queries
    for (auto _ : state) {
        Rectangle query_rect(dis(gen), dis(gen), 100, 100);
        auto results = qt.query_range(query_rect);
        benchmark::DoNotOptimize(results);
    }
    
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_QuadTreeQuery)->Range(8, 8<<10)->Complexity();

BENCHMARK_MAIN(); 