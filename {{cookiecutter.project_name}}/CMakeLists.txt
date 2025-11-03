cmake_minimum_required(VERSION 3.20.3)
project({{ cookiecutter.project_name }})

enable_language(C)
enable_language(CXX)

set(CMAKE_C_STANDARD {{cookiecutter.c_standard}})
{% if cookiecutter.enable_cxx_support == "y" -%}
set(CMAKE_CXX_STANDARD {{cookiecutter.cxx_standard}})
{% else -%}
# set(CMAKE_CXX_STANDARD {{cookiecutter.cxx_standard}})
{%- endif %}

option(USE_MPI "Enable MPI support" OFF)
option(USE_OPENMP "Enable OpenMP support" OFF)
option(REMOTE_RUN "Configurations for remote run" OFF)



# Include dependencies
include({{cookiecutter._cmake_dir}}/utils.cmake)
include({{cookiecutter._cmake_dir}}/ExternalDependencies.cmake)
include({{cookiecutter._cmake_dir}}/PrintDebug.cmake)

add_subdirectory({{cookiecutter._src_dir}}/benchmark)

add_executable(${PROJECT_NAME} {{cookiecutter._src_dir}}/main.cpp)

if (REMOTE_RUN)
    add_compile_definitions(HPC_RUN)
endif ()

if(USE_MPI)
    find_and_link_mpi_for_target(${PROJECT_NAME})
endif()

if(USE_OPENMP)
    find_and_link_openmp_for_target(${PROJECT_NAME})
endif ()


target_link_libraries(${PROJECT_NAME} PRIVATE print_debug)
target_link_libraries(${PROJECT_NAME} PRIVATE benchmark)


#================================ SET PRINT_DEBUG_LEVEL per module ==========================================

#------------- CUSTOM PRINT DEBUG LEVELS (for console print) -----------------
#[[
 Available log levels - set PRINT_DEBUG_LEVEL to one of:
    PRINT_DEBUG_LEVEL_NONE   → disable all logs
    PRINT_DEBUG_LEVEL_ERROR  → show only errors
    PRINT_DEBUG_LEVEL_WARN   → show warnings and errors
    PRINT_DEBUG_LEVEL_INFO   → show info, warnings, and errors
]]

#------------------------------------------------
# Global debug level (used as default for all modules)
set(PRINT_DEBUG_LEVEL_GLOBAL ${PRINT_DEBUG_LEVEL_INFO} CACHE STRING "Global debug level for all modules")

# Per-module debug levels (use global if not explicitly set)
set(PRINT_DEBUG_LEVEL_MAIN ${PRINT_DEBUG_LEVEL_GLOBAL} CACHE STRING "Debug level for main")
set(PRINT_DEBUG_LEVEL_BENCHMARK ${PRINT_DEBUG_LEVEL_GLOBAL} CACHE STRING "Debug level for benchmark")
#------------------------------------------------

define_print_debug_level_for_target(${PROJECT_NAME} ${PRINT_DEBUG_LEVEL_MAIN})
define_print_debug_level_for_target(benchmark ${PRINT_DEBUG_LEVEL_BENCHMARK})

#================================================================================================

