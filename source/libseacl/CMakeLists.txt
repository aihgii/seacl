cmake_minimum_required(VERSION 2.8)
project(${TARGET_1})

include (CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" HAS_CXX11)
if(NOT HAS_CXX11)
  message (FATAL_ERROR "Current compiler does not suppport -std=c++11 option")
endif()
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
add_definitions (${CMAKE_CXX_FLAGS})

find_package (MySQL++ REQUIRED)

set (SRC acl.cpp)

include_directories (${CMAKE_INCLUDE_PATH}
                     ${MySQL++_INCLUDE_DIRS})
add_library (${PROJECT_NAME} SHARED ${SRC})
target_link_libraries (${PROJECT_NAME} ${MySQL++_LIBRARIES})
install (TARGETS ${PROJECT_NAME}
         DESTINATION ${LIB_INSTALL_PATH})
