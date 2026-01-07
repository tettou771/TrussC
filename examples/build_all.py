#!/usr/bin/env python3
import os
import sys
import platform
import subprocess
import shutil
import argparse
import glob
from pathlib import Path

# =============================================================================
# Configuration & Constants
# =============================================================================

# Colors for terminal output
class Colors:
    HEADER = '\033[95m'
    BLUE = '\033[94m'
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    RED = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'

    @staticmethod
    def print(msg, color=ENDC):
        if sys.stdout.isatty():
            print(f"{color}{msg}{Colors.ENDC}")
        else:
            print(msg)

# Paths
SCRIPT_DIR = Path(__file__).resolve().parent
ROOT_DIR = SCRIPT_DIR.parent

# =============================================================================
# Helper Functions
# =============================================================================

def detect_platform_preset():
    system = platform.system()
    if system == 'Darwin':
        return 'macos'
    elif system == 'Windows':
        return 'windows'
    elif system == 'Linux':
        return 'linux'
    else:
        Colors.print(f"Unknown platform: {system}", Colors.RED)
        sys.exit(1)

def find_project_generator(preset):
    pg_path = None
    if preset == 'macos':
        pg_path = ROOT_DIR / "projectGenerator/tools/projectGenerator/bin/projectGenerator.app/Contents/MacOS/projectGenerator"
    elif preset == 'windows':
        pg_path = ROOT_DIR / "projectGenerator/tools/projectGenerator/bin/projectGenerator.exe"
    else: # linux
        pg_path = ROOT_DIR / "projectGenerator/tools/projectGenerator/bin/projectGenerator"

    if not pg_path.exists():
        Colors.print(f"ProjectGenerator not found at: {pg_path}", Colors.RED)
        Colors.print("Please build projectGenerator first.", Colors.YELLOW)
        sys.exit(1)
    
    return pg_path

def find_examples():
    examples = []
    
    # 1. Core examples in examples/
    # Look for directories containing 'src'
    # Exclude templates, tools, build dirs, bin, etc.
    skip_dirs = {'templates', 'tools', 'build', 'bin', 'emscripten', 'CMakeFiles', '.git'}
    
    for root, dirs, files in os.walk(SCRIPT_DIR):
        # Modify dirs in-place to skip unwanted directories
        dirs[:] = [d for d in dirs if d not in skip_dirs and not d.startswith('build-')]
        
        if 'src' in dirs:
            path = Path(root)
            # Double check it's not in an excluded path
            rel_parts = path.relative_to(SCRIPT_DIR).parts
            if not any(p in skip_dirs for p in rel_parts):
                examples.append(path)

    # 2. Addon examples in addons/
    # Look for addons/*/example-*
    addons_dir = ROOT_DIR / "addons"
    if addons_dir.exists():
        # Iterate over addon directories (depth 1)
        for addon in addons_dir.iterdir():
            if addon.is_dir():
                # Iterate over example directories (depth 2)
                for example in addon.iterdir():
                    if example.is_dir() and example.name.startswith("example-") and \
                       not example.name.startswith("build") and example.name != "bin":
                        examples.append(example)

    # Remove duplicates and sort
    return sorted(list(set(examples)))

