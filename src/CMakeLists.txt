cmake_minimum_required(VERSION 3.6)

project(mpeg-ts-split)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(OPT "-g")
endif()

set(CMAKE_CXX_FLAGS_NO_WALL "${CMAKE_CXX_FLAGS} ${OPT}")

if (CMAKE_COMPILER_IS_GNUCXX)
  set(WALL_WERR "")
else()
  set(WALL_WERR "-Wall -Werror")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_NO_WALL} ${WALL_WERR}")
set(CMAKE_C_FLAGS "${OPT}")

if(APPLE)
  set(CMAKE_SKIP_BUILD_RPATH  TRUE)
  set(CMAKE_MACOSX_RPATH 1)
  set(CMAKE_INSTALL_RPATH "@loader_path/lib")
endif()

if(APPLE)
    set(LIBRARY_SUFFIX .dylib )
else()
    set(LIBRARY_SUFFIX .so )
endif()

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")


message("-- CMAKE_C_COMPILER: ${CMAKE_C_COMPILER}")
message("-- CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}")
message("-- CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
message("-- CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message("-- CMAKE_CXX_FLAGS_NO_WALL: ${CMAKE_CXX_FLAGS_NO_WALL}")
message("-- CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")

file (GLOB SRC *.cpp)
add_subdirectory(3rdparty/loguru)
add_executable(${PROJECT_NAME} ${SRC})

set (CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/cmake)

# find_package(Boost)
if (Boost_FOUND)
    message("-- Using system boost")
else()
    include(setup_boost_headeronly)
    add_dependencies(${PROJECT_NAME} external_boost)
endif()

# find_package(ffmpeg REQUIRED)

if (AVCODEC_INCLUDE_DIRS AND AVDEVICE_INCLUDE_DIRS AND AVFORMAT_INCLUDE_DIRS AND AVFILTER_INCLUDE_DIRS AND AVUTIL_INCLUDE_DIRS AND SWSCALE_INCLUDE_DIRS)
  message("-- Using system ffmpeg")
else()
  include(setup_ffmpeg)
  add_dependencies(${PROJECT_NAME} external_ffmpeg)
endif()


target_include_directories(${PROJECT_NAME} PRIVATE
                              ${Boost_INCLUDE_DIR}
                              ${FFMPEG_INCLUDE_DIRS}
                              ${LIBKLV_INCLUDE_DIR}
                              ${CMAKE_SOURCE_DIR}/3rdparty/cxxopts)

target_link_libraries(${PROJECT_NAME}
                      ${FFMPEG_LIBRARIES}
                      loguru
                      -lpthread
                      -ldl
                      -lc++abi)

install (DIRECTORY ${BOOST_LIB_DIR}
         DESTINATION ${CMAKE_BINARY_DIR}/bin
         DIRECTORY_PERMISSIONS USE_SOURCE_PERMISSIONS
         FILES_MATCHING PATTERN "*${LIBRARY_SUFFIX}")

install (DIRECTORY ${FFMPEG_LIB_DIR}
         DESTINATION ${CMAKE_BINARY_DIR}/bin
         DIRECTORY_PERMISSIONS USE_SOURCE_PERMISSIONS
         FILES_MATCHING PATTERN "*.${LIBRARY_SUFFIX}")

install (TARGETS ${PROJECT_NAME}
         DESTINATION ${CMAKE_BINARY_DIR}/bin)

install (FILES ${CMAKE_SOURCE_DIR}/install_libs.sh
         PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
         DESTINATION ${CMAKE_BINARY_DIR}/bin)
