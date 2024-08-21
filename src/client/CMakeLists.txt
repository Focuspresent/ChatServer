# 这里是client子目录

# 添加源文件
aux_source_directory(. SRC_LIST)

# 添加可执行文件
add_executable(ChatClient ${SRC_LIST})

# 添加头文件
target_include_directories(ChatClient PRIVATE ${PROJECT_SOURCE_DIR}/thirdparty/jsoncpp)

target_include_directories(ChatClient PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_include_directories(ChatClient PRIVATE ${PROJECT_SOURCE_DIR}/include/client)
target_include_directories(ChatClient PRIVATE ${PROJECT_SOURCE_DIR}/include/server/model)

# 链接库
target_link_libraries(ChatClient pthread)