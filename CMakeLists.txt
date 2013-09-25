cmake_minimum_required(VERSION 2.8)
project(seacl) 

message ("Starting cmake")

include (config.h.in)

set (TARGET_1 seacl)
set (TARGET_2 users)
set (TARGET_3 auth)
set (TARGET_4 userlist)

add_subdirectory (install/pre)
add_subdirectory (${${PROJECT_NAME}_SOURCE_DIR}/lib${TARGET_1})
add_subdirectory (${${PROJECT_NAME}_SOURCE_DIR}/${TARGET_2})
add_subdirectory (${${PROJECT_NAME}_SOURCE_DIR}/${TARGET_3})
add_subdirectory (${${PROJECT_NAME}_SOURCE_DIR}/${TARGET_4})
add_subdirectory (install/files)
add_subdirectory (install/post)
