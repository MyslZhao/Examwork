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

/**
 * @brief 新建一个文件
 */
void MainWindow::newFile()
{
    if (_maybeSave())
    {
        ui->markdownEdit->clear();
        ui->markdownEdit->document()->setModified(false);
        isUntitled = true;
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
                                                        "打开Markdown源码文件",
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
    if (isUntitled)
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
                                                     "另存为",
                                                    _current_path.isEmpty() ? "未命名.md" : _current_path,
                                                    "Markdown files (*.md *.txt);;All files (*.*)"
                                                     );
    if (file_name.isEmpty())
        return false;

    return _saveTo(file_name);
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
                             "错误",
                             "无法保存文件: " + file.errorString()
                             );
        return false;
    }

    QTextStream stream(&file);
    stream << ui->markdownEdit->toPlainText();
    file.close();

    ui->markdownEdit->document()->setModified(false);
    isUntitled = false;
    _setCurrentFileName(file_name);
    statusBar()->showMessage("文件已保存", 2000);
    return true;
}

/**
 * @brief 当编辑操作被打断时询问如何处理未保存的文件
 * @return 是否继续，进入下一步操作
 */
bool MainWindow::_maybeSave()
{
    if (ui->markdownEdit->document()->isModified())
    {
        QMessageBox::StandardButton query;
        query = QMessageBox::warning(this,
                                     "Markedit",
                                   "是否保存已被修改的文件？",
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
                             "错误",
                             "该文件为二进制文件，无法编辑: " + file.errorString()
                             );
        return;
    }

    QTextStream stream(&file);
    QString content = stream.readAll();
    file.close();

    ui->markdownEdit->setPlainText(content);
    ui->markdownEdit->document()->setModified(false);
    isUntitled = false;
    _setCurrentFileName(file_name);
    statusBar()->showMessage("文件加载完毕", 2000);
}

/**
 * @brief 更新主窗口文件名显示
 * @param fileName
 */
void MainWindow::_setCurrentFileName(const QString &fileName)
{
    _current_path = fileName;
    QString shownName = fileName.isEmpty() ? "未命名" : QFileInfo(fileName).fileName();
    setWindowTitle(shownName + "[*] - Markedit");
}

/**
 * @brief 检测文件是否改动
 */
void MainWindow::_documentWasModified()
{
    setWindowModified(ui->markdownEdit->document()->isModified());
}

/**
 * @brief 关闭软件
 * @param[in] event 关闭事件
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (_maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}
