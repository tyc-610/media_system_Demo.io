#!/bin/bash

# Demo项目简化备份脚本
# 使用方法：将此脚本放在Demo文件夹的上级目录，然后运行 ./backup_demo_simple.sh

# 生成时间戳
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
SOURCE_DIR="Demo"
BACKUP_DIR="Demo_backup_${TIMESTAMP}"

echo "开始备份Demo项目..."
echo "时间：$(date)"

# 检查源目录
if [ ! -d "$SOURCE_DIR" ]; then
    echo "错误：找不到Demo目录！请确保在Demo的上级目录运行此脚本。"
    exit 1
fi

echo "源目录：$SOURCE_DIR"
echo "备份目录：$BACKUP_DIR"

# 创建备份目录
mkdir -p "$BACKUP_DIR"

# 执行备份（保持所有属性、软链接等）
echo "正在备份文件..."
cp -a "$SOURCE_DIR"/* "$BACKUP_DIR"/

# 备份隐藏文件
if ls "$SOURCE_DIR"/.[^.]* 1> /dev/null 2>&1; then
    echo "备份隐藏文件..."
    cp -a "$SOURCE_DIR"/.[^.]* "$BACKUP_DIR"/
fi

# 验证备份
SOURCE_FILES=$(find "$SOURCE_DIR" -type f | wc -l)
BACKUP_FILES=$(find "$BACKUP_DIR" -type f | wc -l)

echo "源文件数：$SOURCE_FILES"
echo "备份文件数：$BACKUP_FILES"

if [ "$SOURCE_FILES" -eq "$BACKUP_FILES" ]; then
    echo "✓ 备份完成！文件数量匹配。"
else
    echo "⚠ 备份完成，但文件数量不完全匹配，请检查。"
fi

echo "备份位置：$(pwd)/$BACKUP_DIR"
echo "备份大小：$(du -sh "$BACKUP_DIR" | cut -f1)"
echo "完成时间：$(date)"
