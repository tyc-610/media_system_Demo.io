cmake_minimum_required(VERSION 3.10)


#设置交叉编译
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
#这里指定的路径必须是绝对路径
set(CMAKE_C_COMPILER /usr/local/arm/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabi/bin/arm-linux-gcc)
set(CMAKE_CXX_COMPILER /usr/local/arm/gcc-linaro-5.5.0-2017.10-x86_64_arm-linux-gnueabi/bin/arm-linux-g++)

project(lvgl)

set(CMAKE_C_STANDARD 99)#C99 # lvgl officially support C99 and above
set(CMAKE_CXX_STANDARD 17)#C17
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 设置全局库输出路径
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# 设置FFmpeg路径
set(FFMPEG_DIR ${PROJECT_SOURCE_DIR}/lib/ffmpeg)
set(FFMPEG_INCLUDE_DIR ${FFMPEG_DIR}/include)
set(FFMPEG_LIB_DIR ${FFMPEG_DIR}/lib)

# 检查FFmpeg是否已安装
if(EXISTS ${FFMPEG_INCLUDE_DIR} AND EXISTS ${FFMPEG_LIB_DIR})
    # 查找FFmpeg库
    find_library(AVCODEC_LIB avcodec PATHS ${FFMPEG_LIB_DIR} NO_DEFAULT_PATH)
    find_library(AVFORMAT_LIB avformat PATHS ${FFMPEG_LIB_DIR} NO_DEFAULT_PATH)
    find_library(AVUTIL_LIB avutil PATHS ${FFMPEG_LIB_DIR} NO_DEFAULT_PATH)
    find_library(SWSCALE_LIB swscale PATHS ${FFMPEG_LIB_DIR} NO_DEFAULT_PATH)
    
    if(AVCODEC_LIB AND AVFORMAT_LIB AND AVUTIL_LIB AND SWSCALE_LIB)
        set(FFMPEG_FOUND TRUE)
        message(STATUS "✓ FFmpeg found: ${FFMPEG_LIB_DIR}")
        message(STATUS "  - avcodec: ${AVCODEC_LIB}")
        message(STATUS "  - avformat: ${AVFORMAT_LIB}")
        message(STATUS "  - avutil: ${AVUTIL_LIB}")
        message(STATUS "  - swscale: ${SWSCALE_LIB}")
    else()
        set(FFMPEG_FOUND FALSE)
        message(WARNING "❌ FFmpeg libraries not complete in ${FFMPEG_LIB_DIR}")
    endif()
else()
    set(FFMPEG_FOUND FALSE)
    message(WARNING "❌ FFmpeg not found. Run ./build_ffmpeg.sh to build FFmpeg first.")
endif()

# 如果找到FFmpeg，设置全局编译选项
if(FFMPEG_FOUND)
    # 设置全局include目录，这样所有目标都能找到FFmpeg头文件
    include_directories(${FFMPEG_INCLUDE_DIR})
    message(STATUS "✓ Global FFmpeg include path configured")
endif()

add_subdirectory(lvgl)
target_include_directories(lvgl PUBLIC ${PROJECT_SOURCE_DIR})

# 设置LVGL为动态链接库
set_target_properties(lvgl PROPERTIES 
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib
    POSITION_INDEPENDENT_CODE ON
)

# 创建动态链接库目录
file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

# 将LVGL编译为动态链接库
add_library(lvgl_shared SHARED $<TARGET_OBJECTS:lvgl>)
set_target_properties(lvgl_shared PROPERTIES 
    OUTPUT_NAME "lvgl"
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib
    VERSION 1.0
    SOVERSION 1
)

# 链接动态库所需的依赖
if(FFMPEG_FOUND)
    # 不直接为lvgl静态库添加FFmpeg库，而是通过lvgl_shared处理
    target_link_libraries(lvgl_shared 
        ${AVCODEC_LIB} ${AVFORMAT_LIB} ${AVUTIL_LIB} ${SWSCALE_LIB}
        m pthread)
    target_include_directories(lvgl_shared PUBLIC ${FFMPEG_INCLUDE_DIR})
    message(STATUS "✓ LVGL shared library linked with FFmpeg libraries")
else()
    target_link_libraries(lvgl_shared m pthread)
    message(STATUS "ℹ LVGL compiled without FFmpeg support")
endif()

add_executable(main 
    src/main.c 
    src/start_video.c 
    src/screen.c 
    src/main_screen.c
    src/kugou_music.c
    src/dcim.c
    src/data.c
    src/my_font.c 
    mouse_cursor_icon.c
)

# 使用动态链接库
if(FFMPEG_FOUND)
    target_include_directories(main PRIVATE ${FFMPEG_INCLUDE_DIR})
endif()

# 设置运行时库路径 
set_target_properties(main PROPERTIES 
    BUILD_RPATH "${PROJECT_SOURCE_DIR}/lib:${FFMPEG_LIB_DIR}"
    INSTALL_RPATH "./lib:./lib/ffmpeg/lib"
)

# 链接配置
if(FFMPEG_FOUND)
    target_link_libraries(main lvgl_shared 
        ${AVCODEC_LIB} ${AVFORMAT_LIB} ${AVUTIL_LIB} ${SWSCALE_LIB}
        m pthread)
    message(STATUS "✓ Main executable linked with FFmpeg libraries")
else()
    # 如果没有FFmpeg，只链接LVGL
    target_link_libraries(main lvgl_shared m pthread)
    message(WARNING "⚠ Main executable compiled without FFmpeg - video playback will not work")
endif()
add_custom_target (run COMMAND ${EXECUTABLE_OUTPUT_PATH}/main DEPENDS main)

# 安装动态库到系统目录
install(TARGETS lvgl_shared 
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# 安装可执行文件
install(TARGETS main
    RUNTIME DESTINATION bin
)

