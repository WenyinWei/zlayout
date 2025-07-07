-- ZLayout - Advanced EDA Layout Library
-- High-performance C++ implementation with xmake build system

set_project("zlayout")
set_version("1.0.0")
set_languages("c++17")

-- Configure compiler options
add_rules("mode.debug", "mode.release")
add_cxxflags("-Wall", "-Wextra", "-Wpedantic")

if is_mode("debug") then
    add_defines("DEBUG")
    set_symbols("debug")
    set_optimize("none")
elseif is_mode("release") then
    add_defines("NDEBUG")
    set_symbols("hidden")
    set_optimize("fastest")
    add_cxxflags("-flto")  -- Link Time Optimization
end

-- Add include directories
add_includedirs("include")
add_includedirs("include/zlayout")

-- Package dependencies (must be at root scope)
add_requires("catch2", {optional = true})
add_requires("benchmark", {optional = true})

-- Optional dependencies
option("with-visualization")
    set_default(false)
    set_showmenu(true)
    set_description("Enable visualization features (requires OpenGL)")
option_end()

option("with-testing")
    set_default(true)
    set_showmenu(true)
    set_description("Build unit tests")
option_end()

option("with-benchmarks")
    set_default(false)
    set_showmenu(true)
    set_description("Build performance benchmarks")
option_end()

option("openmp")
    set_default(false)
    set_showmenu(true)
    set_description("Enable OpenMP parallel processing support")
option_end()

-- Core library target
target("zlayout")
    set_kind("static")
    add_files("src/geometry/*.cpp")
    add_files("src/spatial/*.cpp")
    add_files("src/analysis/*.cpp")
    add_headerfiles("include/zlayout/**.hpp")
    
    -- Math optimization
    add_links("m")  -- Link math library
    
    -- Optional OpenMP support for parallel processing
    if has_config("openmp") then
        add_packages("openmp")
        add_defines("ZLAYOUT_OPENMP")
    end
    
    -- Export symbols for shared library
    add_defines("ZLAYOUT_EXPORTS")

-- Shared library version
target("zlayout_shared")
    set_kind("shared")
    add_deps("zlayout")
    add_files("src/geometry/*.cpp")
    add_files("src/spatial/*.cpp") 
    add_files("src/analysis/*.cpp")
    add_defines("ZLAYOUT_SHARED")

-- Visualization library (optional)
if has_config("with-visualization") then
    target("zlayout_viz")
        set_kind("static")
        add_deps("zlayout")
        add_files("src/visualization/*.cpp")
        add_headerfiles("include/zlayout/visualization/**.hpp")
        
        -- OpenGL dependencies
        add_packages("glfw", "glad", "glm")
        add_defines("ZLAYOUT_VISUALIZATION")
end

-- Command line tool
target("zlayout_cli")
    set_kind("binary")
    add_deps("zlayout")
    add_files("src/cli/*.cpp")
    add_installfiles("bin/zlayout_cli", {prefixdir = "bin"})

-- Examples
target("basic_example")
    set_kind("binary")
    add_deps("zlayout")
    add_files("examples/basic_usage.cpp")
    set_rundir("examples")

target("eda_example")
    set_kind("binary")
    add_deps("zlayout")
    add_files("examples/eda_circuit.cpp")
    set_rundir("examples")

target("quadtree_demo")
    set_kind("binary")
    add_deps("zlayout")
    add_files("examples/quadtree_demo.cpp")
    set_rundir("examples")

-- Unit tests
if has_config("with-testing") then
    target("test_geometry")
        set_kind("binary")
        add_deps("zlayout")
        add_packages("catch2")
        add_files("tests/test_geometry.cpp")
        add_defines("CATCH_CONFIG_MAIN")
        set_rundir("tests")
    
    target("test_spatial")
        set_kind("binary") 
        add_deps("zlayout")
        add_packages("catch2")
        add_files("tests/test_spatial.cpp")
        add_defines("CATCH_CONFIG_MAIN")
        set_rundir("tests")
    
    target("test_analysis")
        set_kind("binary")
        add_deps("zlayout")
        add_packages("catch2")
        add_files("tests/test_analysis.cpp")
        add_defines("CATCH_CONFIG_MAIN")
        set_rundir("tests")
    
    -- Test runner
    task("test")
        on_run(function ()
            os.exec("xmake run test_geometry")
            os.exec("xmake run test_spatial")
            os.exec("xmake run test_analysis")
        end)
        set_menu({
            usage = "xmake test",
            description = "Run all unit tests"
        })
    task_end()
end

-- Benchmarks
if has_config("with-benchmarks") then
    target("bench_quadtree")
        set_kind("binary")
        add_deps("zlayout")
        add_packages("benchmark")
        add_files("benchmarks/bench_quadtree.cpp")
        set_rundir("benchmarks")
    
    target("bench_geometry")
        set_kind("binary")
        add_deps("zlayout")
        add_packages("benchmark")
        add_files("benchmarks/bench_geometry.cpp")
        set_rundir("benchmarks")
end

-- Documentation
task("docs")
    on_run(function ()
        print("Generating documentation...")
        os.exec("doxygen docs/Doxyfile")
    end)
    set_menu({
        usage = "xmake docs",
        description = "Generate API documentation with Doxygen"
    })
task_end()

-- Installation
target("zlayout")
    add_installfiles("include/zlayout/**.hpp", {prefixdir = "include"})
    add_installfiles("build/$(plat)/$(arch)/$(mode)/libzlayout.a", {prefixdir = "lib"})

-- Package configuration
task("install")
    on_run(function ()
        -- Install headers
        os.cp("include/zlayout", "$(buildir)/install/include/")
        -- Install libraries  
        os.cp("$(buildir)/$(plat)/$(arch)/$(mode)/libzlayout.a", "$(buildir)/install/lib/")
        print("Installation completed to $(buildir)/install/")
    end)
    set_menu({
        usage = "xmake install",
        description = "Install ZLayout library and headers"
    })
task_end()

-- Clean extended
task("distclean")
    on_run(function ()
        os.exec("xmake clean")
        os.rm("build")
        os.rm(".xmake")
        print("Deep clean completed")
    end)
    set_menu({
        usage = "xmake distclean", 
        description = "Clean all build artifacts"
    })
task_end()