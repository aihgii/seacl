include (FindPackageHandleStandardArgs)

if(MYSQLPP_INCLUDE_DIRS AND MYSQLPP_LIBRARIES)
  set (MySQL++_FIND_QUIETLY TRUE)
endif(MYSQLPP_INCLUDE_DIRS AND MYSQLPP_LIBRARIES)

find_path (MYSQLPP_INCLUDE_PATH mysql++.h /usr/include/mysql++/)

find_path (MYSQL_INCLUDE_PATH mysql.h /usr/include/mysql/)
if(MYSQL_INCLUDE_PATH AND MYSQLPP_INCLUDE_PATH)
  set (MySQL++_INCLUDE_DIRS
     ${MYSQLPP_INCLUDE_PATH}
     ${MYSQL_INCLUDE_PATH}
  )
endif(MYSQL_INCLUDE_PATH AND MYSQLPP_INCLUDE_PATH)

find_library (MySQL++_LIBRARIES mysqlpp)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(MySQL++ DEFAULT_MSG MySQL++_LIBRARIES MySQL++_INCLUDE_DIRS)

mark_as_advanced (MySQL++_LIBRARIES MySQL++_INCLUDE_DIRS)