def run_command(cmd, cwd, verbose=False):
    """Run a shell command. Returns True if successful."""
    try:
        if verbose:
            subprocess.run(cmd, cwd=cwd, check=True)
        else:
            # Capture output and only show on error
            subprocess.run(cmd, cwd=cwd, check=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        return True
    except subprocess.CalledProcessError as e:
        if not verbose:
            # Show the captured output if it failed (and wasn't already shown)
            if e.stdout:
                print(e.stdout.decode('utf-8', errors='replace'))
        return False

# =============================================================================
# Main
# =============================================================================

def main():
    parser = argparse.ArgumentParser(description="TrussC Batch Build Script")
    parser.add_argument('--clean', action='store_true', help="Clean build directories before building")
    parser.add_argument('--web', action='store_true', help="Also build for WebAssembly")
    parser.add_argument('--web-only', action='store_true', help="Build for WebAssembly only (skip native build)")
    parser.add_argument('--verbose', action='store_true', help="Show detailed build output")
    args = parser.parse_args()

    if args.web_only:
        args.web = True

    preset = detect_platform_preset()
    pg_bin = find_project_generator(preset)
    
    Colors.print("=== TrussC Examples Batch Build (Python) ===", Colors.BLUE)
    Colors.print(f"Platform: {preset}", Colors.YELLOW)
    Colors.print(f"ProjectGenerator: {pg_bin}", Colors.YELLOW)
    if args.web_only:
        Colors.print("Mode: Web Only", Colors.YELLOW)
    print("")

    example_dirs = find_examples()
    
    if not example_dirs:
        Colors.print("No example directories found!", Colors.RED)
        sys.exit(1)

    total = len(example_dirs)
    print(f"Found {total} examples")
    print("")

    # Clean TrussC shared build if requested
    if args.clean:
        trussc_dir = ROOT_DIR / "trussc"
        Colors.print("Cleaning TrussC shared build...", Colors.YELLOW)
        if not args.web_only:
            shared_build = trussc_dir / f"build-{preset}"
            if shared_build.exists():
                shutil.rmtree(shared_build)
        
        if args.web:
            web_build = trussc_dir / "build-web"
            if web_build.exists():
                shutil.rmtree(web_build)

    success_count = 0
    failed_count = 0
    failed_list = []

    for i, example_dir in enumerate(example_dirs):
        # Calculate relative name for display
        try:
            example_name = example_dir.relative_to(ROOT_DIR)
        except ValueError:
            example_name = example_dir # Fallback

        Colors.print(f"[{i+1}/{total}] Updating & Building: {example_name}", Colors.YELLOW)

        # ---------------------------------------------------------
        # 1. Update Project
        # ---------------------------------------------------------
        pg_cmd = [str(pg_bin), "--update", str(example_dir), "--tc-root", str(ROOT_DIR)]
        if args.web:
            pg_cmd.append("--web")
        
        if not run_command(pg_cmd, cwd=ROOT_DIR, verbose=args.verbose):
            Colors.print(f"  Project update failed!", Colors.RED)
            failed_count += 1
            failed_list.append(f"{example_name} (update)")
            continue

        # ---------------------------------------------------------
        # 2. Build Native
        # ---------------------------------------------------------
        if not args.web_only:
            build_dir = example_dir / f"build-{preset}"
            if args.clean and build_dir.exists():
                shutil.rmtree(build_dir)

            # Configure
            if not run_command(["cmake", "--preset", preset], cwd=example_dir, verbose=args.verbose):
                Colors.print(f"  Native Configure failed!", Colors.RED)
                failed_count += 1
                failed_list.append(f"{example_name} (native-conf)")
                continue

            # Build
            if not run_command(["cmake", "--build", "--preset", preset], cwd=example_dir, verbose=args.verbose):
                Colors.print(f"  Native Build failed!", Colors.RED)
                failed_count += 1
                failed_list.append(f"{example_name} (native-build)")
                continue

        # ---------------------------------------------------------
        # 3. Build Web (Optional)
        # ---------------------------------------------------------
        if args.web:
            # ProjectGenerator has already updated presets and scripts
            build_dir_web = example_dir / "build-web"
            if args.clean and build_dir_web.exists():
                shutil.rmtree(build_dir_web)

            # Check for cmake (we assume environment is set up if not using emcmake wrapper explicitly)
            # The preset 'web' handles the toolchain file.
            
            # Configure
            if not run_command(["cmake", "--preset", "web"], cwd=example_dir, verbose=args.verbose):
                Colors.print(f"  Web Configure failed!", Colors.RED)
                failed_count += 1
                failed_list.append(f"{example_name} (web-conf)")
                continue

            # Build
            if not run_command(["cmake", "--build", "--preset", "web"], cwd=example_dir, verbose=args.verbose):
                Colors.print(f"  Web Build failed!", Colors.RED)
                failed_count += 1
                failed_list.append(f"{example_name} (web-build)")
                continue

        Colors.print("  Success!", Colors.GREEN)
        success_count += 1

    # ---------------------------------------------------------
    # Summary
    # ---------------------------------------------------------
    print("")
    Colors.print("=== Build Summary ===", Colors.BLUE)
    print(f"Total:   {total}")
    Colors.print(f"Success: {success_count}", Colors.GREEN)
    
    if failed_count > 0:
        Colors.print(f"Failed:  {failed_count}", Colors.RED)
        print("")
        Colors.print("Failed examples:", Colors.RED)
        for fail in failed_list:
            print(f"  - {fail}")
        sys.exit(1)
    else:
        Colors.print(f"Failed:  0", Colors.ENDC)
        print("")
        Colors.print("All examples built successfully!", Colors.GREEN)
        sys.exit(0)

if __name__ == "__main__":
    main()
