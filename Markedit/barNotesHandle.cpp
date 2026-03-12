/**
 ****************************************************************************
 * Copyright (C), 2026, MyslZhao 159505988+MyslZhao@users.noreply.github.com
 *
 * @file    barNotesHandle.cpp
 * @brief   SideBar侧边栏“Notes”功能相关方法的实现
 * @details
 * 该文件包含了：
 * + 对“笔记”按钮与笔记内条块点击所触发的事件逻辑
 * + “笔记”右键菜单相关操作的实现
 *  (基本全部改自menuEditHandle与menuFileHandle文件内部分方法的逻辑)
 *
 * @author  MyslZhao
 */
#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <QFileInfo>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>

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

        QDir dir(path);
        dir.setNameFilters(QStringList() << "*.md");
        dir.setFilter(QDir::Files);
        QStringList files = dir.entryList();
        if (!files.isEmpty())
        {
            files.sort();
            QString f_file = path + "/" + files.first();
            _loadFile(f_file);
        }
    }
    else if (file_info.isFile() && file_info.suffix() == "md")
    {
        _loadFile(path);
        _current_note_path = file_info.absolutePath();
        _note_name -> setText(QFileInfo(_current_note_path).fileName());
    }
}

/**
 * @brief 显示“笔记”窗口右键菜单
 * @param[in] pos 鼠标点击坐标
 */
void MainWindow::showNotesMenu(const QPoint &pos)
{
    QModelIndex index = notes_tree -> indexAt(pos);
    if (!index.isValid())
    {
        return;
    }

    QString path = _notes_system -> filePath(index);
    QFileInfo info(path);

    if (!info.isFile() || info.suffix() != "md")
    {
        return;
    }

    QMenu menu(this);
    QAction *move_act = menu.addAction("移动");
    QAction *remove_act = menu.addAction("移除");
    QAction *rename_act = menu.addAction("重命名");
    QAction *savas_act = menu.addAction("另存为");

    connect(remove_act, &QAction::triggered, this, &MainWindow::removeSelectedFile);
    connect(move_act, &QAction::triggered, this, &MainWindow::moveSelectedFile);
    connect(rename_act, &QAction::triggered, this, &MainWindow::renameSelectedFile);
    connect(savas_act, &QAction::triggered, this, &MainWindow::saveAsSelectedFile);

    menu.exec(notes_tree -> viewport() -> mapToGlobal(pos));
}

/**
 * @brief 移除所选文件
 */
void MainWindow::removeSelectedFile()
{
    QModelIndex index = notes_tree->currentIndex();
    if (!index.isValid()) return;

    QString path = _notes_system->filePath(index);
    QFileInfo info(path);
    if (!info.isFile()) return;

    if (path == _current_path) {
        int ret = QMessageBox::question(this, "文件正在编辑",
                                        "该文件正在编辑中，确定要删除吗？",
                                        QMessageBox::Yes | QMessageBox::No);
        if (ret != QMessageBox::Yes) return;
        newFile();
    }

    QFile file(path);
    if (file.remove()) {
        QMessageBox::information(this, "成功", "文件已移除。");
    } else {
        QMessageBox::warning(this, "错误", "无法删除文件: " + file.errorString());
    }
}

/**
 * @brief 在笔记间移动文件
 */
void MainWindow::moveSelectedFile()
{
    QModelIndex index = notes_tree->currentIndex();
    if (!index.isValid()) return;

    QString srcPath = _notes_system->filePath(index);
    QFileInfo srcInfo(srcPath);
    if (!srcInfo.isFile()) return;

    QDir notesDir(_notes_path);
    QStringList noteFolders = notesDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    if (noteFolders.isEmpty()) {
        QMessageBox::information(this, "提示", "没有可移动的目标笔记。");
        return;
    }

    bool ok;
    QString targetNote = QInputDialog::getItem(this, "选择目标笔记",
                                               "请选择要移动到的笔记:",
                                               noteFolders, 0, false, &ok);
    if (!ok) return;

    QString targetDir = _notes_path + "/" + targetNote;
    QString targetPath = targetDir + "/" + srcInfo.fileName();

    if (QFile::exists(targetPath)) {
        int ret = QMessageBox::question(this, "文件已存在",
                                        "目标笔记中已有同名文件，是否覆盖？",
                                        QMessageBox::Yes | QMessageBox::No);
        if (ret != QMessageBox::Yes) return;
        QFile::remove(targetPath);
    }

    if (srcPath == _current_path) {
        if (QFile::rename(srcPath, targetPath)) {
            _current_path = targetPath;
            setWindowTitle(QFileInfo(targetPath).fileName() + "[*] - Markedit");
            QMessageBox::information(this, "成功", "文件已移动。");
        } else {
            QMessageBox::warning(this, "错误", "移动失败。");
        }
    } else {
        if (!QFile::rename(srcPath, targetPath)) {
            QMessageBox::warning(this, "错误", "移动失败。");
        } else {
            QMessageBox::information(this, "成功", "文件已移动。");
        }
    }
}

/**
 * @brief 重命名文件
 */
void MainWindow::renameSelectedFile()
{
    QModelIndex index = notes_tree->currentIndex();
    if (!index.isValid()) return;

    QString oldPath = _notes_system->filePath(index);
    QFileInfo oldInfo(oldPath);
    if (!oldInfo.isFile()) return;

    bool ok;
    QString newName = QInputDialog::getText(this, "重命名文件",
                                            "输入新文件名（保留 .md 后缀）:",
                                            QLineEdit::Normal,
                                            oldInfo.fileName(), &ok);
    if (!ok || newName.isEmpty()) return;

    if (!newName.endsWith(".md", Qt::CaseInsensitive)) {
        newName += ".md";
    }

    QString newPath = oldInfo.absolutePath() + "/" + newName;

    if (QFile::exists(newPath)) {
        QMessageBox::warning(this, "错误", "该名称已存在，请重新输入。");
        return;
    }

    if (oldPath == _current_path) {
        if (QFile::rename(oldPath, newPath)) {
            _current_path = newPath;
            setWindowTitle(newName + "[*] - Markedit");
            QMessageBox::information(this, "成功", "文件已重命名。");
        } else {
            QMessageBox::warning(this, "错误", "重命名失败。");
        }
    } else {
        if (QFile::rename(oldPath, newPath)) {
            QMessageBox::information(this, "成功", "文件已重命名。");
        } else {
            QMessageBox::warning(this, "错误", "重命名失败。");
        }
    }
}

/**
 * @brief 另存为文件
 */
void MainWindow::saveAsSelectedFile()
{
    QModelIndex index = notes_tree->currentIndex();
    if (!index.isValid()) return;

    QString srcPath = _notes_system->filePath(index);
    QFileInfo srcInfo(srcPath);
    if (!srcInfo.isFile()) return;

    QString savePath = QFileDialog::getSaveFileName(this, "另存为",
                                                    srcInfo.fileName(),
                                                    "Markdown files (*.md)");
    if (savePath.isEmpty()) return;

    if (QFile::copy(srcPath, savePath)) {
        QMessageBox::information(this, "成功", "文件已另存。");
    } else {
        QMessageBox::warning(this, "错误", "另存失败，请检查目标路径。");
    }
}
