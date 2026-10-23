cmake_minimum_required(VERSION 3.5)
project(armor_detect)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -O3 -g")

# ===== ① 搜索外部库（PDF 要求的第一步）=====
set(OpenCV_DIR /usr/local/opencv4.5.4/lib/cmake/opencv4)
find_package(OpenCV REQUIRED)
message(STATUS "OpenCV version: ${OpenCV_VERSION}")

# ===== 头文件搜索路径 =====
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR})
file(GLOB_RECURSE sources "src/*.cpp")

# ===== 主程序 =====
add_executable(armor main.cpp ${sources})

# ===== ② 指定头文件路径（PDF 第二步）=====
target_include_directories(armor PRIVATE ${OpenCV_INCLUDE_DIRS})

# ===== ③ 添加库链接（PDF 第三步）=====
target_link_libraries(armor PRIVATE ${OpenCV_LIBS})

# ==========================================================================
# 练习目标：practice/ 下每个 .cpp 编译成一个独立可执行程序
#
#   新增练习文件后：
#       cd ~/armor_detect/build
#       cmake ..                    # 重新生成（新文件才会被识别）
#       make p01_mat -j6            # 只编这一个
#       ./p01_mat                   # 运行
#
#   也可以 make -j6 一次编全部（armor + 所有练习）
# ==========================================================================
# GLOB_RECURSE：递归搜索 practice/ 及其子目录（例如 practice/p01ex/）
file(GLOB_RECURSE practice_sources "practice/*.cpp")
foreach(p ${practice_sources})
    get_filename_component(pname ${p} NAME_WE)
    add_executable(${pname} ${p})
    target_include_directories(${pname} PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(${pname} PRIVATE ${OpenCV_LIBS})
    message(STATUS "练习目标: ${pname}")
endforeach()
