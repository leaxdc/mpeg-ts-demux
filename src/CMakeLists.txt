cmake_minimum_required(VERSION 3.6)

project(mpeg-ts-demux CXX)
set(CMAKE_CXX_COMPILER_EXTENSION OFF)

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS   OFF)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.66.0 REQUIRED COMPONENTS system filesystem program_options log log_setup)

file (GLOB_RECURSE SRC *.cpp)
add_executable(${PROJECT_NAME} ${SRC})

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_compile_options(${PROJECT_NAME} PRIVATE  -Wall -Werror -Wpedantic)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR} ${Boost_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Boost_LIBRARIES})
target_compile_definitions(${PROJECT_NAME} PRIVATE BOOST_ALL_DYN_LINK)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_options(${PROJECT_NAME} PRIVATE -g -O0)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -O3)
endif()

message("-- CMAKE_C_COMPILER: ${CMAKE_C_COMPILER}")
message("-- CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}")
message("-- CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
message("-- CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message("-- CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")
message("-- Boost Include Dir: ${Boost_INCLUDE_DIR}")
message("-- Boost Libraries Used: ${Boost_LIBRARIES}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
