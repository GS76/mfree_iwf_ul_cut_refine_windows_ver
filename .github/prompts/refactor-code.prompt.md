---
description: "Apply quick refactoring suggestions to selected C++ code in the project"
name: "Refactor Code"
argument-hint: "Describe the refactoring needed (e.g., extract method, replace raw pointers)"
agent: "agent"
tools: [read, edit, search]
---
Apply the specified refactoring to the selected C++ code, following the project's C++ guidelines and best practices.

## Steps
1. Analyze the selected code and understand the refactoring request
2. Ensure the change aligns with project conventions (e.g., smart pointers, encapsulation)
3. Make the refactoring change using appropriate C++ patterns
4. Verify the code still compiles and maintains functionality

## Output
- Show the refactored code
- Explain the changes made and their benefits
- Note any additional considerations