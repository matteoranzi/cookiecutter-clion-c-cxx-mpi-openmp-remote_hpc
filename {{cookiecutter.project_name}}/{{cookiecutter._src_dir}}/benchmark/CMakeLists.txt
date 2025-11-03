add_library(benchmark benchmark.c)

include(${CMAKE_SOURCE_DIR}/{{cookiecutter._cmake_dir}}/PrintDebug.cmake)
include(${CMAKE_SOURCE_DIR}/{{cookiecutter._cmake_dir}}/ExternalDependencies.cmake)


# Public headers
target_include_directories(benchmark PUBLIC ${CMAKE_SOURCE_DIR}/{{cookiecutter._include_dir}}/)

find_and_link_mpi_for_target(benchmark)
target_link_libraries(benchmark PUBLIC print_debug)