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
 *
 */
void MainWindow::_updateOutline()
{
    _outline_system -> clear();

    QString text = ui->markdownEdit->toPlainText();
    QStringList lines = text.split('\n');
    QRegularExpression re("^(\\s*)(#{1,6})\\s+(.*)$"); // NOTE: “大纲”以文件标题(#{1,6})为条目

    QList<QStandardItem*> parent_stack;
    parent_stack.append(_outline_system -> invisibleRootItem());

    int block_num = 0;
    int last_level = 0;

    for (const QString &line : lines) {
        QRegularExpressionMatch match = re.match(line);
        if (match.hasMatch()) {
            QString levelStr = match.captured(2);
            QString title = match.captured(3);
            int level = levelStr.length();

            if (level > last_level)
            {
                while (parent_stack.size() <= level)
                {
                    parent_stack.append(parent_stack.last());
                }
            }
            else if (level < last_level)
            {
                while (parent_stack.size() > level + 1)
                {
                    parent_stack.removeLast();
                }
            }

            QStandardItem *item = new QStandardItem(title);
            item -> setData(block_num, Qt::UserRole);

            parent_stack.append(item);

            last_level = level;
        }
        block_num += 1;
    }
    outline_tree -> expandAll();
}

/**
 * @brief 大纲按钮交互事件
 * @param[in] index 被点击按钮的索引
 */
void MainWindow::onOutlineItemClicked(const QModelIndex &index)
{
    if (!index.isValid())
    {
        return;
    }

    QStandardItem *item = _outline_system -> itemFromIndex(index);
    if (!item)
    {
        return;
    }

    int block_num = item -> data(Qt::UserRole).toInt();

    QTextBlock block = ui->markdownEdit->document()->findBlockByNumber(block_num);
    if (block.isValid()) {
        QTextCursor cursor(block);
        ui -> markdownEdit -> setTextCursor(cursor);
        ui -> markdownEdit -> ensureCursorVisible();
    }
}
