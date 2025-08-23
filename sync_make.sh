#!/bin/bash

# 智能同步编译脚本 - 同步共享文件夹到本地并编译
# 用法: ./sync_make.sh [clean|help]

SHARED_DIR="/mnt/hgfs/share/work_station/Demo"
LOCAL_DIR="/home/china/work_station/Demo"
TFTP_DIR="/home/china/tftpboot"

# 显示帮助信息
show_help() {
    echo "用法: ./sync_make.sh [选项]"
    echo ""
    echo "选项:"
    echo "  (无参数)  智能同步并编译"
    echo "  clean     强制重新cmake和编译"
    echo "  help      显示此帮助信息"
    echo ""
    echo "工作流程:"
    echo "  1. 同步共享文件夹到本地工作目录"
    echo "  2. 检测是否需要重新cmake（新增文件等）"
    echo "  3. 智能编译（make或cmake+make）"
    echo "  4. 复制可执行文件到TFTP目录"
    echo ""
    echo "文件路径:"
    echo "  共享目录: $SHARED_DIR"
    echo "  本地目录: $LOCAL_DIR"
    echo "  TFTP目录: $TFTP_DIR"
}

# 检查参数
if [ "$1" = "help" ]; then
    show_help
    exit 0
fi

echo "=== 智能同步编译系统 ==="

# 检查当前是否在build目录中
current_dir=$(basename "$PWD")
if [ "$current_dir" != "build" ]; then
    echo "❌ 请在build目录中运行此脚本"
    echo "正确路径: $LOCAL_DIR/build"
    echo "当前路径: $PWD"
    exit 1
fi

# 检查目录是否存在
if [ ! -d "$SHARED_DIR" ]; then
    echo "❌ 共享目录不存在: $SHARED_DIR"
    exit 1
fi

if [ ! -d "$LOCAL_DIR" ]; then
    echo "❌ 本地工作目录不存在: $LOCAL_DIR"
    exit 1
fi

