/**
 ****************************************************************************
 * Copyright (C), 2026, MyslZhao 159505988+MyslZhao@users.noreply.github.com
 *
 * @file    menuFileHandle.cpp
 * @brief   MenuBar导航栏“File”下拉项操作相关方法的实现
 *
 * @author  MyslZhao
 */
#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QCloseEvent>
#include <QFileInfo>
#include <QStatusBar>
#include <QInputDialog>
#include <QLineEdit>

/**
 * @brief 新建一个文件
 */
void MainWindow::newFile()
{
    if (_maybeSave())
    {
        ui -> markdownEdit -> clear();
        ui -> markdownEdit -> document() -> setModified(false);
        _is_untitled = true;
        _current_path.clear();
        _setCurrentFileName(QString());
    }
}

/**
 * @brief 打开一个Markdown源码文件
 */
void MainWindow::openFile()
{
    if (_maybeSave())
    {
        QString file_name = QFileDialog::getOpenFileName(this,
                                                        QObject::tr("打开Markdown源码文件"),
                                                        QString(),
                                                        "Markdown files (*.md);;All files (*.*)"
                                                        );
        if (!file_name.isEmpty())
        {
            _loadFile(file_name);
        }
    }
}

/**
 * @brief 保存当前文件
 * @return 是否保存成功
 */
bool MainWindow::saveFile()
{
    if (_is_untitled)
    {
        return saveFileAs();
    }
    else
    {
        return _saveTo(_current_path);
    }
}

/**
 * @brief 把文件另存为(.md\.txt)
 * @return 是否成功另存为
 */
bool MainWindow::saveFileAs()
{
    QString file_name = QFileDialog::getSaveFileName(this,
                                                     QObject::tr("另存为"),
                                                     _current_path.isEmpty() ? "未命名.md" : _current_path,
                                                    "Markdown files (*.md *.txt);;All files (*.*)"
                                                     );
    if (file_name.isEmpty())
        return false;

    return _saveTo(file_name);
}

/**
 * @brief 新建笔记操作
 */
void MainWindow::newNote()
{
    bool pass;
    QString note_name = QInputDialog::getText(this,
                                              QObject::tr("新建笔记"),
                                              QObject::tr("输入笔记名字:"),
                                              QLineEdit::Normal,
                                              "",
                                              &pass
                                              );

    if (!pass || note_name.isEmpty())
    {
        return;
    }

    QString note_path = _notes_path + "/" + note_name;
    QDir dir;
    if (!dir.mkpath(note_path))
    {
        QMessageBox::warning(this,
                             QObject::tr("错误"),
                             QObject::tr("无法新建笔记")
                             );
        return;
    }

    QString default_path = note_path + "/intro.md";
    QFile file(default_path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        file.write("# Intro for your notes");
        file.close();
    }

    _loadFile(default_path);

    _current_note_path = note_path;
    _note_name -> setText(note_name);

    notes_tree -> expand(_notes_system -> index(note_path));
}

/**
 * @brief 添加到笔记操作
 */
void MainWindow::addToNote()
{
    if(_current_note_path.isEmpty())
    {
        QMessageBox::information(this,
                                 QObject::tr("提示"),
                                 QObject::tr("请先在侧边栏选择一个笔记。")
                                 );
        return;
    }

    if (_is_untitled)
    {
        QString base_name = "intro.md";
        QString target_path = _current_note_path + "/" + base_name;

        if (QFile::exists(target_path))
        {
            int i = 1;
            while (QFile::exists(_current_note_path + "/intro" + QString::number(i) + ".md"))
            {
                i += 1;
            }
            target_path = _current_note_path + "/intro" + QString::number(i) + ".md";
        }

        if (_saveTo(target_path))
        {
            QMessageBox::information(this,
                                     QObject::tr("成功"),
                                     QObject::tr("文件以保存到笔记。")
                                     );
        }
        else
        {
            QMessageBox::warning(this,
                                 QObject::tr("错误"),
                                 QObject::tr("保存失败。")
                                 );
        }
        return;
    }

    QString file_name = QFileInfo(_current_path).fileName();
    QString target_path = _current_note_path + "/" + file_name;

    if (QFileInfo(_current_path).absolutePath() == _current_note_path)
    {
        QMessageBox::information(this,
                                 QObject::tr("提示 "),
                                 QObject::tr("文件以存在于该笔记中。")
                                 );
        return;
    }

    if (QFile::exists(target_path))
    {
        int ret = QMessageBox::question(this,
                                        QObject::tr("文件已存在"),
                                        QObject::tr("该笔记中已存在同名文件，是否覆盖？"),
                                        QMessageBox::Yes | QMessageBox::No
                                        );

        if (ret == QMessageBox::No)
        {
            return;
        }
        QFile::remove(target_path);
    }

    // HACK: 此处使用复制的方式实现加入笔记功能
    //      可能造成高空间占用
    if (QFile::copy(_current_path, target_path))
    {
        QMessageBox::information(this,
                                 QObject::tr("成功"),
                                 QObject::tr("成功添加文件到笔记")
                                 );
    }
    else
    {
        QMessageBox::warning(this,
                             QObject::tr("错误"),
                             QObject::tr("添加失败")
                             );
    }
}

