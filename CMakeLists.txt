cmake_minimum_required(VERSION 2.8)
project(seacl) 

message ("Starting cmake")

include (config.h.in)

set (TARGET_1 users)
set (TARGET_2 auth)
set (TARGET_3 userlist)

add_subdirectory (install/pre)
add_subdirectory (${${PROJECT_NAME}_SOURCE_DIR}/${TARGET_1})
add_subdirectory (${${PROJECT_NAME}_SOURCE_DIR}/${TARGET_2})
add_subdirectory (${${PROJECT_NAME}_SOURCE_DIR}/${TARGET_3})
add_subdirectory (install/files)
add_subdirectory (install/post)
