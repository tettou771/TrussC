import os
import subprocess
import sys
import glob
import time

def find_project_generator(root_dir):
    pg_path = os.path.join(root_dir, "projectGenerator", "tools", "projectGenerator", "bin", "projectGenerator.exe")
    if os.path.exists(pg_path):
        return pg_path
    print(f"ProjectGenerator not found at: {pg_path}")
    sys.exit(1)

def find_examples(root_dir):
    examples_dir = os.path.join(root_dir, "examples")
    addons_dir = os.path.join(root_dir, "addons")
    
    example_paths = []
    
    # Core examples
    for root, dirs, files in os.walk(examples_dir):
        if "src" in dirs:
            # Exclude templates, tools, build, bin, etc.
            if "templates" in root or "tools" in root or "build" in root or "bin" in root:
                continue
            example_paths.append(root)

    # Addon examples
    if os.path.exists(addons_dir):
        for addon in os.listdir(addons_dir):
            addon_path = os.path.join(addons_dir, addon)
            if os.path.isdir(addon_path):
                for item in os.listdir(addon_path):
                    if item.startswith("example-"):
                        example_path = os.path.join(addon_path, item)
                        if os.path.exists(os.path.join(example_path, "src")):
                            example_paths.append(example_path)

    return sorted(list(set(example_paths)))

def update_projects(pg_bin, examples, root_dir):
    print("Updating projects...")
    count = 0
    total = len(examples)
    for example in examples:
        count += 1
        print(f"[{count}/{total}] Updating: {os.path.relpath(example, root_dir)}")
        cmd = [pg_bin, "--update", example, "--tc-root", root_dir]
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode != 0:
            print(f"Update failed for {example}")
            print(result.stdout)
            print(result.stderr)
            sys.exit(1)

def generate_root_cmake(examples_dir, examples, root_dir):
    print("Generating root CMakeLists.txt...")
    root_cmake_path = os.path.join(examples_dir, "CMakeLists.txt")
    
    with open(root_cmake_path, "w") as f:
        f.write("cmake_minimum_required(VERSION 3.20)\n")
        f.write("project(TrussC_All_Examples)\n\n")
        trussc_dir_path = os.path.join(examples_dir, "../trussc").replace(os.sep, "/")
        f.write(f'set(TRUSSC_DIR "{trussc_dir_path}")\n')
        
        f.write('if(WIN32)\n')
        f.write('    set(_TC_BUILD_DIR "${TRUSSC_DIR}/build-windows")\n')
        f.write('else()\n')
        f.write('    set(_TC_BUILD_DIR "${TRUSSC_DIR}/build-linux")\n')
        f.write('endif()\n\n')
        
        f.write('if(NOT TARGET TrussC)\n')
        f.write('    add_subdirectory(${TRUSSC_DIR} ${_TC_BUILD_DIR})\n')
        f.write('endif()\n\n')
        
        for example in examples:
            rel_path = os.path.relpath(example, examples_dir).replace(os.sep, "/")
            if rel_path.startswith("../"):
                bin_name = rel_path.replace("../", "").replace("/", "_")
                f.write(f'add_subdirectory("{rel_path}" "{bin_name}")\n')
            else:
                f.write(f'add_subdirectory("{rel_path}")\n')

def build_native(examples_dir):
    print("Starting Native Batch Build...")
    build_dir = os.path.join(examples_dir, "build-windows")
    os.makedirs(build_dir, exist_ok=True)
    
    print("Configuring...")
    cmd_config = ["cmake", "-S", examples_dir, "-B", build_dir]
    subprocess.check_call(cmd_config)
    
    print("Building...")
    # Use -j for parallel build
    cmd_build = ["cmake", "--build", build_dir, "--config", "Release", "-j", str(os.cpu_count())]
    subprocess.check_call(cmd_build)
    print("Native Batch Build Success!")

def main():
    start_time = time.time()
    root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    examples_dir = os.path.join(root_dir, "examples")
    
    pg_bin = find_project_generator(root_dir)
    examples = find_examples(root_dir)
    
    print(f"Found {len(examples)} examples")
    
    update_projects(pg_bin, examples, root_dir)
    generate_root_cmake(examples_dir, examples, root_dir)
    build_native(examples_dir)

    end_time = time.time()
    elapsed_time = end_time - start_time
    print(f"\nTotal execution time: {elapsed_time:.2f} seconds")

if __name__ == "__main__":
    main()