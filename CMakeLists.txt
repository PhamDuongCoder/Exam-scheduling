cmake_minimum_required(VERSION 3.10)
project(ExamScheduling)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directory
include_directories(${PROJECT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.cpp
    src/problem.cpp
    src/greedy_solver.cpp
    src/sa_solver.cpp
)

# Executable
add_executable(exam_scheduler ${SOURCES})

# Enable compiler warnings
if(MSVC)
    target_compile_options(exam_scheduler PRIVATE /W4)
else()
    target_compile_options(exam_scheduler PRIVATE -Wall -Wextra -Wpedantic)
endif()
