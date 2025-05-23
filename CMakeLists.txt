cmake_minimum_required(VERSION 3.12)
project(JPEG_SERVER 
    LANGUAGES C CXX  # 显式声明支持的语言
    VERSION 1.0
)

# 设置CMake模块路径
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

get_filename_component(PROJECT_ROOT "${CMAKE_SOURCE_DIR}" ABSOLUTE)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
# 查找依赖包
find_package(AML REQUIRED)
find_package(NeatVNC REQUIRED)
find_package(Pixman REQUIRED)

set(MPP_INC
    "${PROJECT_ROOT}/../mpp/osal/inc"
    "${PROJECT_ROOT}/../mpp/inc"
    "${PROJECT_ROOT}/../mpp/utils")

set(MPP_LIB
    "${PROJECT_ROOT}/../mpp/build/linux/aarch64/mpp"
    "${PROJECT_ROOT}/../mpp/build/linux/aarch64/utils"
    )

set(RGA_INC "${PROJECT_ROOT}/../librga/include")

set(RGA_LIB "${PROJECT_ROOT}/../librga/libs/Linux/gcc-aarch64")


message(STATUS "pixman_INCLUDE_DIRS = ${PIXMAN_INCLUDE_DIRS}")
message(STATUS "pixman_LIBS = ${PIXMAN_LIBRARIES}")
message(STATUS "JPEG_INCLUDE_DIRS = ${JPEG_INCLUDE_DIRS}")
message(STATUS "JPEG_LIBRARIES = ${JPEG_LIBRARIES}")
message(STATUS "AML_INCLUDE_DIRS = ${AML_INCLUDE_DIRS}")
message(STATUS "AML_LIBS = ${AML_LIBRARIES}")
message(STATUS "NeatVNC_INCLUDE_DIRS = ${NeatVNC_INCLUDE_DIRS}")
message(STATUS "NeatVNC_LIBS = ${NeatVNC_LIBRARIES}")

# 添加子目录
add_subdirectory(libs)

# 设置主程序
add_executable(jpeg-server 
    jpeg-server.c
)

add_executable(mjpeg-server 
    mjpeg-server.c
)

link_directories(${MPP_LIB} ${RGA_LIB})

find_library(UTILS_LIB 
    NAMES libutils.a
    PATHS "${PROJECT_SOURCE_DIR}/../mpp/build/linux/aarch64/utils"
    NO_DEFAULT_PATH
    DOC "Path to libutils.so"
)
message(STATUS "Found utils library: ${UTILS_LIB}")
find_library(ROCKCHIP_MPP_LIBS rockchip_mpp
    PATHS "${PROJECT_ROOT}/../mpp/build/linux/aarch64/mpp"
    NO_DEFAULT_PATH
    REQUIRED
)

message(STATUS "Found ROCKCHIP_MPP_LIBS library: ${ROCKCHIP_MPP_LIBS}")

# 包含目录
target_include_directories(jpeg-server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/libs
    ${AML_INCLUDE_DIRS}
    ${NEATVNC_INCLUDE_DIRS}
    ${PIXMAN_INCLUDE_DIRS}
)

# 链接依赖项
target_link_libraries(jpeg-server PRIVATE
    jpgfb_mpp
    rga_bridge
    ${AML_LIBRARIES}
    ${NeatVNC_LIBRARIES}
    ${PIXMAN_LIBRARIES}
    ${UTILS_LIBS}
    rt
    pthread
)

# 包含目录
target_include_directories(mjpeg-server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/libs
    ${AML_INCLUDE_DIRS}
    ${NEATVNC_INCLUDE_DIRS}
    ${PIXMAN_INCLUDE_DIRS}
)

# 链接依赖项
target_link_libraries(mjpeg-server PRIVATE
    jpgfb_mpp
    rga_bridge
    ${AML_LIBRARIES}
    ${NeatVNC_LIBRARIES}
    ${PIXMAN_LIBRARIES}
    ${UTILS_LIBS}
    rt
    pthread
)