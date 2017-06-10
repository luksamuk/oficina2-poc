cmake_minimum_required(VERSION 2.8)
project(ofsonic-the-hedgehog)

#Retrieve C++ flags and linking libraries
execute_process(COMMAND oficina2-config --cppflags
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE OFICINA2_CPPFLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND oficina2-config --libs
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE OFICINA2_LIBS
  OUTPUT_STRIP_TRAILING_WHITESPACE)
string(STRIP ${OFICINA2_CPPFLAGS} OFICINA2_CPPFLAGS)
string(STRIP ${OFICINA2_LIBS} OFICINA2_LIBS)

IF(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
ENDIF()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OFICINA2_CPPFLAGS}")


set(OFSONIC_SOURCES
  src/GameScreen.cpp
  src/MenuScreen.cpp
  src/main.cpp)

set(OFSONIC_HEADERS
  src/GameScreen.hpp
  src/MenuScreen.hpp)

add_executable(SonicGame ${OFSONIC_SOURCES} ${OFSONIC_HEADERS})
TARGET_LINK_LIBRARIES(SonicGame ${OFICINA2_LIBS})
