cmake_minimum_required(VERSION 3.6)

project(mpeg-ts-demux)

# For copy elision guarantee, can be downgraded to 14, optimization/standard tradeoff
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
else()
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif()

file (GLOB_RECURSE SRC *.cpp)

add_executable(${PROJECT_NAME} ${SRC})

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS   OFF)
set(Boost_USE_STATIC_RUNTIME OFF)
add_definitions(-DBOOST_ALL_DYN_LINK)

find_package(Boost 1.66.0 REQUIRED COMPONENTS system filesystem program_options log log_setup)

message("-- CMAKE_C_COMPILER: ${CMAKE_C_COMPILER}")
message("-- CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}")
message("-- CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
message("-- CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message("-- CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")
message("-- Boost Include Dir: ${Boost_INCLUDE_DIR}")
message("-- Boost Libraries Used: ${Boost_LIBRARIES}")

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR} ${Boost_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE ${Boost_LIBRARIES})

