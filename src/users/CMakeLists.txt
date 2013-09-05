cmake_minimum_required(VERSION 2.8)
project(${TARGET_1})

include (CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" HAS_CXX11)
if(NOT HAS_CXX11)
  message (FATAL_ERROR "Current compiler does not suppport -std=c++11 option")
endif()
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
add_definitions (${CMAKE_CXX_FLAGS})

set (Boost_USE_STATIC_LIBS OFF)
set (Boost_USE_MULTITHREADED ON)
find_package (Boost COMPONENTS regex REQUIRED)
find_package (MySQL++ REQUIRED)

set (SRC users.cpp acl.cpp eui48_t.cpp)

include_directories (${Boost_INCLUDE_DIRS}
                     ${MySQL++_INCLUDE_DIRS})
add_executable (${PROJECT_NAME} ${SRC})
target_link_libraries (${PROJECT_NAME} ${Boost_LIBRARIES} ${MySQL++_LIBRARIES})
install (TARGETS ${PROJECT_NAME}
         DESTINATION ${INSTALL_PATH})
