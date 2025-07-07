#!/usr/bin/env python3
"""
Benchmark comparison script for ZLayout
Compares benchmark results between different runs and generates performance regression reports.
"""

import json
import argparse
import sys
from pathlib import Path
from typing import Dict, List, Tuple, Optional, Any
import datetime

class BenchmarkComparator:
    def __init__(self, tolerance: float = 0.05):
        """
        Initialize benchmark comparator.
        
        Args:
            tolerance: Performance regression tolerance (5% by default)
        """
        self.tolerance = tolerance
    
    def load_benchmark_results(self, file_path: str) -> Optional[Dict[str, Any]]:
        """Load benchmark results from JSON file."""
        try:
            with open(file_path, 'r') as f:
                return json.load(f)
        except (FileNotFoundError, json.JSONDecodeError) as e:
            print(f"Failed to load benchmark results from {file_path}: {e}")
            return None
    
    def extract_benchmark_metrics(self, results: Dict[str, Any]) -> Dict[str, Dict[str, float]]:
        """Extract key metrics from benchmark results."""
        metrics = {}
        
        for benchmark_name, benchmark_data in results.get('benchmark_results', {}).items():
            if benchmark_data is None:
                continue
                
            benchmark_metrics = {}
            
            for bench in benchmark_data.get('benchmarks', []):
                bench_name = bench.get('name', 'unknown')
                benchmark_metrics[bench_name] = {
                    'time_ns': bench.get('time_ns', 0),
                    'time_ms': bench.get('time_ms', 0),
                    'iterations': bench.get('iterations', 0),
                    'items_per_second': bench.get('items_per_second', 0)
                }
            
            metrics[benchmark_name] = benchmark_metrics
        
        return metrics
    
    def compare_benchmark_metrics(self, baseline: Dict[str, Dict[str, Any]], 
                                  current: Dict[str, Dict[str, Any]]) -> Dict[str, Any]:
        """Compare benchmark metrics and identify regressions/improvements."""
        comparison = {
            'summary': {
                'total_benchmarks': 0,
                'improved': 0,
                'regressed': 0,
                'unchanged': 0,
                'new_benchmarks': 0,
                'removed_benchmarks': 0
            },
            'detailed_results': {},
            'regressions': [],
            'improvements': []
        }
        
        # Get all unique benchmark names
        all_benchmarks = set()
        for benchmark_group in [baseline, current]:
            for benchmark_name in benchmark_group:
                for bench_name in benchmark_group[benchmark_name]:
                    all_benchmarks.add((benchmark_name, bench_name))
        
        comparison['summary']['total_benchmarks'] = len(all_benchmarks)
        
        for benchmark_name, bench_name in all_benchmarks:
            baseline_exists = (benchmark_name in baseline and 
                             bench_name in baseline[benchmark_name])
            current_exists = (benchmark_name in current and 
                            bench_name in current[benchmark_name])
            
            if baseline_exists and current_exists:
                # Compare existing benchmarks
                baseline_time = baseline[benchmark_name][bench_name]['time_ns']
                current_time = current[benchmark_name][bench_name]['time_ns']
                
                if baseline_time > 0:
                    change_ratio = (current_time - baseline_time) / baseline_time
                    change_percent = change_ratio * 100
                    
                    result = {
                        'benchmark_group': benchmark_name,
                        'benchmark_name': bench_name,
                        'baseline_time_ns': baseline_time,
                        'current_time_ns': current_time,
                        'change_ratio': change_ratio,
                        'change_percent': change_percent,
                        'status': 'unchanged'
                    }
                    
                    if change_ratio > self.tolerance:
                        result['status'] = 'regressed'
                        comparison['summary']['regressed'] += 1
                        comparison['regressions'].append(result)
                    elif change_ratio < -self.tolerance:
                        result['status'] = 'improved'
                        comparison['summary']['improved'] += 1
                        comparison['improvements'].append(result)
                    else:
                        comparison['summary']['unchanged'] += 1
                    
                    comparison['detailed_results'][f"{benchmark_name}::{bench_name}"] = result
                
            elif not baseline_exists and current_exists:
                # New benchmark
                comparison['summary']['new_benchmarks'] += 1
                comparison['detailed_results'][f"{benchmark_name}::{bench_name}"] = {
                    'benchmark_group': benchmark_name,
                    'benchmark_name': bench_name,
                    'current_time_ns': current[benchmark_name][bench_name]['time_ns'],
                    'status': 'new'
                }
                
            elif baseline_exists and not current_exists:
                # Removed benchmark
                comparison['summary']['removed_benchmarks'] += 1
                comparison['detailed_results'][f"{benchmark_name}::{bench_name}"] = {
                    'benchmark_group': benchmark_name,
                    'benchmark_name': bench_name,
                    'baseline_time_ns': baseline[benchmark_name][bench_name]['time_ns'],
                    'status': 'removed'
                }
        
        return comparison
    
    def generate_comparison_report(self, comparison: Dict[str, Any], 
                                   baseline_info: Dict[str, Any], 
                                   current_info: Dict[str, Any]) -> str:
        """Generate a markdown report from comparison results."""
        lines = [
            "# ZLayout Benchmark Comparison Report",
            "",
            f"**Baseline**: {baseline_info.get('system_info', {}).get('timestamp', 'Unknown')}",
            f"**Current**: {current_info.get('system_info', {}).get('timestamp', 'Unknown')}",
            f"**Tolerance**: ±{self.tolerance * 100:.1f}%",
            "",
            "## Summary",
            "",
            f"- **Total Benchmarks**: {comparison['summary']['total_benchmarks']}",
            f"- **Improved**: {comparison['summary']['improved']} ✅",
            f"- **Regressed**: {comparison['summary']['regressed']} ❌",
            f"- **Unchanged**: {comparison['summary']['unchanged']} ➖",
            f"- **New**: {comparison['summary']['new_benchmarks']} 🆕",
            f"- **Removed**: {comparison['summary']['removed_benchmarks']} 🗑️",
            ""
        ]
        
        # Performance status
        if comparison['summary']['regressed'] > 0:
            lines.extend([
                "## ❌ Performance Regressions",
                "",
                "The following benchmarks have regressed beyond the tolerance threshold:",
                "",
                "| Benchmark | Baseline | Current | Change |",
                "|-----------|----------|---------|--------|"
            ])
            
            for regression in comparison['regressions']:
                baseline_ms = regression['baseline_time_ns'] / 1_000_000
                current_ms = regression['current_time_ns'] / 1_000_000
                change_str = f"+{regression['change_percent']:.1f}%"
                
                lines.append(
                    f"| {regression['benchmark_name']} | {baseline_ms:.3f}ms | "
                    f"{current_ms:.3f}ms | {change_str} |"
                )
            
            lines.append("")
        
        if comparison['summary']['improved'] > 0:
            lines.extend([
                "## ✅ Performance Improvements",
                "",
                "The following benchmarks have improved:",
                "",
                "| Benchmark | Baseline | Current | Change |",
                "|-----------|----------|---------|--------|"
            ])
            
            for improvement in comparison['improvements']:
                baseline_ms = improvement['baseline_time_ns'] / 1_000_000
                current_ms = improvement['current_time_ns'] / 1_000_000
                change_str = f"{improvement['change_percent']:.1f}%"
                
                lines.append(
                    f"| {improvement['benchmark_name']} | {baseline_ms:.3f}ms | "
                    f"{current_ms:.3f}ms | {change_str} |"
                )
            
            lines.append("")
        
        if comparison['summary']['new_benchmarks'] > 0:
            lines.extend([
                "## 🆕 New Benchmarks",
                "",
                "| Benchmark | Time |",
                "|-----------|------|"
            ])
            
            for result in comparison['detailed_results'].values():
                if result['status'] == 'new':
                    current_ms = result['current_time_ns'] / 1_000_000
                    lines.append(f"| {result['benchmark_name']} | {current_ms:.3f}ms |")
            
            lines.append("")
        
        if comparison['summary']['removed_benchmarks'] > 0:
            lines.extend([
                "## 🗑️ Removed Benchmarks",
                "",
                "| Benchmark | Previous Time |",
                "|-----------|---------------|"
            ])
            
            for result in comparison['detailed_results'].values():
                if result['status'] == 'removed':
                    baseline_ms = result['baseline_time_ns'] / 1_000_000
                    lines.append(f"| {result['benchmark_name']} | {baseline_ms:.3f}ms |")
            
            lines.append("")
        
        # Overall assessment
        lines.extend([
            "## Overall Assessment",
            ""
        ])
        
        if comparison['summary']['regressed'] == 0:
            lines.append("✅ **No performance regressions detected**")
        else:
            lines.append(f"❌ **{comparison['summary']['regressed']} performance regressions detected**")
        
        if comparison['summary']['improved'] > 0:
            lines.append(f"✅ **{comparison['summary']['improved']} performance improvements**")
        
        return "\n".join(lines)
    
    def compare_benchmark_files(self, baseline_file: str, current_file: str) -> Tuple[bool, str]:
        """
        Compare two benchmark result files.
        
        Returns:
            Tuple of (success, report_text)
        """
        baseline_results = self.load_benchmark_results(baseline_file)
        current_results = self.load_benchmark_results(current_file)
        
        if baseline_results is None or current_results is None:
            return False, "Failed to load benchmark results"
        
        baseline_metrics = self.extract_benchmark_metrics(baseline_results)
        current_metrics = self.extract_benchmark_metrics(current_results)
        
        comparison = self.compare_benchmark_metrics(baseline_metrics, current_metrics)
        report = self.generate_comparison_report(comparison, baseline_results, current_results)
        
        # Determine success based on regressions
        success = comparison['summary']['regressed'] == 0
        
        return success, report

def main():
    parser = argparse.ArgumentParser(description='Compare ZLayout benchmark results')
    parser.add_argument('baseline', help='Path to baseline benchmark results (JSON)')
    parser.add_argument('current', help='Path to current benchmark results (JSON)')
    parser.add_argument('--tolerance', type=float, default=0.05, 
                       help='Performance regression tolerance (default: 0.05 = 5%)')
    parser.add_argument('--output', help='Output file for comparison report')
    parser.add_argument('--fail-on-regression', action='store_true',
                       help='Exit with error code if regressions are detected')
    
    args = parser.parse_args()
    
    comparator = BenchmarkComparator(args.tolerance)
    success, report = comparator.compare_benchmark_files(args.baseline, args.current)
    
    if args.output:
        with open(args.output, 'w') as f:
            f.write(report)
        print(f"Comparison report saved to: {args.output}")
    else:
        print(report)
    
    if args.fail_on_regression and not success:
        print("\n❌ Performance regressions detected!", file=sys.stderr)
        sys.exit(1)
    
    if success:
        print("\n✅ No performance regressions detected!")
        sys.exit(0)
    else:
        sys.exit(0)  # Don't fail by default, just report

if __name__ == '__main__':
    main() 