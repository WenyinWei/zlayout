#!/usr/bin/env python3
"""
CI Workflow Testing Script for ZLayout

This script tests all components of the GitHub Actions CI workflow
that can be tested locally without external dependencies.
"""

import os
import sys
import subprocess
import json
from pathlib import Path

def run_command(cmd, cwd=None, shell=True):
    """Run a command and return success status and output."""
    try:
        result = subprocess.run(
            cmd, 
            cwd=cwd, 
            shell=shell, 
            capture_output=True, 
            text=True,
            timeout=300  # 5 minute timeout
        )
        return result.returncode == 0, result.stdout, result.stderr
    except subprocess.TimeoutExpired:
        return False, "", "Command timed out"
    except Exception as e:
        return False, "", str(e)

def test_python_environment():
    """Test Python environment and dependencies."""
    print("=== Testing Python Environment ===")
    
    # Check Python version
    print(f"Python version: {sys.version}")
    
    # Check required modules
    required_modules = ['pytest', 'matplotlib', 'os', 'sys', 'json', 'pathlib']
    missing_modules = []
    
    for module in required_modules:
        try:
            __import__(module)
            print(f"✅ {module} available")
        except ImportError:
            print(f"❌ {module} missing")
            missing_modules.append(module)
    
    if missing_modules:
        print(f"Missing modules: {missing_modules}")
        return False
    
    print("✅ Python environment OK")
    return True

def test_project_structure():
    """Test that all required project files exist."""
    print("\n=== Testing Project Structure ===")
    
    required_files = [
        'xmake.lua',
        'Doxyfile',
        '.github/workflows/docs.yml',
        '.gitignore',
        'docs/index.md',
        'docs/tutorials/getting_started.md',
        'docs/examples/eda_circuit_analysis.md',
        'docs/benchmarks/performance_results.md',
        'src/zlayout.cpp',
        'include/zlayout/zlayout.hpp'
    ]
    
    required_dirs = [
        'src',
        'include',
        'docs',
        'tests',
        'examples',
        'benchmarks',
        '.github/workflows'
    ]
    
    missing_files = []
    missing_dirs = []
    
    for file_path in required_files:
        if not os.path.exists(file_path):
            missing_files.append(file_path)
            print(f"❌ Missing file: {file_path}")
        else:
            print(f"✅ Found: {file_path}")
    
    for dir_path in required_dirs:
        if not os.path.isdir(dir_path):
            missing_dirs.append(dir_path)
            print(f"❌ Missing directory: {dir_path}")
        else:
            print(f"✅ Found directory: {dir_path}")
    
    if missing_files or missing_dirs:
        print(f"Missing files: {missing_files}")
        print(f"Missing directories: {missing_dirs}")
        return False
    
    print("✅ Project structure OK")
    return True

def test_python_tests():
    """Run Python tests."""
    print("\n=== Running Python Tests ===")
    
    # Try different ways to run tests
    test_commands = [
        ['python', 'tests/run_tests.py'],
        ['python', '-m', 'pytest', 'tests/', '-v'],
        ['python', '-m', 'unittest', 'discover', 'tests']
    ]
    
    for cmd in test_commands:
        print(f"Trying: {' '.join(cmd)}")
        success, stdout, stderr = run_command(cmd)
        
        if success:
            print("✅ Python tests passed")
            print("Test output summary:")
            lines = stdout.split('\n')
            for line in lines[-10:]:  # Show last 10 lines
                if line.strip():
                    print(f"  {line}")
            return True
        else:
            print(f"❌ Test command failed: {' '.join(cmd)}")
            if stderr:
                print(f"Error: {stderr[:200]}...")
    
    print("❌ All Python test methods failed")
    return False

def test_documentation_files():
    """Test documentation file integrity."""
    print("\n=== Testing Documentation Files ===")
    
    doc_files = [
        'docs/index.md',
        'docs/tutorials/getting_started.md', 
        'docs/examples/eda_circuit_analysis.md',
        'docs/benchmarks/performance_results.md'
    ]
    
    all_good = True
    
    for doc_file in doc_files:
        if not os.path.exists(doc_file):
            print(f"❌ Missing: {doc_file}")
            all_good = False
            continue
            
        # Check file content
        try:
            with open(doc_file, 'r', encoding='utf-8') as f:
                content = f.read()
                
            if len(content) < 100:
                print(f"❌ {doc_file} too short ({len(content)} chars)")
                all_good = False
            elif '# ' not in content:
                print(f"❌ {doc_file} missing markdown headers")
                all_good = False
            else:
                print(f"✅ {doc_file} OK ({len(content)} chars)")
                
        except Exception as e:
            print(f"❌ Error reading {doc_file}: {e}")
            all_good = False
    
    if all_good:
        print("✅ Documentation files OK")
    
    return all_good

