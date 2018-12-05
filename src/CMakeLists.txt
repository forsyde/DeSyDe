# WARNING!! THIS FILE SHOULD NOT BE USED INDEPEDENTLY
# The intent is that the CMakeLists.txt on the top level
# project dir invokes this script.

# get boost dep
find_package(Boost REQUIRED COMPONENTS graph)
find_package(LibXml2 REQUIRED)

# this is a lazy hack for the moment, ideally every single source
# file should be added to his compiling script.
file(GLOB SRCS **.cpp)
file(GLOB HEADS **.hpp)


# include the actual src dir
include_directories(${CMAKE_SOURCE_DIR})
# include boost files
include_directories(${Boost_INCLUDE_DIRS})
# include libxml files
include_directories(${LIBXML2_INCLUDE_DIRS})
# include gecode files, assuming they have been compiled
include_directories(${gecode_INCLUDE_DIRS})

add_executable(adse adse.cpp ${SRCS})
target_link_libraries(adse ${Boost_GRAPH_LIBRARY} ${LIBXML2_LIBRARY} ${gecode_LIBRARY})
