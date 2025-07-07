#!/usr/bin/env python3
"""
Test script for documentation generation
Verifies that Doxygen can generate documentation correctly.
"""

import os
import subprocess
import sys
from pathlib import Path

def test_doxygen_generation():
    """Test if Doxygen can generate documentation successfully."""
    print("Testing Doxygen documentation generation...")
    
    # Check if Doxygen is available
    try:
        result = subprocess.run(['doxygen', '--version'], 
                              capture_output=True, text=True, check=True)
        print(f"✅ Doxygen version: {result.stdout.strip()}")
    except (subprocess.CalledProcessError, FileNotFoundError):
        print("❌ Doxygen not found. Please install Doxygen.")
        return False
    
    # Check if Doxyfile exists
    doxyfile = Path("Doxyfile")
    if not doxyfile.exists():
        print("❌ Doxyfile not found")
        return False
    
    print("✅ Doxyfile found")
    
    # Generate documentation
    try:
        print("Generating documentation...")
        result = subprocess.run(['doxygen', 'Doxyfile'], 
                              capture_output=True, text=True, check=True)
        print("✅ Documentation generation completed successfully")
        
        # Check if output directory was created
        output_dir = Path("docs/html")
        if output_dir.exists():
            print(f"✅ Output directory created: {output_dir}")
            
            # Check for key files
            index_file = output_dir / "index.html"
            if index_file.exists():
                print("✅ index.html generated")
            else:
                print("⚠️  index.html not found")
                
            # List some files
            files = list(output_dir.glob("*.html"))
            print(f"✅ Generated {len(files)} HTML files")
            
        else:
            print("❌ Output directory not created")
            return False
            
    except subprocess.CalledProcessError as e:
        print(f"❌ Documentation generation failed:")
        print(f"stdout: {e.stdout}")
        print(f"stderr: {e.stderr}")
        return False
    
    return True

def test_benchmark_scripts():
    """Test if benchmark scripts can be imported."""
    print("\nTesting benchmark scripts...")
    
    try:
        # Test import of benchmark scripts
        sys.path.insert(0, 'scripts')
        
        import run_benchmarks
        print("✅ run_benchmarks.py imports successfully")
        
        import compare_benchmarks
        print("✅ compare_benchmarks.py imports successfully")
        
    except ImportError as e:
        print(f"❌ Failed to import benchmark scripts: {e}")
        return False
    
    return True

def main():
    """Run all documentation tests."""
    print("🧪 Running documentation tests...\n")
    
    success = True
    
    # Test Doxygen generation
    if not test_doxygen_generation():
        success = False
    
    # Test benchmark scripts
    if not test_benchmark_scripts():
        success = False
    
    print("\n" + "="*50)
    if success:
        print("✅ All documentation tests passed!")
        print("The CI workflow should work correctly.")
    else:
        print("❌ Some documentation tests failed.")
        print("Please fix the issues before pushing to CI.")
    
    return 0 if success else 1

if __name__ == '__main__':
    sys.exit(main()) 