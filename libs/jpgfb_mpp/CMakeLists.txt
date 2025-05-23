# 创建JPEG-MPP库
add_library(jpgfb_mpp STATIC
    jpgfb_mpp.c
    get_img_size.c
)

# 包含目录
target_include_directories(jpgfb_mpp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${MPP_INC}
)

link_directories(${PROJECT_ROOT}/../mpp/build/linux/aarch64/utils)
# 链接库
target_link_libraries(jpgfb_mpp PRIVATE
    rga_bridge
    ${ROCKCHIP_MPP_LIBS}
    ${PROJECT_ROOT}/../mpp/build/linux/aarch64/utils/libutils.a
    pthread
)
