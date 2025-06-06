# CMake Generated Files Best Practices

## Key Concepts

### 1. OUTPUT vs BYPRODUCTS
- **OUTPUT**: Primary files that targets depend on. Must be absolute paths in CMAKE_BINARY_DIR.
- **BYPRODUCTS**: Additional files created but not primary outputs. Used by Ninja generator for proper dependency tracking.

### 2. Directory Creation
Always create output directories before generating files:
```cmake
COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/path/to/output
```

### 3. Absolute Paths
Use `CMAKE_BINARY_DIR` for all generated file paths:
```cmake
OUTPUT ${CMAKE_BINARY_DIR}/gen/file.hpp
```

### 4. VERBATIM
Always use `VERBATIM` to properly escape command arguments.

## Example: Proper Generated File Setup

```cmake
# Define source files
list(APPEND IDL_FILES_LIST 
  idl/example.npidl 
  idl/another.npidl
)

# Generate variables for outputs
set(GENERATED_HEADERS)
set(GENERATED_SOURCES)
set(GENERATED_TS_FILES)

foreach(file ${IDL_FILES_LIST})
  get_filename_component(basename ${file} NAME_WE)
  
  set(gen_hpp ${CMAKE_BINARY_DIR}/gen/${basename}.hpp)
  set(gen_cpp ${CMAKE_BINARY_DIR}/gen/${basename}.cpp)
  set(gen_ts ${CMAKE_BINARY_DIR}/gen/js/${basename}.ts)
  
  list(APPEND GENERATED_HEADERS ${gen_hpp})
  list(APPEND GENERATED_SOURCES ${gen_cpp})
  list(APPEND GENERATED_TS_FILES ${gen_ts})
  
  add_custom_command(
    OUTPUT ${gen_hpp} ${gen_cpp}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/gen
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/gen/js
    COMMAND your_generator
      --out-hpp ${CMAKE_BINARY_DIR}/gen
      --out-cpp ${CMAKE_BINARY_DIR}/gen
      --out-ts ${CMAKE_BINARY_DIR}/gen/js
      ${CMAKE_SOURCE_DIR}/${file}
    DEPENDS ${file} your_generator
    BYPRODUCTS ${gen_ts}
    COMMENT "Generating code from ${file}"
    VERBATIM
  )
endforeach()

# Create generation target
add_custom_target(generate_code
  DEPENDS ${GENERATED_HEADERS} ${GENERATED_SOURCES}
  COMMENT "Generating all code files"
)

# Create library using generated files
add_library(mylib STATIC
  ${GENERATED_SOURCES}
  ${GENERATED_HEADERS}
  # Add regular source files here
  src/regular_file.cpp
)

# Ensure generation happens before compilation
add_dependencies(mylib generate_code)

# Include directories
target_include_directories(mylib
  PUBLIC 
    ${CMAKE_BINARY_DIR}/gen  # For generated headers
    include                  # For regular headers
)
```

## Common Pitfalls

### 1. Mixing relative and absolute paths
❌ Wrong:
```cmake
OUTPUT ${CMAKE_BINARY_DIR}/gen/file.hpp
COMMAND tool --out gen  # Relative path
```

✅ Correct:
```cmake
OUTPUT ${CMAKE_BINARY_DIR}/gen/file.hpp
COMMAND tool --out ${CMAKE_BINARY_DIR}/gen  # Absolute path
```

### 2. Not creating directories
❌ Wrong:
```cmake
COMMAND tool --out ${CMAKE_BINARY_DIR}/gen/subdir
# Directory might not exist
```

✅ Correct:
```cmake
COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/gen/subdir
COMMAND tool --out ${CMAKE_BINARY_DIR}/gen/subdir
```

### 3. Missing BYPRODUCTS for additional files
❌ Wrong:
```cmake
# Tool generates .hpp, .cpp, and .ts files but only .hpp and .cpp in OUTPUT
OUTPUT file.hpp file.cpp
COMMAND tool --generates-three-files
```

✅ Correct:
```cmake
OUTPUT file.hpp file.cpp
BYPRODUCTS file.ts  # Additional file for Ninja
COMMAND tool --generates-three-files
```

## Advanced: Source Groups for IDE Organization

```cmake
# Organize generated files in IDE
source_group("Generated\\Headers" FILES ${GENERATED_HEADERS})
source_group("Generated\\Sources" FILES ${GENERATED_SOURCES})

# Mark files as generated (automatic for OUTPUT files)
set_source_files_properties(${GENERATED_SOURCES} ${GENERATED_HEADERS}
  PROPERTIES GENERATED TRUE
)
```

## Debugging Generated Files

1. Check if files are actually generated:
```bash
cmake --build . --target generate_code --verbose
```

2. Verify OUTPUT paths are correct:
```cmake
message(STATUS "Generated headers: ${GENERATED_HEADERS}")
```

3. Use `file(GLOB)` to verify files exist (for debugging only):
```cmake
file(GLOB_RECURSE found_files "${CMAKE_BINARY_DIR}/gen/*")
message(STATUS "Found generated files: ${found_files}")
```
