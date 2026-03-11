/**
 ****************************************************************************
 * Copyright (C), 2026, MyslZhao 159505988+MyslZhao@users.noreply.github.com
 *
 * @file    barNotesHandle.cpp
 * @brief   SideBar侧边栏“Notes”功能相关方法的实现
 *
 * @author  MyslZhao
 */
#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <QFileInfo>

/**
 * @brief 笔记文件点击事件
 * @param[in] index 文件索引
 */
void MainWindow::onNoteTreeClicked(const QModelIndex &index)
{
    if (!index.isValid())
    {
        return;
    }

    QString path = _notes_system -> filePath(index);
    QFileInfo file_info(path);

    if (file_info.isDir())
    {
        _current_note_path = path;
        _note_name -> setText(file_info.fileName());
    }
    else if (file_info.isFile() && file_info.suffix() == "md")
    {
        _loadFile(path);
        _current_note_path = file_info.absolutePath();
        _note_name -> setText(QFileInfo(_current_note_path).fileName());
    }
}
