/**
 ****************************************************************************
 * Copyright (C), 2026, MyslZhao 159505988+MyslZhao@users.noreply.github.com
 *
 * @file    barOutlineHandle.cpp
 * @brief   SideBar侧边栏“Outline”功能相关方法的实现
 *
 * @author  MyslZhao
 */
#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <QTextBlock>

/**
 * @brief 更新文件大纲内容
 * @details
 *
 */
void MainWindow::_updateOutline()
{
    outline_list->clear();
    outline_pos.clear();

    QString text = ui->markdownEdit->toPlainText();
    QStringList lines = text.split('\n');
    QRegularExpression re("^(\\s*)(#{1,6})\\s+(.*)$"); // NOTE: “大纲”以文件标题(#{1,6})为条目
    int block_num = 0;
    for (const QString &line : lines) {
        QRegularExpressionMatch match = re.match(line);
        if (match.hasMatch()) {
            QString levelStr = match.captured(2);
            QString title = match.captured(3);
            int level = levelStr.length();

            // 根据级别缩进（每级两个空格）
            QString indent = QString(" ").repeated(level * 2);
            QListWidgetItem *item = new QListWidgetItem(indent + title, outline_list);
            QFont font = item -> font();
            font.setBold(true);
            item->setFont(font);

            outline_pos[item] = block_num;
        }
        block_num += 1;
    }
}

/**
 * @brief MainWindow::onOutlineItemClicked
 * @param item
 */
void MainWindow::onOutlineItemClicked(QListWidgetItem *item)
{
    if (!outline_pos.contains(item)) return;
    int block_num = outline_pos[item];
    QTextBlock block = ui->markdownEdit->document()->findBlockByNumber(block_num);
    if (block.isValid()) {
        QTextCursor cursor(block);
        ui -> markdownEdit -> setTextCursor(cursor);
        ui -> markdownEdit -> ensureCursorVisible();
    }
}
