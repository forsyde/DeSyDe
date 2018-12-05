cmake_minimum_required(VERSION 2.8)
project(DeSyDe)

# Add all specific gecode libraries
include(ExternalProject)
ExternalProject_Add(gecode
  GIT_REPOSITORY    https://github.com/Gecode/gecode.git
  GIT_TAG           release-5.1.0 # commit tag for release 5.1.0
  SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/gecode
  BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/gecode
  CONFIGURE_COMMAND ${CMAKE_CURRENT_BINARY_DIR}/gecode/configure
  UPDATE_COMMAND ""
  BUILD_COMMAND ${MAKE}
  INSTALL_COMMAND ""
)

# include gecode
# include_directories(${PROJECT_SOURCE_DIR}/gecode)
# add_library(gecode_int SHARED ${gecode_int_srcs} ${gecode_int_heads})

add_subdirectory(src)

