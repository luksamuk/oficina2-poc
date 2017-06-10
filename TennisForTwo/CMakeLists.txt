cmake_minimum_required(VERSION 2.8)
project(tennis-for-two)

# Retrieve C++ flags and linking libraries
execute_process(COMMAND oficina2-config --cppflags
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE OFICINA2_CFG_CPPFLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND oficina2-config --libs
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE OFICINA2_CFG_LIBS
  OUTPUT_STRIP_TRAILING_WHITESPACE)
# Strip trailing whitespaces so CMake doesn't
# trigger an error
string(STRIP ${OFICINA2_CFG_CPPFLAGS} OFICINA2_CFG_CPPFLAGS)
string(STRIP ${OFICINA2_CFG_LIBS} OFICINA2_CFG_LIBS)

# Configure build type and feed flags to C++
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OFICINA2_CFG_CPPFLAGS}")

# Define game sources
set(T42_SOURCES
  src/main.cpp)

# Define game executable
add_executable(t42 ${T42_SOURCES})
# Link with Oficina2 libraries
TARGET_LINK_LIBRARIES(t42 ${OFICINA2_CFG_LIBS})
