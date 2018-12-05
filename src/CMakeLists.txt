cmake_minimum_required (VERSION 2.8)
project (DeSyDe)

# get boost dep
find_package(Boost REQUIRED COMPONENTS graph)
find_package(LibXml2 REQUIRED)

# this is a lazy hack for the moment, ideally every single source
# file should be added to his compiling script.
file(GLOB SRCS **.cpp)
file(GLOB HEADS **.hpp)


include_directories(${CMAKE_SOURCE_DIR})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${LIBXML2_INCLUDE_DIRS})
add_executable(adse adse.cpp ${SRCS})
target_link_libraries(adse ${Boost_GRAPH_LIBRARY} ${LIBXML2_LIBRARY})
