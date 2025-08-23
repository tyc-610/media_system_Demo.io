#!/bin/bash

# Demo项目备份脚本
# 功能：完整备份Demo文件夹，包括所有文件、库文件、软连接等
# 使用方法：在Demo文件夹的上级目录中运行此脚本

# 设置颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 生成时间戳 (格式: YYYY-MM-DD_HH-MM-SS)
TIMESTAMP=$(date +"%Y-%m-%d_%H-%M-%S")

# 源目录名称
SOURCE_DIR="Demo"

# 备份目录名称
BACKUP_DIR="Demo_backup_${TIMESTAMP}"

# 日志文件
LOG_FILE="${BACKUP_DIR}_backup.log"

# 函数：打印带颜色的信息
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1" | tee -a "$LOG_FILE"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1" | tee -a "$LOG_FILE"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1" | tee -a "$LOG_FILE"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1" | tee -a "$LOG_FILE"
}

# 函数：检查命令是否存在
check_command() {
    if ! command -v $1 &> /dev/null; then
        print_error "命令 '$1' 未找到，请先安装"
        exit 1
    fi
}

# 函数：获取目录大小
get_dir_size() {
    du -sh "$1" 2>/dev/null | cut -f1
}

# 开始备份
echo "===============================================" | tee "$LOG_FILE"
echo "Demo项目备份脚本" | tee -a "$LOG_FILE"
echo "开始时间: $(date)" | tee -a "$LOG_FILE"
echo "===============================================" | tee -a "$LOG_FILE"

# 检查必要的命令
print_info "检查必要的命令..."
check_command "cp"
check_command "tar"
check_command "du"

# 检查源目录是否存在
if [ ! -d "$SOURCE_DIR" ]; then
    print_error "源目录 '$SOURCE_DIR' 不存在！"
    print_error "请确保在Demo文件夹的上级目录中运行此脚本"
    exit 1
fi

print_success "找到源目录: $SOURCE_DIR"

# 获取源目录大小
SOURCE_SIZE=$(get_dir_size "$SOURCE_DIR")
print_info "源目录大小: $SOURCE_SIZE"

# 检查磁盘空间
AVAILABLE_SPACE=$(df . | tail -1 | awk '{print $4}')
SOURCE_SIZE_KB=$(du -sk "$SOURCE_DIR" | cut -f1)

if [ "$SOURCE_SIZE_KB" -gt "$AVAILABLE_SPACE" ]; then
    print_error "磁盘空间不足！需要至少 $SOURCE_SIZE 的空间"
    exit 1
fi

print_success "磁盘空间检查通过"

# 创建备份目录
print_info "创建备份目录: $BACKUP_DIR"
if ! mkdir -p "$BACKUP_DIR"; then
    print_error "无法创建备份目录"
    exit 1
fi

# 执行备份 - 使用cp -a保持所有属性、链接等
print_info "开始备份文件..."
print_info "正在复制所有文件（包括软链接、硬链接、权限等）..."

