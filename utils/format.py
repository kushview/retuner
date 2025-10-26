#!/usr/bin/env python3
"""
Code formatting script for ReTuner project.
Runs clang-format on all C++ source files in the src directory.
"""

import os
import subprocess
import sys
from pathlib import Path

def find_source_files(src_dir):
    """Find all C++ source files in the given directory."""
    extensions = {'.cpp', '.hpp', '.cc', '.cxx', '.h', '.hxx'}
    source_files = []
    
    for root, dirs, files in os.walk(src_dir):
        for file in files:
            if any(file.endswith(ext) for ext in extensions):
                source_files.append(os.path.join(root, file))
    
    return sorted(source_files)

def run_clang_format(files, check_only=False):
    """Run clang-format on the given files."""
    if not files:
        print("No source files found.")
        return True
    
    # Check if clang-format is available
    try:
        subprocess.run(['clang-format', '--version'], 
                      check=True, capture_output=True)
    except (subprocess.CalledProcessError, FileNotFoundError):
        print("Error: clang-format not found. Please install clang-format.")
        return False
    
    cmd = ['clang-format']
    if check_only:
        cmd.extend(['--dry-run', '--Werror'])
        print("Checking code formatting...")
    else:
        cmd.append('-i')  # In-place formatting
        print("Formatting code...")
    
    # Process files in batches to avoid command line length limits
    batch_size = 50
    success = True
    
    for i in range(0, len(files), batch_size):
        batch = files[i:i + batch_size]
        try:
            result = subprocess.run(cmd + batch, check=check_only, 
                                  capture_output=True, text=True)
            if check_only and result.returncode != 0:
                print("Code formatting issues found:")
                print(result.stderr)
                success = False
            elif not check_only:
                print(f"Formatted {len(batch)} files...")
        except subprocess.CalledProcessError as e:
            print(f"Error running clang-format: {e}")
            if e.stderr:
                print(e.stderr)
            success = False
    
    return success

def main():
    """Main function."""
    # Get the project root directory (parent of utils)
    script_dir = Path(__file__).parent
    project_root = script_dir.parent
    src_dir = project_root / "src"
    
    if not src_dir.exists():
        print(f"Error: Source directory {src_dir} not found.")
        sys.exit(1)
    
    # Parse command line arguments
    check_only = '--check' in sys.argv
    
    print(f"Looking for source files in: {src_dir}")
    source_files = find_source_files(src_dir)
    
    if not source_files:
        print("No C++ source files found.")
        sys.exit(0)
    
    print(f"Found {len(source_files)} source files:")
    for file in source_files:
        print(f"  {os.path.relpath(file, project_root)}")
    print()
    
    success = run_clang_format(source_files, check_only)
    
    if check_only:
        if success:
            print("✓ All files are properly formatted.")
        else:
            print("✗ Some files need formatting. Run without --check to fix.")
            sys.exit(1)
    else:
        if success:
            print("✓ Code formatting complete.")
        else:
            print("✗ Some errors occurred during formatting.")
            sys.exit(1)

if __name__ == "__main__":
    main()
