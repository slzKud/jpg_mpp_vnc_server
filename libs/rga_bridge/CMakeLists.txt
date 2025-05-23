# 创建RGA桥接库
add_library(rga_bridge STATIC
    rga_bridge.cpp
    rga_bridge_wrapper.cpp
)
# 包含目录
target_include_directories(rga_bridge PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${RGA_INC}
)

# link_directories(${RGA_LIB})

# 链接库
target_link_libraries(rga_bridge PRIVATE
    ${PROJECT_ROOT}/../librga/libs/Linux/gcc-aarch64/librga.a
    pthread
)