# 使用cp -a进行完整备份，保持所有属性
if cp -a "$SOURCE_DIR"/* "$BACKUP_DIR"/ 2>>"$LOG_FILE"; then
    print_success "文件备份完成"
else
    print_warning "备份过程中可能出现一些警告，请检查日志文件"
fi

# 备份隐藏文件（如果存在）
if ls -la "$SOURCE_DIR"/.[^.]* 2>/dev/null; then
    print_info "发现隐藏文件，正在备份..."
    cp -a "$SOURCE_DIR"/.[^.]* "$BACKUP_DIR"/ 2>>"$LOG_FILE"
fi

# 验证备份
print_info "验证备份完整性..."

# 比较文件数量
SOURCE_FILES=$(find "$SOURCE_DIR" -type f | wc -l)
BACKUP_FILES=$(find "$BACKUP_DIR" -type f | wc -l)

print_info "源目录文件数: $SOURCE_FILES"
print_info "备份目录文件数: $BACKUP_FILES"

if [ "$SOURCE_FILES" -eq "$BACKUP_FILES" ]; then
    print_success "文件数量验证通过"
else
    print_warning "文件数量不匹配，可能存在特殊文件"
fi

# 获取备份目录大小
BACKUP_SIZE=$(get_dir_size "$BACKUP_DIR")
print_info "备份目录大小: $BACKUP_SIZE"

# 创建压缩备份（可选）
read -p "是否创建压缩备份文件？(y/N): " CREATE_ARCHIVE

if [[ $CREATE_ARCHIVE =~ ^[Yy]$ ]]; then
    print_info "正在创建压缩备份文件..."
    ARCHIVE_NAME="${BACKUP_DIR}.tar.gz"
    
    if tar -czf "$ARCHIVE_NAME" "$BACKUP_DIR" 2>>"$LOG_FILE"; then
        ARCHIVE_SIZE=$(get_dir_size "$ARCHIVE_NAME")
        print_success "压缩备份创建成功: $ARCHIVE_NAME (大小: $ARCHIVE_SIZE)"
        
        # 询问是否删除未压缩的备份目录
        read -p "是否删除未压缩的备份目录？(y/N): " DELETE_UNCOMPRESSED
        if [[ $DELETE_UNCOMPRESSED =~ ^[Yy]$ ]]; then
            rm -rf "$BACKUP_DIR"
            print_info "已删除未压缩的备份目录"
        fi
    else
        print_error "压缩备份创建失败"
    fi
fi

# 生成备份报告
print_info "生成备份报告..."
REPORT_FILE="${BACKUP_DIR}_report.txt"

cat > "$REPORT_FILE" << EOF
===============================================
Demo项目备份报告
===============================================
备份时间: $(date)
源目录: $SOURCE_DIR
备份目录: $BACKUP_DIR
源目录大小: $SOURCE_SIZE
备份目录大小: $BACKUP_SIZE
源文件数量: $SOURCE_FILES
备份文件数量: $BACKUP_FILES

备份内容包括：
- 所有源代码文件
- 编译后的二进制文件
- 库文件和依赖
- 软链接和硬链接
- 文件权限和属性
- 隐藏文件和配置文件

特殊处理：
- 保持软链接状态
- 保持文件权限
- 保持时间戳
- 保持目录结构

备份验证: $([ "$SOURCE_FILES" -eq "$BACKUP_FILES" ] && echo "通过" || echo "需要检查")
===============================================
EOF

print_success "备份报告已保存到: $REPORT_FILE"

# 显示备份内容概览
print_info "备份内容概览："
echo "主要目录和文件：" | tee -a "$LOG_FILE"
ls -la "$BACKUP_DIR" | head -20 | tee -a "$LOG_FILE"

if [ "$(ls -la "$BACKUP_DIR" | wc -l)" -gt 21 ]; then
    echo "... (还有更多文件)" | tee -a "$LOG_FILE"
fi

# 备份完成
echo "===============================================" | tee -a "$LOG_FILE"
print_success "备份完成！"
echo "完成时间: $(date)" | tee -a "$LOG_FILE"
echo "备份位置: $(pwd)/$BACKUP_DIR" | tee -a "$LOG_FILE"
echo "日志文件: $(pwd)/$LOG_FILE" | tee -a "$LOG_FILE"
echo "报告文件: $(pwd)/$REPORT_FILE" | tee -a "$LOG_FILE"
echo "===============================================" | tee -a "$LOG_FILE"

# 显示使用提示
print_info "使用提示："
echo "1. 备份目录保持了所有原始属性"
echo "2. 可以直接使用备份目录进行开发"
echo "3. 要恢复备份，可以使用: cp -a $BACKUP_DIR/* /目标路径/"
echo "4. 日志文件包含了详细的备份过程信息"

exit 0
