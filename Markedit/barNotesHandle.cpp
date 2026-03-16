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
#include <QObject>

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
    QAction *move_act = menu.addAction(QObject::tr("移动"));
    QAction *remove_act = menu.addAction(QObject::tr("移除"));
    QAction *rename_act = menu.addAction(QObject::tr("重命名"));
    QAction *savas_act = menu.addAction(QObject::tr("另存为"));

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
        int ret = QMessageBox::question(this, QObject::tr("文件正在编辑"),
                                        QObject::tr("该文件正在编辑中，确定要删除吗？"),
                                        QMessageBox::Yes | QMessageBox::No);
        if (ret != QMessageBox::Yes) return;
    }

    QFile file(path);
    if (file.remove()) {
        if (path == _current_path)
        {
            ui -> markdownEdit -> clear();
            ui -> markdownEdit -> document() -> setModified(false);

            _is_untitled = true;
            _current_path.clear();
            _setCurrentFileName(QString());
        }
        QMessageBox::information(this, QObject::tr("成功"), QObject::tr("文件已移除。"));
    }
    else
    {
        QMessageBox::warning(this, QObject::tr("错误"), QObject::tr("无法删除文件: ") + file.errorString());
    }
}

/**
 * @brief 在笔记间移动文件
 */
void MainWindow::moveSelectedFile()
{
    QModelIndex index = notes_tree->currentIndex();
    if (!index.isValid()) return;

    QString src_path = _notes_system->filePath(index);
    QFileInfo src_info(src_path);
    if (!src_info.isFile()) return;

    QDir notes_dir(_notes_path);
    QStringList note_folders = notes_dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    if (note_folders.isEmpty()) {
        QMessageBox::information(this, QObject::tr("提示"), QObject::tr("没有可移动的目标笔记。"));
        return;
    }

    bool ok;
    QString target_note = QInputDialog::getItem(this, QObject::tr("选择目标笔记"),
                                               QObject::tr("请选择要移动到的笔记:"),
                                               note_folders, 0, false, &ok);
    if (!ok) return;

    QString target_dir = _notes_path + "/" + target_note;
    QString target_path = target_dir + "/" + src_info.fileName();

    if (QFile::exists(target_path)) {
        int ret = QMessageBox::question(this, QObject::tr("文件已存在"),
                                        QObject::tr("目标笔记中已有同名文件，是否覆盖？"),
                                        QMessageBox::Yes | QMessageBox::No);
        if (ret != QMessageBox::Yes) return;
        QFile::remove(target_path);
    }

    if (src_path == _current_path) {
        if (QFile::rename(src_path, target_path)) {
            _current_path = target_path;
            setWindowTitle(QFileInfo(target_path).fileName() + "[*] - Markedit");
            QMessageBox::information(this, QObject::tr("成功"), QObject::tr("文件已移动。"));
        } else {
            QMessageBox::warning(this, QObject::tr("错误"), QObject::tr("移动失败。"));
        }
    } else {
        if (!QFile::rename(src_path, target_path)) {
            QMessageBox::warning(this, QObject::tr("错误"), QObject::tr("移动失败。"));
        } else {
            QMessageBox::information(this, QObject::tr("成功"), QObject::tr("文件已移动。"));
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

    QString old_path = _notes_system->filePath(index);
    QFileInfo old_info(old_path);
    if (!old_info.isFile()) return;

    bool ok;
    QString new_name = QInputDialog::getText(this, QObject::tr("重命名文件"),
                                            QObject::tr("输入新文件名（保留 .md 后缀）:"),
                                            QLineEdit::Normal,
                                            old_info.fileName(), &ok);
    if (!ok || new_name.isEmpty()) return;

    if (!new_name.endsWith(".md", Qt::CaseInsensitive)) {
        new_name += ".md";
    }

    QString new_path = old_info.absolutePath() + "/" + new_name;

    if (QFile::exists(new_path)) {
        QMessageBox::warning(this, QObject::tr("错误"), QObject::tr("该名称已存在，请重新输入。"));
        return;
    }

    if (old_path == _current_path) {
        if (QFile::rename(old_path, new_path)) {
            _current_path = new_path;
            setWindowTitle(new_name + "[*] - Markedit");
            QMessageBox::information(this, QObject::tr("成功"), QObject::tr("文件已重命名。"));
        } else {
            QMessageBox::warning(this, QObject::tr("错误"), QObject::tr("重命名失败。"));
        }
    } else {
        if (QFile::rename(old_path, new_path)) {
            QMessageBox::information(this, QObject::tr("成功"), QObject::tr("文件已重命名。"));
        } else {
            QMessageBox::warning(this, QObject::tr("错误"), QObject::tr("重命名失败。"));
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

    QString src_path = _notes_system->filePath(index);
    QFileInfo src_info(src_path);
    if (!src_info.isFile()) return;

    QString save_path = QFileDialog::getSaveFileName(this, QObject::tr("另存为"),
                                                    src_info.fileName(),
                                                    "Markdown files (*.md)");
    if (save_path.isEmpty()) return;

    if (QFile::copy(src_path, save_path)) {
        QMessageBox::information(this, QObject::tr("成功"), QObject::tr("文件已另存。"));
    } else {
        QMessageBox::warning(this, QObject::tr("错误"), QObject::tr("另存失败，请检查目标路径。"));
    }
}
