# 设置cmake的版本
cmake_minimum_required(VERSION 3.4.1)

# 导入头文件
include_directories(${CMAKE_SOURCE_DIR}/src/main/include)

# 搜集指定路径下的源文件的文件名，将输出结果列表储存在指定的变量中
aux_source_directory(${CMAKE_SOURCE_DIR}/src/main/cpp cpp_src)

# 将指定的源文件生成目标文件
add_library(webrtc_ns SHARED ${cpp_src})

# 设置编译选项
# -fvisibility=hidden : don't export all symbols
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

# 找到需要链接的系统库
find_library(log-lib log) # for logging

# 将目标文件与外部库文件进行链接
target_link_libraries(webrtc_ns
        ${log-lib})
