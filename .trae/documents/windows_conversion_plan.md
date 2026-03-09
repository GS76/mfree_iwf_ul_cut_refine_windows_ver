# Windows Conversion Plan

This plan outlines the steps to convert the Linux-specific codebase to be fully compatible with Windows.

## 1. Build System Modernization
- [ ] Create a `CMakeLists.txt` file to replace the existing Makefiles. This ensures cross-platform build support (Windows/Linux/macOS).
    - Define project and version.
    - Specify C++ standard (C++17 recommended for `std::filesystem`).
    - Add source files.
    - Handle include directories.

## 2. Code Adaptation
- [ ] **Header Management**:
    - Identify files including `<sys/time.h>`, `<unistd.h>`, `<sys/stat.h>`, `<sys/types.h>`.
    - Replace POSIX headers with C++ standard headers (`<chrono>`, `<filesystem>`) or Windows equivalents where necessary.
- [ ] **File System Operations**:
    - Replace `mkdir(folder, 0777)` with `std::filesystem::create_directory` or `_mkdir` (Windows specific).
    - Replace `system("rm ...")` calls with `std::filesystem::remove` or `std::filesystem::remove_all`.
    - Fix path separators in string literals (e.g., `"%s/out_%06d.vtk"`) to use `std::filesystem::path` or compatible separators.
- [ ] **Timing Functions**:
    - Replace `gettimeofday` and `struct timeval` with `std::chrono::high_resolution_clock`.

## 3. Verification
- [ ] Configure the project using CMake.
- [ ] Build the project on Windows.
- [ ] Run the executable to verify functionality (file creation, output generation).
