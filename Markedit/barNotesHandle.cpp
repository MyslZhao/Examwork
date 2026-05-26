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

    QString path = _notes_system->filePath(index);
    QFileInfo file_info(path);

    if (file_info.isDir())
    {
        return;
    }
    else if (file_info.isFile())
    {
        ui->statusbar->showMessage(tr("已选择笔记文件: %1").arg(file_info.fileName()), 2000);
    }
}

/**
 * @brief 显示“笔记”窗口右键菜单
 * @param[in] pos 鼠标点击坐标
 */
void MainWindow::showNotesMenu(const QPoint &pos)
{
    QModelIndex index = notes_tree->indexAt(pos);
    if (!index.isValid())
    {
        return;
    }

    QString path = _notes_system->filePath(index);
    QFileInfo info(path);

    if (!info.isFile() || info.suffix() != "md")
    {
        return;
    }

    QMenu menu(this);
    QAction *move_act = menu.addAction(tr("移动"));
    QAction *remove_act = menu.addAction(tr("移除"));
    QAction *rename_act = menu.addAction(tr("重命名"));
    QAction *save_as_act = menu.addAction(tr("另存为"));

    QAction *act = menu.exec(notes_tree->viewport()->mapToGlobal(pos));
    if (!act)
    {
        return;
    }

    if (act == move_act) {
        moveSelectedFile();
    } else if (act == remove_act) {
        removeSelectedFile();
    } else if (act == rename_act) {
        renameSelectedFile();
    } else if (act == save_as_act) {
        saveAsSelectedFile();
    }
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

    QString target_note = QInputDialog::getItem(this, tr("选择目标笔记"),
                                               tr("请选择要移动到的笔记:"),
                                               note_folders, 0, false);
    if (!target_note.isEmpty()) {
        QString target_dir = _notes_path + "/" + target_note;
        QString target_path = target_dir + "/" + src_info.fileName();

        if (QFile::exists(target_path)) {
            int ret = QMessageBox::question(this, tr("文件已存在"),
                                        tr("目标笔记中已有同名文件，是否覆盖？"),
                                        QMessageBox::Yes | QMessageBox::No);
            if (ret != QMessageBox::Yes) return;
            QFile::remove(target_path);
        }

        if (src_path == _current_path) {
            if (QFile::rename(src_path, target_path)) {
                _current_path = target_path;
                setWindowTitle(QFileInfo(target_path).fileName() + "[*] - Markedit");
                QMessageBox::information(this, tr("成功"), tr("文件已移动。"));
            } else {
                QMessageBox::warning(this, tr("错误"), tr("移动失败。"));
            }
        } else {
            if (!QFile::rename(src_path, target_path)) {
                QMessageBox::warning(this, tr("错误"), tr("移动失败。"));
            } else {
                QMessageBox::information(this, tr("成功"), tr("文件已移动。"));
            }
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
    QString new_name = QInputDialog::getText(this, tr("重命名文件"),
                                            tr("输入新文件名（保留 .md 后缀）:"),
                                            QLineEdit::Normal,
                                            old_info.fileName(), &ok);
    if (!ok || new_name.isEmpty()) return;

    if (!new_name.endsWith(".md", Qt::CaseInsensitive)) {
        new_name += ".md";
    }

    QString new_path = old_info.absolutePath() + "/" + new_name;

    if (QFile::exists(new_path)) {
        QMessageBox::warning(this, tr("错误"), tr("该名称已存在，请重新输入。"));
        return;
    }

    if (old_path == _current_path) {
        if (QFile::rename(old_path, new_path)) {
            _current_path = new_path;
            setWindowTitle(new_name + "[*] - Markedit");
            QMessageBox::information(this, tr("成功"), tr("文件已重命名。"));
        } else {
            QMessageBox::warning(this, tr("错误"), tr("重命名失败。"));
        }
    } else {
        if (QFile::rename(old_path, new_path)) {
            QMessageBox::information(this, tr("成功"), tr("文件已重命名。"));
        } else {
            QMessageBox::warning(this, tr("错误"), tr("重命名失败。"));
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

    QString save_path = QFileDialog::getSaveFileName(this, tr("另存为"),
                                                    src_info.fileName(),
                                                    "Markdown files (*.md)");
    if (save_path.isEmpty()) return;

    if (QFile::copy(src_path, save_path)) {
        QMessageBox::information(this, tr("成功"), tr("文件已另存。"));
    } else {
        QMessageBox::warning(this, tr("错误"), tr("另存失败，请检查目标路径。"));
    }
}