# 强制清理模式
if [ "$1" = "clean" ]; then
    echo "🧹 强制清理重建模式..."
    
    # 回到项目根目录
    cd "$LOCAL_DIR" || exit 1
    
    # 先同步文件
    echo "1. 完整同步项目文件..."
    
    # 保护本地编译的FFmpeg库
    if [ -d "$LOCAL_DIR/lib/ffmpeg" ] && [ -f "$LOCAL_DIR/lib/ffmpeg/lib/libavcodec.so" ]; then
        echo "  🛡️  备份本地FFmpeg库..."
        mv "$LOCAL_DIR/lib/ffmpeg" "/tmp/ffmpeg_backup_$$"
        ffmpeg_backup_path="/tmp/ffmpeg_backup_$$"
    fi
    
    rsync -av --delete \
        --exclude 'build/' \
        --exclude 'bin/' \
        --exclude 'lib/' \
        "$SHARED_DIR/" "$LOCAL_DIR/"
    
    # 恢复FFmpeg库
    if [ -n "$ffmpeg_backup_path" ] && [ -d "$ffmpeg_backup_path" ]; then
        echo "  🔄 恢复本地FFmpeg库..."
        mkdir -p "$LOCAL_DIR/lib"
        mv "$ffmpeg_backup_path" "$LOCAL_DIR/lib/ffmpeg"
        echo "  ✓ FFmpeg库已恢复"
    fi
    
    echo "  ✓ 文件同步完成"
    
    # 自动扫描并更新CMakeLists.txt中的源文件
    echo "2. 自动扫描并更新源文件列表..."
    
    # 扫描所有.c文件（排除build和bin目录）
    c_files=$(find . -name "*.c" -not -path "./build/*" -not -path "./bin/*" -not -path "./ffmpeg-*/*" -not -path "./lvgl/*" -not -path "./lib/*" | sort)
    
    if [ -n "$c_files" ]; then
        echo "  📄 发现以下.c文件:"
        for file in $c_files; do
            echo "    - $file"
        done
        
        # 创建CMakeLists.txt的备份
        if [ -f "CMakeLists.txt" ]; then
            cp "CMakeLists.txt" "CMakeLists.txt.backup"
            echo "  💾 已备份 CMakeLists.txt → CMakeLists.txt.backup"
        fi
        
        # 更新CMakeLists.txt中的源文件列表
        # 找到 add_executable(main 这一行，并替换其后的文件列表
        if [ -f "CMakeLists.txt" ]; then
            # 构建新的源文件列表
            source_list="add_executable(main"
            for file in $c_files; do
                # 移除开头的./
                clean_file=${file#./}
                source_list="$source_list\n    $clean_file"
            done
            source_list="$source_list\n)"
            
            # 使用sed替换add_executable部分 - 更安全的方法
            # 首先找到add_executable(main开始的行号
            start_line=$(grep -n "add_executable(main" CMakeLists.txt | cut -d: -f1)
            if [ -n "$start_line" ]; then
                # 找到对应的)结束行号
                end_line=$(sed -n "${start_line},\$p" CMakeLists.txt | grep -n ")" | head -1 | cut -d: -f1)
                if [ -n "$end_line" ]; then
                    end_line=$((start_line + end_line - 1))
                    
                    # 创建临时文件进行替换
                    {
                        sed -n "1,$((start_line-1))p" CMakeLists.txt
                        echo -e "$source_list"
                        sed -n "$((end_line+1)),\$p" CMakeLists.txt
                    } > CMakeLists.txt.tmp && mv CMakeLists.txt.tmp CMakeLists.txt
                    
                    echo "  ✅ 已自动更新 CMakeLists.txt 中的源文件列表"
                else
                    echo "  ⚠️  无法找到add_executable的结束位置，跳过自动更新"
                fi
            else
                echo "  ⚠️  未找到add_executable(main行，跳过自动更新"
            fi
        fi
    else
        echo "  ⚠️  未发现.c文件"
    fi
    
    # 删除并重建build目录
    echo "3. 重建build目录..."
    rm -rf build
    mkdir build
    
    # 复制sync_make.sh脚本到build目录
    if [ -f "sync_make.sh" ]; then
        cp "sync_make.sh" "build/"
        chmod +x "build/sync_make.sh"
        echo "  📝 已复制 sync_make.sh → build/sync_make.sh"
    fi
    
    cd build || exit 1
    echo "  ✓ build目录已重建"
    
    # CMake配置
    echo "4. CMake配置..."
    cmake ..
    if [ $? -ne 0 ]; then
        echo "❌ CMake配置失败"
        exit 1
    fi
    echo "  ✓ CMake配置完成"
    
    # 编译
    echo "5. 开始编译..."
    make
    
    if [ $? -eq 0 ]; then
        echo "  ✅ 编译成功！"
        
        # 复制到TFTP
        if [ -f "../bin/main" ]; then
            mkdir -p "$TFTP_DIR"
            cp "../bin/main" "$TFTP_DIR/"
            echo "  ✓ 已复制 main → $TFTP_DIR/main"
        fi
        
        if [ -f "../lib/liblvgl.so" ]; then
            cp "../lib/liblvgl.so" "$TFTP_DIR/"
            echo "  ✓ 已复制 liblvgl.so → $TFTP_DIR/liblvgl.so"
        fi
        
        echo ""
        echo "🌐 文件已准备就绪，可以下载到开发板！"
    else
        echo "❌ 编译失败"
        exit 1
    fi
    
    exit 0
fi

# 检测需要重新cmake的情况
need_recmake=false

echo "1. 同步共享文件夹到本地工作目录..."

# 同步源代码目录
if [ -d "$SHARED_DIR/src" ]; then
    rsync -av "$SHARED_DIR/src/" "$LOCAL_DIR/src/"
    echo "  ✓ 同步 src/ 目录"
fi

# 同步头文件目录
if [ -d "$SHARED_DIR/inc" ]; then
    rsync -av "$SHARED_DIR/inc/" "$LOCAL_DIR/inc/"
    echo "  ✓ 同步 inc/ 目录"
fi

if [ -d "$SHARED_DIR/include" ]; then
    rsync -av "$SHARED_DIR/include/" "$LOCAL_DIR/include/"
    echo "  ✓ 同步 include/ 目录"
fi

# 同步库文件目录（但排除FFmpeg库，保护本地编译的版本）
if [ -d "$SHARED_DIR/lib" ]; then
    # 检查本地是否有编译好的FFmpeg库
    if [ -d "$LOCAL_DIR/lib/ffmpeg" ] && [ -f "$LOCAL_DIR/lib/ffmpeg/lib/libavcodec.so" ]; then
        echo "  🛡️  检测到本地FFmpeg库，跳过同步以保护本地编译版本"
        rsync -av --copy-links --exclude 'ffmpeg/' "$SHARED_DIR/lib/" "$LOCAL_DIR/lib/"
        echo "  ✓ 同步 lib/ 目录（已排除ffmpeg/）"
    else
        rsync -av --copy-links "$SHARED_DIR/lib/" "$LOCAL_DIR/lib/"
        echo "  ✓ 同步 lib/ 目录（库文件）"
    fi
fi

# 同步资源文件
if [ -d "$SHARED_DIR/pic" ]; then
    rsync -av "$SHARED_DIR/pic/" "$LOCAL_DIR/pic/"
    echo "  ✓ 同步 pic/ 目录（资源文件）"
fi

# 同步根目录的源文件
rsync -av "$SHARED_DIR"/*.c "$LOCAL_DIR/" 2>/dev/null && echo "  ✓ 同步根目录 .c 文件" || true
rsync -av "$SHARED_DIR"/*.h "$LOCAL_DIR/" 2>/dev/null && echo "  ✓ 同步根目录 .h 文件" || true

# 同步配置文件
rsync -av "$SHARED_DIR/CMakeLists.txt" "$LOCAL_DIR/" 2>/dev/null && echo "  ✓ 同步 CMakeLists.txt" || true
rsync -av "$SHARED_DIR/lv_conf.h" "$LOCAL_DIR/" 2>/dev/null && echo "  ✓ 同步 lv_conf.h" || true
rsync -av "$SHARED_DIR/Makefile" "$LOCAL_DIR/" 2>/dev/null && echo "  ✓ 同步 Makefile" || true

# 同步脚本文件到当前build目录
if [ -f "$SHARED_DIR/sync_make.sh" ]; then
    cp "$SHARED_DIR/sync_make.sh" .
    chmod +x sync_make.sh
    echo "  📝 同步 sync_make.sh → build/sync_make.sh"
fi

echo ""
echo "2. 检测项目变化..."

# 检查CMakeLists.txt是否更新
if [ "$SHARED_DIR/CMakeLists.txt" -nt "CMakeCache.txt" ] 2>/dev/null; then
    echo "  � 检测到 CMakeLists.txt 更新"
    need_recmake=true
fi

# 检查是否有新的.c文件（比较文件数量）
shared_c_files=$(find "$SHARED_DIR" -name "*.c" -not -path "*/build/*" -not -path "*/bin/*" 2>/dev/null | wc -l)
local_c_files=$(find "$LOCAL_DIR" -name "*.c" -not -path "*/build/*" -not -path "*/bin/*" 2>/dev/null | wc -l)
if [ "$shared_c_files" -ne "$local_c_files" ]; then
    echo "  📄 检测到 .c 文件数量变化 (共享:$shared_c_files vs 本地:$local_c_files)"
    need_recmake=true
fi

# 检查是否有新的.h文件
shared_h_files=$(find "$SHARED_DIR" -name "*.h" -not -path "*/build/*" -not -path "*/bin/*" 2>/dev/null | wc -l)
local_h_files=$(find "$LOCAL_DIR" -name "*.h" -not -path "*/build/*" -not -path "*/bin/*" 2>/dev/null | wc -l)
if [ "$shared_h_files" -ne "$local_h_files" ]; then
    echo "  📄 检测到 .h 文件数量变化 (共享:$shared_h_files vs 本地:$local_h_files)"
    need_recmake=true
fi

# 检查CMakeCache.txt是否存在
if [ ! -f "CMakeCache.txt" ]; then
    echo "  🔧 未找到CMake缓存，需要重新配置"
    need_recmake=true
fi

echo ""
echo "3. 决定编译策略..."

if [ "$need_recmake" = true ]; then
    echo "  🔄 检测到结构变化，需要重新CMake配置"
    
    # 清理CMake缓存
    rm -f CMakeCache.txt
    rm -rf CMakeFiles/
    
    # 重新配置
    echo "  📋 执行 cmake .."
    cmake ..
    if [ $? -ne 0 ]; then
        echo "❌ CMake配置失败"
        echo "💡 建议: ./sync_make.sh clean"
        exit 1
    fi
    echo "  ✓ CMake配置完成"
else
    echo "  ⚡ 仅代码修改，直接编译"
fi

echo ""
echo "4. 开始编译..."

# 执行make
make

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ 编译成功！"
    
    # 显示生成的文件信息
    if [ -f "../bin/main" ]; then
        file_size=$(stat -c%s ../bin/main 2>/dev/null | numfmt --to=iec 2>/dev/null || stat -c%s ../bin/main)
        echo "📱 可执行文件: $LOCAL_DIR/bin/main (${file_size})"
        
        # 复制到TFTP目录
        echo ""
        echo "📡 复制到TFTP传输目录..."
        mkdir -p "$TFTP_DIR"
        cp "../bin/main" "$TFTP_DIR/"
        echo "  ✓ 已复制 main → $TFTP_DIR/main"
    else
        echo "⚠️  未找到可执行文件 $LOCAL_DIR/bin/main"
    fi
    
    # 复制动态库（如果存在）
    if [ -f "../lib/liblvgl.so" ]; then
        lib_size=$(stat -c%s ../lib/liblvgl.so 2>/dev/null | numfmt --to=iec 2>/dev/null || stat -c%s ../lib/liblvgl.so)
        echo "📚 动态库: $LOCAL_DIR/lib/liblvgl.so (${lib_size})"
        
        cp "../lib/liblvgl.so" "$TFTP_DIR/"
        echo "  ✓ 已复制 liblvgl.so → $TFTP_DIR/liblvgl.so"
    fi
    
    echo ""
    echo "🌐 TFTP传输文件准备就绪！"
    echo "  📁 TFTP目录: $TFTP_DIR"
    echo "  🔧 开发板获取命令: tftp -g -r main [Linux主机IP]"
    if [ -f "$TFTP_DIR/liblvgl.so" ]; then
        echo "  📚 开发板获取库文件: tftp -g -r liblvgl.so [Linux主机IP]"
    fi
    
    echo ""
    echo "� 下次编译提示:"
    echo "  • 仅修改代码内容 → ./sync_make.sh"
    echo "  • 添加新文件或大幅修改 → ./sync_make.sh clean"
    
else
    echo ""
    echo "❌ 编译失败！"
    echo "💡 建议:"
    echo "  1. 检查代码错误"
    echo "  2. 尝试清理重建: ./sync_make.sh clean"
    echo "  3. 检查共享文件夹同步是否正常"
fi