/**
 * @brief 丢弃笔记
 */
void MainWindow::deleteNote()
{
    if (_current_note_path.isEmpty())
    {
        QMessageBox::information(this,
                                 QObject::tr("提示"),
                                 QObject::tr("请在侧边栏打开一个笔记。")
                                 );
        return;
    }

    QString path = _current_note_path;
    QFileInfo info(path);

    int ret = QMessageBox::question(this,
                                    QObject::tr("确认丢弃"),
                                    QString(QObject::tr("确定要丢弃笔记 \"%1\" 吗？之后将无法找回。"))
                                    .arg(info.fileName()),
                                    QMessageBox::Yes | QMessageBox::No
                                    );
    if (ret != QMessageBox::Yes)
    {
        return;
    }

    QDir dir(path);
    if (dir.removeRecursively())
    {
        QMessageBox::information(this,
                                 QObject::tr("成功"),
                                 QObject::tr("笔记已删除。")
                                 );
        _current_note_path.clear();
        _note_name -> setText(QObject::tr("未打开笔记"));

        notes_tree -> setRootIndex(_notes_system -> index(_notes_path));
    }
    else
    {
        QMessageBox::warning(this,
                             QObject::tr("错误"),
                             QObject::tr("删除失败，请检查该笔记文件夹是否被其他程序占用。")
                             );
    }
}

/**
 * @brief 从笔记中移除文件
 */
void MainWindow::removeFromNote()
{
    if (_current_note_path.isEmpty())
    {
        QMessageBox::information(this,
                                 QObject::tr("提示"),
                                 QObject::tr("请先打开笔记")
                                 );
        return;
    }

    QStringList files = QFileDialog::getOpenFileNames(this,
                                                      QObject::tr("选择要移除的文件"),
                                                      _current_note_path,
                                                      "Markdown file (*.md)"
                                                      );
    if (files.isEmpty())
    {
        return;
    }

    _whetherSave(files);
}

/**
 * @brief 被移除文件如何后续处理
 */
