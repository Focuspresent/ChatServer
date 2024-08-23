# 这里是server子目录

# 添加源文件
aux_source_directory(. SRC_LIST)
aux_source_directory(./db DB_LIST)
aux_source_directory(./model MODEL_LIST)

# 添加可执行文件
add_executable(ChatServer ${SRC_LIST} ${DB_LIST} ${MODEL_LIST})

# 添加头文件搜索路径
target_include_directories(ChatServer PRIVATE ${PROJECT_SOURCE_DIR}/thirdparty/jsoncpp)

target_include_directories(ChatServer PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_include_directories(ChatServer PRIVATE ${PROJECT_SOURCE_DIR}/include/server)
target_include_directories(ChatServer PRIVATE ${PROJECT_SOURCE_DIR}/include/server/db)
target_include_directories(ChatServer PRIVATE ${PROJECT_SOURCE_DIR}/include/server/model)

# 链接库
target_link_libraries(ChatServer muduo_net muduo_base pthread mysqlclient hiredis)