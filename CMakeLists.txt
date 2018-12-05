cmake_minimum_required(VERSION 2.8)
project(DeSyDe)

# Add all specific gecode libraries
include(ExternalProject)
ExternalProject_Add(gecode
  GIT_REPOSITORY    https://github.com/Gecode/gecode.git
  GIT_PROGRESS      TRUE
  # GIT_TAG           origin/1e8c55c # commit tag for release 5.1.0
  COMMAND "${GIT_EXECUTABLE} checkout 1e8c55c"
  CONFIGURE_COMMAND ${CMAKE_CURRENT_BINARY_DIR}/configure
  BUILD_COMMAND ${MAKE}
)

# include gecode
# include_directories(${PROJECT_SOURCE_DIR}/gecode)
# add_library(gecode_int SHARED ${gecode_int_srcs} ${gecode_int_heads})

add_subdirectory(src)

