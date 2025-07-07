# ZLayout Scripts

This directory contains automation scripts for the ZLayout project, including benchmark automation and documentation generation.

## Benchmark Automation

### `run_benchmarks.py`

Comprehensive benchmark automation script that runs C++ benchmarks and generates results in multiple formats for documentation integration.

#### Usage

```bash
# Run all benchmarks with default settings
python scripts/run_benchmarks.py

# Specify custom build and output directories
python scripts/run_benchmarks.py --build-dir build --output-dir my_results

# Run and update documentation
python scripts/run_benchmarks.py --update-docs
```

#### Features

- **Automatic Build**: Configures and builds benchmark executables using XMake
- **Multi-format Output**: Generates JSON, markdown, and summary reports
- **System Information**: Captures hardware and software configuration
- **Documentation Integration**: Automatically updates benchmark results in documentation
- **Error Handling**: Robust error handling and reporting

#### Output Files

- `benchmark_summary.json`: Complete benchmark results with system information
- `benchmark_report.md`: Human-readable markdown report
- `geometry_results.json`: Raw geometry benchmark results
- `quadtree_results.json`: Raw spatial index benchmark results
- `benchmark_report_YYYYMMDD_HHMMSS.md`: Timestamped reports

### `compare_benchmarks.py`

Benchmark comparison script that compares results between different runs and generates performance regression reports.

#### Usage

```bash
# Compare two benchmark result files
python scripts/compare_benchmarks.py baseline.json current.json

# Set custom regression tolerance (default 5%)
python scripts/compare_benchmarks.py baseline.json current.json --tolerance 0.10

# Save comparison report to file
python scripts/compare_benchmarks.py baseline.json current.json --output comparison_report.md

# Exit with error if regressions are detected (useful for CI)
python scripts/compare_benchmarks.py baseline.json current.json --fail-on-regression
```

#### Features

- **Regression Detection**: Automatically identifies performance regressions
- **Improvement Tracking**: Highlights performance improvements
- **Tolerance Settings**: Configurable tolerance for regression detection
- **Detailed Reports**: Comprehensive markdown reports with tables and summaries
- **CI Integration**: Exit codes for automated pipeline integration

#### Report Sections

- **Summary**: Overview of benchmark comparison results
- **Performance Regressions**: Benchmarks that have slowed down
- **Performance Improvements**: Benchmarks that have sped up
- **New Benchmarks**: Benchmarks added since baseline
- **Removed Benchmarks**: Benchmarks removed since baseline
- **Overall Assessment**: Summary of performance impact

## CI Integration

The benchmark automation is integrated into the GitHub Actions workflow:

1. **Benchmark Job**: Runs on every push to main branch
2. **Automatic Documentation Updates**: Updates benchmark results in documentation
3. **Performance Monitoring**: Tracks performance changes over time
4. **Artifact Storage**: Stores benchmark results as CI artifacts

### Workflow Configuration

The CI workflow includes:

```yaml
benchmark:
  runs-on: ubuntu-latest
  needs: [test, build]
  if: github.ref == 'refs/heads/main'
  
  steps:
    - name: Run Benchmarks
      run: python scripts/run_benchmarks.py --output-dir benchmark_results
    
    - name: Update documentation
      run: |
        git add docs/benchmarks/performance_results.md
        git commit -m "📊 Update benchmark results [skip ci]"
        git push
```

## Requirements

### System Requirements

- **Python 3.8+**: Required for running automation scripts
- **XMake 2.6+**: Build system for C++ benchmarks
- **Google Benchmark**: C++ benchmarking framework
- **Git**: Version control (for CI integration)

### Python Dependencies

```bash
# Install from requirements.txt
pip install -r requirements.txt

# Or install individually
pip install json argparse pathlib typing datetime
```

### Building Requirements

```bash
# Install system dependencies (Ubuntu/Debian)
sudo apt-get install build-essential cmake git curl

# Install Google Benchmark
git clone https://github.com/google/benchmark.git
cd benchmark
cmake -E make_directory "build"
cmake -E chdir "build" cmake -DBENCHMARK_DOWNLOAD_DEPENDENCIES=on -DCMAKE_BUILD_TYPE=Release ../
cmake --build "build" --config Release --target install
```

## Best Practices

### Running Benchmarks

1. **Consistent Environment**: Run benchmarks on the same hardware configuration
2. **Minimal Background Load**: Close unnecessary applications during benchmarking
3. **Multiple Runs**: Use benchmark repetitions for statistical significance
4. **Thermal Management**: Allow system to cool between intensive benchmark runs

### Performance Monitoring

1. **Regular Benchmarking**: Run benchmarks on every major change
2. **Baseline Establishment**: Maintain stable baseline results for comparison
3. **Regression Investigation**: Investigate any performance regressions immediately
4. **Performance Budgets**: Set performance budgets for different operations

### Documentation Updates

1. **Automated Updates**: Use CI to automatically update benchmark results
2. **Historical Tracking**: Keep timestamped benchmark reports for trend analysis
3. **Performance Explanations**: Document reasons for significant performance changes
4. **Benchmark Descriptions**: Maintain clear descriptions of what each benchmark measures

## Troubleshooting

### Common Issues

**Build Failures**:
- Ensure XMake is installed and in PATH
- Check system dependencies (build-essential, cmake)
- Verify C++ compiler is available

**Benchmark Failures**:
- Check Google Benchmark installation
- Verify benchmark executables are built correctly
- Ensure sufficient system resources

**Documentation Updates**:
- Check file permissions for documentation directory
- Verify git configuration for automated commits
- Ensure CI has proper repository access

### Getting Help

For issues with benchmark automation:

1. Check the GitHub Actions logs for detailed error messages
2. Run benchmarks locally to isolate issues
3. Review system requirements and dependencies
4. Submit issues to the project repository

## Contributing

When adding new benchmarks:

1. **Add C++ Benchmark**: Create benchmark in `benchmarks/` directory
2. **Update Build Configuration**: Add benchmark target to `xmake.lua`
3. **Test Locally**: Run `python scripts/run_benchmarks.py` to verify
4. **Documentation**: Update this README with any new features
5. **CI Testing**: Ensure benchmarks run correctly in CI environment

For script improvements:

1. **Maintain Compatibility**: Ensure changes work with existing CI setup
2. **Add Tests**: Include unit tests for new functionality
3. **Update Documentation**: Keep this README current with changes
4. **Performance Considerations**: Optimize scripts for CI execution time 