def test_configuration_files():
    """Test configuration file syntax."""
    print("\n=== Testing Configuration Files ===")
    
    all_good = True
    
    # Test Doxyfile
    print("Testing Doxyfile...")
    if os.path.exists('Doxyfile'):
        try:
            with open('Doxyfile', 'r') as f:
                content = f.read()
            
            required_settings = [
                'PROJECT_NAME',
                'OUTPUT_DIRECTORY',
                'INPUT',
                'GENERATE_HTML'
            ]
            
            missing_settings = []
            for setting in required_settings:
                if setting not in content:
                    missing_settings.append(setting)
            
            if missing_settings:
                print(f"❌ Doxyfile missing settings: {missing_settings}")
                all_good = False
            else:
                print("✅ Doxyfile OK")
                
        except Exception as e:
            print(f"❌ Error reading Doxyfile: {e}")
            all_good = False
    else:
        print("❌ Doxyfile not found")
        all_good = False
    
    # Test GitHub Actions workflow
    print("Testing GitHub Actions workflow...")
    workflow_file = '.github/workflows/docs.yml'
    if os.path.exists(workflow_file):
        try:
            with open(workflow_file, 'r') as f:
                content = f.read()
            
            required_sections = [
                'name:',
                'on:',
                'jobs:',
                'build-docs:',
                'steps:'
            ]
            
            missing_sections = []
            for section in required_sections:
                if section not in content:
                    missing_sections.append(section)
            
            if missing_sections:
                print(f"❌ Workflow missing sections: {missing_sections}")
                all_good = False
            else:
                print("✅ GitHub Actions workflow OK")
                
        except Exception as e:
            print(f"❌ Error reading workflow: {e}")
            all_good = False
    else:
        print("❌ GitHub Actions workflow not found")
        all_good = False
    
    return all_good

def test_example_files():
    """Test that example files are valid and can be read."""
    print("\n=== Testing Example Files ===")
    
    example_dirs = ['examples', 'docs/examples']
    all_good = True
    
    for example_dir in example_dirs:
        if not os.path.exists(example_dir):
            print(f"❌ Example directory not found: {example_dir}")
            all_good = False
            continue
            
        example_files = []
        for ext in ['.py', '.cpp', '.hpp', '.md']:
            example_files.extend(Path(example_dir).glob(f'*{ext}'))
        
        if not example_files:
            print(f"❌ No example files found in {example_dir}")
            all_good = False
            continue
        
        for example_file in example_files:
            try:
                with open(example_file, 'r', encoding='utf-8') as f:
                    content = f.read()
                
                if len(content) < 50:
                    print(f"❌ {example_file} too short")
                    all_good = False
                else:
                    print(f"✅ {example_file} OK ({len(content)} chars)")
                    
            except Exception as e:
                print(f"❌ Error reading {example_file}: {e}")
                all_good = False
    
    return all_good

def simulate_ci_environment():
    """Simulate CI environment variables and conditions."""
    print("\n=== Simulating CI Environment ===")
    
    # Set CI-like environment variables
    ci_env = {
        'CI': 'true',
        'GITHUB_ACTIONS': 'true',
        'GITHUB_WORKSPACE': os.getcwd(),
        'RUNNER_OS': 'Windows'
    }
    
    for key, value in ci_env.items():
        os.environ[key] = value
        print(f"Set {key}={value}")
    
    # Test environment
    print("✅ CI environment simulated")
    return True

def generate_test_report():
    """Generate a comprehensive test report."""
    print("\n" + "="*60)
    print("           ZLAYOUT CI WORKFLOW TEST REPORT")
    print("="*60)
    
    tests = [
        ("Python Environment", test_python_environment),
        ("Project Structure", test_project_structure),
        ("Python Tests", test_python_tests),
        ("Documentation Files", test_documentation_files),
        ("Configuration Files", test_configuration_files),
        ("Example Files", test_example_files),
        ("CI Environment", simulate_ci_environment)
    ]
    
    results = {}
    total_tests = len(tests)
    passed_tests = 0
    
    for test_name, test_func in tests:
        try:
            result = test_func()
            results[test_name] = result
            if result:
                passed_tests += 1
        except Exception as e:
            print(f"❌ {test_name} failed with exception: {e}")
            results[test_name] = False
    
    # Summary
    print("\n" + "="*60)
    print("                    TEST SUMMARY")
    print("="*60)
    
    for test_name, result in results.items():
        status = "✅ PASS" if result else "❌ FAIL"
        print(f"{test_name:.<30} {status}")
    
    print(f"\nTotal: {passed_tests}/{total_tests} tests passed")
    
    if passed_tests == total_tests:
        print("\n🎉 ALL TESTS PASSED! CI workflow should work correctly.")
        return True
    else:
        print(f"\n⚠️  {total_tests - passed_tests} tests failed. CI workflow may have issues.")
        return False

def main():
    """Main test runner."""
    print("ZLayout CI Workflow Test")
    print("This script tests the GitHub Actions workflow components locally.")
    print("=" * 60)
    
    # Change to repo root if needed
    if not os.path.exists('xmake.lua'):
        print("❌ Not in ZLayout repository root")
        sys.exit(1)
    
    success = generate_test_report()
    
    if success:
        print("\n✅ Ready for CI deployment!")
        sys.exit(0)
    else:
        print("\n❌ Issues found - fix before deploying")
        sys.exit(1)

if __name__ == "__main__":
    main() 