#!/usr/bin/env python3
"""
Benchmark automation script for ZLayout
Runs C++ benchmarks and generates results in multiple formats for documentation integration.
"""

import os
import sys
import json
import subprocess
import argparse
import datetime
import platform
import shutil
from pathlib import Path
from typing import Dict, List, Any, Optional

class BenchmarkRunner:
    def __init__(self, build_dir: str = "build", output_dir: str = "benchmark_results"):
        self.build_dir = Path(build_dir)
        self.output_dir = Path(output_dir)
        self.output_dir.mkdir(exist_ok=True)
        self.timestamp = datetime.datetime.now().isoformat()
        
        # System information
        self.system_info = {
            'platform': platform.platform(),
            'processor': platform.processor(),
            'architecture': platform.architecture(),
            'python_version': platform.python_version(),
            'timestamp': self.timestamp
        }
    
    def check_dependencies(self) -> bool:
        """Check if all required tools are available."""
        required_tools = ['xmake']
        
        for tool in required_tools:
            if not shutil.which(tool):
                print(f"Error: {tool} not found in PATH")
                return False
        
        return True
    
    def build_benchmarks(self) -> bool:
        """Build the benchmark executables."""
        print("Building benchmarks...")
        
        try:
            # Configure with benchmarks enabled
            subprocess.run([
                'xmake', 'config', 
                '--with-benchmarks=true',
                '--mode=release',
                '--optimization=fastest'
            ], check=True, capture_output=True)
            
            # Build benchmark targets
            subprocess.run([
                'xmake', 'build', 
                'bench_geometry',
                'bench_quadtree'
            ], check=True, capture_output=True)
            
            return True
            
        except subprocess.CalledProcessError as e:
            print(f"Build failed: {e}")
            return False
    
    def run_benchmark_executable(self, executable: str, output_file: str) -> bool:
        """Run a single benchmark executable and capture results."""
        print(f"Running {executable}...")
        
        try:
            # Run benchmark with JSON output
            result = subprocess.run([
                'xmake', 'run', executable,
                '--benchmark_out=' + output_file,
                '--benchmark_out_format=json',
                '--benchmark_repetitions=3',
                '--benchmark_report_aggregates_only=true'
            ], check=True, capture_output=True, text=True)
            
            print(f"Benchmark {executable} completed successfully")
            return True
            
        except subprocess.CalledProcessError as e:
            print(f"Benchmark {executable} failed: {e}")
            print(f"stdout: {e.stdout}")
            print(f"stderr: {e.stderr}")
            return False
    
    def run_all_benchmarks(self) -> Dict[str, Any]:
        """Run all benchmark executables and collect results."""
        benchmark_results = {}
        
        benchmarks = [
            ('bench_geometry', 'geometry_results.json'),
            ('bench_quadtree', 'quadtree_results.json')
        ]
        
        for benchmark_name, output_file in benchmarks:
            output_path = self.output_dir / output_file
            
            if self.run_benchmark_executable(benchmark_name, str(output_path)):
                try:
                    with open(output_path, 'r') as f:
                        results = json.load(f)
                        benchmark_results[benchmark_name] = results
                except json.JSONDecodeError as e:
                    print(f"Failed to parse results for {benchmark_name}: {e}")
                    benchmark_results[benchmark_name] = None
            else:
                benchmark_results[benchmark_name] = None
        
        return benchmark_results
    
    def generate_summary_report(self, results: Dict[str, Any]) -> Dict[str, Any]:
        """Generate a summary report from benchmark results."""
        summary = {
            'system_info': self.system_info,
            'benchmark_results': {},
            'summary_statistics': {}
        }
        
        for benchmark_name, result_data in results.items():
            if result_data is None:
                continue
                
            benchmark_summary = {
                'total_benchmarks': len(result_data.get('benchmarks', [])),
                'fastest_benchmark': None,
                'slowest_benchmark': None,
                'average_time': 0,
                'benchmarks': []
            }
            
            benchmarks = result_data.get('benchmarks', [])
            if benchmarks:
                times = []
                for bench in benchmarks:
                    time_ns = bench.get('real_time', 0)
                    times.append(time_ns)
                    
                    benchmark_summary['benchmarks'].append({
                        'name': bench.get('name', 'unknown'),
                        'time_ns': time_ns,
                        'time_ms': time_ns / 1_000_000,
                        'iterations': bench.get('iterations', 0),
                        'bytes_per_second': bench.get('bytes_per_second', 0),
                        'items_per_second': bench.get('items_per_second', 0)
                    })
                
                if times:
                    fastest_idx = times.index(min(times))
                    slowest_idx = times.index(max(times))
                    
                    benchmark_summary['fastest_benchmark'] = benchmarks[fastest_idx]['name']
                    benchmark_summary['slowest_benchmark'] = benchmarks[slowest_idx]['name']
                    benchmark_summary['average_time'] = sum(times) / len(times)
            
            summary['benchmark_results'][benchmark_name] = benchmark_summary
        
        return summary
    
    def generate_markdown_report(self, summary: Dict[str, Any]) -> str:
        """Generate a markdown report from the summary."""
        lines = [
            "# ZLayout Benchmark Results",
            "",
            f"Generated on: {summary['system_info']['timestamp']}",
            f"Platform: {summary['system_info']['platform']}",
            f"Processor: {summary['system_info']['processor']}",
            "",
            "## Performance Summary",
            ""
        ]
        
        for benchmark_name, benchmark_data in summary['benchmark_results'].items():
            lines.extend([
                f"### {benchmark_name.replace('_', ' ').title()}",
                "",
                f"- **Total Benchmarks**: {benchmark_data['total_benchmarks']}",
                f"- **Fastest**: {benchmark_data['fastest_benchmark']}",
                f"- **Slowest**: {benchmark_data['slowest_benchmark']}",
                f"- **Average Time**: {benchmark_data['average_time'] / 1_000_000:.2f} ms",
                "",
                "| Benchmark | Time (ms) | Iterations | Items/sec |",
                "|-----------|-----------|------------|-----------|"
            ])
            
            for bench in benchmark_data['benchmarks']:
                items_per_sec = bench['items_per_second']
                items_per_sec_str = f"{items_per_sec:.2e}" if items_per_sec > 0 else "N/A"
                
                lines.append(
                    f"| {bench['name']} | {bench['time_ms']:.3f} | "
                    f"{bench['iterations']:,} | {items_per_sec_str} |"
                )
            
            lines.append("")
        
        return "\n".join(lines)
    
    def update_documentation(self, summary: Dict[str, Any], markdown_report: str) -> None:
        """Update the documentation with new benchmark results."""
        # Update the main benchmark results file
        benchmark_results_file = Path("docs/benchmarks/performance_results.md")
        
        if benchmark_results_file.exists():
            # Read existing file and update the timestamp and system info
            with open(benchmark_results_file, 'r', encoding='utf-8') as f:
                content = f.read()
            
            # Update system info section
            system_info_section = f"""**Test System Specifications:**
- **CPU**: {summary['system_info']['processor']}
- **Platform**: {summary['system_info']['platform']}
- **Architecture**: {summary['system_info']['architecture'][0]}
- **Python**: {summary['system_info']['python_version']}
- **Last Updated**: {summary['system_info']['timestamp']}
- **ZLayout Version**: 1.0.0"""
            
            # Replace the system info section
            import re
            content = re.sub(
                r'\*\*Test System Specifications:\*\*.*?\*\*ZLayout Version\*\*: [^\n]*',
                system_info_section,
                content,
                flags=re.DOTALL
            )
            
            with open(benchmark_results_file, 'w', encoding='utf-8') as f:
                f.write(content)
        
        # Create a timestamped report
        timestamp_str = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
        timestamped_report = self.output_dir / f"benchmark_report_{timestamp_str}.md"
        
        with open(timestamped_report, 'w', encoding='utf-8') as f:
            f.write(markdown_report)
        
        print(f"Documentation updated")
        print(f"Timestamped report saved to: {timestamped_report}")
    
    def run_full_benchmark_suite(self) -> bool:
        """Run the complete benchmark suite and generate reports."""
        print("Starting ZLayout benchmark suite...")
        
        if not self.check_dependencies():
            return False
        
        if not self.build_benchmarks():
            return False
        
        results = self.run_all_benchmarks()
        
        if not any(results.values()):
            print("No benchmark results collected")
            return False
        
        summary = self.generate_summary_report(results)
        markdown_report = self.generate_markdown_report(summary)
        
        # Save results
        summary_file = self.output_dir / "benchmark_summary.json"
        with open(summary_file, 'w') as f:
            json.dump(summary, f, indent=2)
        
        report_file = self.output_dir / "benchmark_report.md"
        with open(report_file, 'w') as f:
            f.write(markdown_report)
        
        self.update_documentation(summary, markdown_report)
        
        print(f"Benchmark suite completed successfully!")
        print(f"Results saved to: {self.output_dir}")
        print(f"Summary: {summary_file}")
        print(f"Report: {report_file}")
        
        return True

def main():
    parser = argparse.ArgumentParser(description='Run ZLayout benchmarks and generate reports')
    parser.add_argument('--build-dir', default='build', help='Build directory')
    parser.add_argument('--output-dir', default='benchmark_results', help='Output directory for results')
    parser.add_argument('--update-docs', action='store_true', help='Update documentation with results')
    
    args = parser.parse_args()
    
    runner = BenchmarkRunner(args.build_dir, args.output_dir)
    
    if runner.run_full_benchmark_suite():
        sys.exit(0)
    else:
        sys.exit(1)

if __name__ == '__main__':
    main() 