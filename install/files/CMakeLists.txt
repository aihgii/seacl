install (FILES       cfg/${PROJECT_NAME}.conf
         DESTINATION ${CONFIG_PATH})
install (FILES       cfg/mysql.conf
         DESTINATION ${CONFIG_PATH}
         PERMISSIONS OWNER_READ
                     OWNER_WRITE)
