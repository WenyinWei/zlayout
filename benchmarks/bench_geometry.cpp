/**
 * @file bench_geometry.cpp
 * @brief Benchmark tests for geometry operations performance
 */

#define _USE_MATH_DEFINES
#include <benchmark/benchmark.h>
#include <zlayout/geometry/polygon.hpp>
#include <zlayout/geometry/point.hpp>
#include <zlayout/geometry/rectangle.hpp>
#include <random>
#include <vector>
#include <cmath>

// Fallback for M_PI if not defined (MSVC 2012 compatibility)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace zlayout::geometry;

// Benchmark polygon area calculation
static void BM_PolygonArea(benchmark::State& state) {
    std::vector<Point> vertices;
    for (int i = 0; i < state.range(0); ++i) {
        double angle = 2.0 * M_PI * i / state.range(0);
        vertices.emplace_back(100 * cos(angle), 100 * sin(angle));
    }
    
    Polygon poly(vertices);
    
    for (auto _ : state) {
        double area = poly.area();
        benchmark::DoNotOptimize(area);
    }
    
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_PolygonArea)->Range(8, 8<<8)->Complexity();

// Benchmark point-in-polygon testing
static void BM_PointInPolygon(benchmark::State& state) {
    std::vector<Point> vertices;
    for (int i = 0; i < 100; ++i) {
        double angle = 2.0 * M_PI * i / 100;
        vertices.emplace_back(100 * cos(angle), 100 * sin(angle));
    }
    
    Polygon poly(vertices);
    
    std::random_device rd;
    std::mt19937 gen(42);
    std::uniform_real_distribution<> dis(-150.0, 150.0);
    
    for (auto _ : state) {
        Point test_point(dis(gen), dis(gen));
        bool contains = poly.contains_point(test_point);
        benchmark::DoNotOptimize(contains);
    }
}
BENCHMARK(BM_PointInPolygon);

// Benchmark sharp angle detection
static void BM_SharpAngleDetection(benchmark::State& state) {
    std::vector<Point> vertices;
    std::random_device rd;
    std::mt19937 gen(42);
    std::uniform_real_distribution<> dis(0.0, 1000.0);
    
    for (int i = 0; i < state.range(0); ++i) {
        vertices.emplace_back(dis(gen), dis(gen));
    }
    
    Polygon poly(vertices);
    
    for (auto _ : state) {
        auto sharp_angles = poly.get_sharp_angles(30.0);
        benchmark::DoNotOptimize(sharp_angles);
    }
    
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_SharpAngleDetection)->Range(8, 8<<6)->Complexity();

// Benchmark polygon distance calculation
static void BM_PolygonDistance(benchmark::State& state) {
    std::vector<Point> vertices1 = {
        Point(0, 0), Point(100, 0), Point(100, 100), Point(0, 100)
    };
    std::vector<Point> vertices2 = {
        Point(150, 50), Point(250, 50), Point(250, 150), Point(150, 150)
    };
    
    Polygon poly1(vertices1);
    Polygon poly2(vertices2);
    
    for (auto _ : state) {
        double distance = poly1.distance_to(poly2);
        benchmark::DoNotOptimize(distance);
    }
}
BENCHMARK(BM_PolygonDistance);

BENCHMARK_MAIN(); 