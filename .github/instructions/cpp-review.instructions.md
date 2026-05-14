---
description: "Use when reviewing, editing, or writing C++ source files in this project. Focuses on best practices, performance, maintainability, and project-specific conventions."
name: "C++ Code Review Guidelines"
applyTo: ["**/*.cpp", "**/*.h", "**/*.hpp"]
---
# C++ Code Guidelines for mfree_iwf Project

## Memory Management
- Prefer smart pointers (`std::unique_ptr`, `std::shared_ptr`) over raw pointers for owned resources
- Avoid global variables; use dependency injection instead
- Ensure RAII principles are followed for resource management

## Code Structure
- Keep functions under 50 lines; extract complex logic into smaller functions
- Use encapsulation: make data members private with accessors when needed
- Avoid monolithic main functions; separate concerns into classes/methods

## Performance
- Use OpenMP pragmas consistently for parallelizable loops
- Minimize data copying; use move semantics where appropriate
- Profile and optimize bottlenecks, especially in simulation loops

## Best Practices
- Replace macros with `constexpr` constants or enums
- Use C++17 features (e.g., `std::optional`, structured bindings)
- Add unit tests for core classes and functions
- Document TODOs and resolve them promptly

## Project-Specific
- Follow the existing naming conventions (e.g., `m_` for member variables)
- Ensure compatibility with CMake build system
- Validate changes against existing tests before committing