void MainWindow::_whetherSave(const QStringList &files)
{
    QMessageBox quirey(this);
    quirey.setWindowTitle(QObject::tr("移除文件"));
    quirey.setText(QObject::tr("选择被移除文件的处理方式:");

    QPushButton *delete_btn = quirey.addButton(QObject::tr("直接删除"), QMessageBox::DestructiveRole);
    QPushButton *presave_btn = quirey.addButton(QObject::tr("另存为后删除"), QMessageBox::ActionRole);
    QPushButton *cancel_btn = quirey.addButton(QObject::tr("取消"), QMessageBox::RejectRole);
    auto result = quirey.exec();

    if (result == QDialog::Rejected || quirey.clickedButton() == cancel_btn)
    {
        return;
    }

    bool need_save = (quirey.clickedButton() == presave_btn);
    QString save_dir;
    if (need_save)
    {
        save_dir = QFileDialog::getExistingDirectory(this, QObject::tr("选择保存位置"), QDir::homePath());
        if (save_dir.isEmpty())
        {
            return;
        }
    }

    int amount = 0;
    for (const QString &file_path: files)
    {
        QFileInfo info(file_path);
        if (need_save)
        {
            QString target_path = save_dir + "/" + info.fileName();
            if (QFile::exists(target_path))
            {
                int ret = QMessageBox::question(this,
                                                QObject::tr("文件已存在"),
                                                QString(QObject::tr("文件 \"%1\" 已存在，是否覆盖？"))
                                                    .arg(info.fileName()),
                                                QMessageBox::Yes | QMessageBox::No
                                                );
                if (ret == QMessageBox::No)
                {
                    continue;
                }
                QString temp_path = save_dir + "/temp_" + info.fileName();

                if (!QFile::copy(file_path, temp_path))
                {
                    QMessageBox::warning(this,
                                         QObject::tr("保存失败"),
                                         QString(QObject::tr("文件 \"%1\" 移动失败。"))
                                             .arg(info.fileName())
                                         );
                    continue;
                }

                if (QFile::exists(target_path) && !QFile::remove(target_path))
                {
                    QMessageBox::warning(this,
                                         QObject::tr("同名冲突"),
                                         QObject::tr("存在同名文件")
                                         );
                    QFile::remove(temp_path);
                    continue;
                }

                if (!QFile::rename(temp_path, target_path))
                {
                    QMessageBox::warning(this,
                                         QObject::tr("同名冲突"),
                                         QObject::tr("存在同名文件")
                                         );
                    QFile::remove(temp_path);
                    continue;
                }
            }
            else
            {
                if (!QFile::copy(file_path, target_path))
                {
                    QMessageBox::warning(this,
                                         QObject::tr("错误"),
                                         QString(QObject::tr("无法复制文件 \"%1\" 到目标位置。"))
                                             .arg(info.fileName())
                                         );
                    continue;
                }
            }
        }

        QFile file(file_path);
        if (!file.remove())
        {
            QMessageBox::warning(this,
                                 QObject::tr("错误"),
                                 QString(QObject::tr("无法删除文件。"))
                                    .arg(info.fileName())
                                 );
        }
        else
        {
            amount += 1;
        }
    }

    QMessageBox::information(this,
                             QObject::tr("完成"),
                             QString(QObject::tr("成功移除 %1 个文件。"))
                                .arg(amount)
                             );
}

/**
 * @brief 保存到文件
 * @param[in] file_name 目标文件绝对路径
 * @return 是否成功保存
 */
bool MainWindow::_saveTo(const QString &file_name)
{
    QFile file(file_name);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this,
                             QObject::tr("错误"),
                             QObject::tr("无法保存文件: ") + file.errorString()
                             );
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(_stringToEncoding(_current_encoding));
    stream << (ui -> markdownEdit -> toPlainText());
    file.close();

    ui -> markdownEdit -> document() -> setModified(false);
    _is_untitled = false;
    _setCurrentFileName(file_name);
    statusBar()->showMessage(QObject::tr("文件已保存"), 2000);
    return true;
}

/**
 * @brief 当编辑操作被打断时询问如何处理未保存的文件
 * @return 是否继续，进入下一步操作
 */
bool MainWindow::_maybeSave()
{
    if (ui -> markdownEdit -> document() -> isModified())
    {
        QMessageBox::StandardButton query;
        query = QMessageBox::warning(this,
                                     QObject::tr("提示"),
                                   QObject::tr("是否保存已被修改的文件？"),
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
                                     );
        if (query == QMessageBox::Save)
        {
            return saveFile();
        }
        else if (query == QMessageBox::Cancel)
        {
            return false;
        }
    }
    return true;
}

/**
 * @brief 向编辑框载入文件内容
 * @param[in] file_name 目标文件的绝对路径
 */
void MainWindow::_loadFile(const QString &file_name)
{
    QFile file(file_name);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this,
                             QObject::tr("错误"),
                             QObject::tr("该文件无法编辑: ") + file.errorString()
                             );
        return;
    }

    QTextStream stream(&file);
    stream.setEncoding(_stringToEncoding(_current_encoding));
    QString content = stream.readAll();
    file.close();

    ui -> markdownEdit -> setPlainText(content);
    ui -> markdownEdit -> document() -> setModified(false);
    _is_untitled = false;
    _setCurrentFileName(file_name);
    statusBar()->showMessage(QObject::tr("文件加载完毕"), 2000);
}

/**
 * @brief 更新主窗口文件名显示
 * @param[in] fileName 目标文件的绝对路径
 */
void MainWindow::_setCurrentFileName(const QString &file_name)
{
    _current_path = file_name;
    QString shown_name = file_name.isEmpty() ? "未命名" : QFileInfo(file_name).fileName();
    setWindowTitle(shown_name + "[*] - Markedit");
}

/**
 * @brief 检测文件是否改动
 */
void MainWindow::_documentWasModified()
{
    setWindowModified(ui -> markdownEdit -> document() -> isModified());
}

/**
 * @brief 关闭软件
 * @param[in] event 关闭事件
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (_maybeSave()) {
        event -> accept();
    } else {
        event -> ignore();
    }
}
