cmake_minimum_required(VERSION 2.8)
project(${TARGET_2}) 

find_package (MySQL++ REQUIRED)

set (SRC eui48.cpp)

include_directories (${MySQL++_INCLUDE_DIRS})
add_executable (${PROJECT_NAME} ${SRC})
target_link_libraries (${PROJECT_NAME} ${MySQL++_LIBRARIES})
install (TARGETS ${PROJECT_NAME}
         DESTINATION ${INSTALL_PATH}/acl)
