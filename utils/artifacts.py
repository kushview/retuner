#!/usr/bin/env python3
"""
Plugin Artifacts Packager
Creates a zip file containing plugin binaries from the build artifacts directory.
"""

import os
import zipfile
import platform
import re
from pathlib import Path

def get_project_version():
    """Extract version from CMakeLists.txt"""
    cmake_file = Path("CMakeLists.txt")
    
    if not cmake_file.exists():
        print("Warning: CMakeLists.txt not found, using default version")
        return "1.0.0"
    
    try:
        with open(cmake_file, 'r') as f:
            content = f.read()
            
        # Look for project(smoothie VERSION x.y.z) pattern
        version_match = re.search(r'project\s*\(\s*retuner\s+VERSION\s+([0-9]+\.[0-9]+\.[0-9]+)', content, re.IGNORECASE)
        
        if version_match:
            return version_match.group(1)
        else:
            print("Warning: Version not found in CMakeLists.txt, using default")
            return "1.0.0"
            
    except Exception as e:
        print(f"Warning: Error reading CMakeLists.txt: {e}")
        return "1.0.0"

def get_platform_name():
    """Get platform name for filename"""
    system = platform.system().lower()
    
    if system == "windows":
        # Detect architecture
        arch = platform.machine().lower()
        if arch in ["amd64", "x86_64"]:
            return "win64"
        elif arch in ["x86", "i386"]:
            return "win32"
        else:
            return "windows"
    elif system == "darwin":
        return "macos"
    elif system == "linux":
        return "linux"
    else:
        return system

def create_plugin_zip():
    """Create a zip file containing Smoothie plugin artifacts."""
    
    # Get version and platform for filename
    version = get_project_version()
    platform_name = get_platform_name()
    
    # Define paths
    artifacts_dir = Path("build/reTuner_artefacts/Release")
    output_zip = f"retuner-{platform_name}-{version}.zip"
    output_dir = 'deploy'

    print(f"Creating package: {output_zip}")
    print(f"Platform: {platform_name}, Version: {version}")
    
    # Check if artifacts directory exists
    if not artifacts_dir.exists():
        print(f"Error: Artifacts directory not found: {artifacts_dir}")
        return False
    
    # Define file extensions to include
    extensions = [".exe", ".lv2", ".clap", ".vst3"]
    
    # Collect files to zip by walking the entire directory tree
    files_to_zip = []
    
    linux_standalone = os.path.join (artifacts_dir, 'Standalone', 'KV-reTuner')
    if  os.path.exists (linux_standalone):
        files_to_zip.append (str (linux_standalone))
    
    # Walk through all files and directories recursively
    for root, dirs, files in os.walk(artifacts_dir):
        for file in files:
            file_path = Path(root) / file
            file_ext = file_path.suffix.lower()
            
            # Check if file has one of our target extensions
            if file_ext in extensions:
                files_to_zip.append(str(file_path))
        
        # Also check for directories with target extensions (like .lv2, .vst3 bundles)
        for dir_name in dirs:
            dir_path = Path(root) / dir_name
            dir_ext = dir_path.suffix.lower()
            
            if dir_ext in [".lv2", ".vst3", ".component"]:
                # Add all files within the plugin bundle directory
                for bundle_root, bundle_dirs, bundle_files in os.walk(dir_path):
                    for bundle_file in bundle_files:
                        bundle_file_path = Path(bundle_root) / bundle_file
                        files_to_zip.append(str(bundle_file_path))
    
    if not files_to_zip:
        print(f"No plugin files found in {artifacts_dir}")
        return False
    
    # Ensure output directory exists
    os.makedirs(output_dir, exist_ok=True)
    
    # Prepend output directory to zip filename
    output_zip_path = os.path.join(output_dir, output_zip)
    
    print(f"Creating {output_zip_path} with {len(files_to_zip)} files...")
    
    # Create the zip file
    try:
        with zipfile.ZipFile(output_zip_path, 'w', zipfile.ZIP_DEFLATED) as zipf:
            for file_path in files_to_zip:
                # Calculate relative path from artifacts directory to preserve structure
                rel_path = os.path.relpath(file_path, artifacts_dir)
                zipf.write(file_path, rel_path)
                print(f"  Added: {rel_path}")
        
        print(f"Successfully created {output_zip_path}")
        
        # Show zip file size
        zip_size = os.path.getsize(output_zip_path)
        print(f"Zip file size: {zip_size / (1024*1024):.1f} MB")
        
        return True
        
    except Exception as e:
        print(f"Error creating zip file: {e}")
        return False

def list_artifacts():
    """List all available artifacts in the build directory."""
    artifacts_dir = Path("build/reTuner_artefacts/Release")
    
    if not artifacts_dir.exists():
        print(f"Artifacts directory not found: {artifacts_dir}")
        return
    
    print(f"Contents of {artifacts_dir}:")
    for item in artifacts_dir.iterdir():
        if item.is_file():
            print(f"  File: {item.name} ({item.stat().st_size} bytes)")
        elif item.is_dir():
            print(f"  Dir:  {item.name}/")
            # Show contents of plugin directories
            if item.suffix in [".lv2", ".vst3"]:
                for subitem in item.rglob("*"):
                    if subitem.is_file():
                        rel_path = subitem.relative_to(artifacts_dir)
                        print(f"    {rel_path}")

if __name__ == "__main__":
    import sys
    
    if len(sys.argv) > 1 and sys.argv[1] == "list":
        list_artifacts()
    else:        
        if not create_plugin_zip():
            exit(1)
    exit(0)
