/**
 ****************************************************************************
 * Copyright (C), 2026, MyslZhao 159505988+MyslZhao@users.noreply.github.com
 *
 * @file    statusBarHandle.cpp
 * @brief   文件状态栏交互实现
 *
 * @author  MyslZhao
 */
#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <QDialog>
#include <QRadioButton>
#include <QDialogButtonBox>
#include <QStringList>
#include <QInputDialog>
#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QVBoxLayout>
#include <QLabel>
#include <QSettings>

/**
 * @brief 更新光标位置信息
 */
void MainWindow::_updateCursorPos()
{
    QTextCursor cursor = ui -> markdownEdit -> textCursor();
    int line = cursor.blockNumber() + 1;
    int col = cursor.columnNumber() + 1;
    cursor_pos -> setText(QString(tr("行 %1, 列 %2   ")).arg(line).arg(col));
}

/**
 * @brief 显示选择编码操作窗口
 * @param[out] pass_status 窗口操作是否通过
 * @return 是否选择“重新打开”操作(否即代表选择“保存”操作)
 */
bool MainWindow::_showEncodingOpe(bool &pass_status)
{
    QDialog dialog(this);
    dialog.setWindowTitle(tr("更改文件编码"));

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QRadioButton *reopen_with = new QRadioButton(tr("以新编码重新打开"));
    QRadioButton *resave_with = new QRadioButton(tr("以新编码保存"));
    reopen_with->setChecked(true);

    layout->addWidget(new QLabel(tr("选择编码处理方式:")));
    layout->addWidget(reopen_with);
    layout->addWidget(resave_with);

    QDialogButtonBox *button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(button_box);

    connect(button_box, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(button_box, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        pass_status = true;
        return reopen_with->isChecked();
    }

    pass_status = false;
    return false;
}

/**
 * @brief 显示选择编码类型
 * @param[out] pass_status 窗口操作是否通过
 * @return 选择的字符编码字符串
 */
QString MainWindow::_showEncodingType(bool &pass_status)
{
    QStringList encodings;
    encodings << "UTF-8" << "UTF-16" << "UTF-16BE" << "UTF-16LE" << "ISO-8859-1" << "ASCII";

    QString current = QSettings("config.ini", QSettings::IniFormat).value("encoding", "UTF-8").toString();
    int currentIndex = encodings.indexOf(current);
    if (currentIndex < 0) {
        currentIndex = 0;
    }

    QString encoding = QInputDialog::getItem(this, tr("选择编码类型"), tr("编码:"), encodings, currentIndex, false, &pass_status);
    return encoding;
}

/**
 * @brief 编码按钮交互事件
 */
void MainWindow::onEncodingBtnClicked()
{
    bool pass_status = false;

    bool reopen = _showEncodingOpe(pass_status);
    if (!pass_status)
    {
        return;
    }

    QString encoding = _showEncodingType(pass_status);
    if (!pass_status)
    {
        return;
    }

    _handleEncodingAction(encoding, reopen);
}

/**
 * @brief 从字符串映射编码
 * @param[in] name 编码名称
 * @return Qt的Encoding对象
 */
QStringConverter::Encoding MainWindow::_stringToEncoding(const QString &name)
{
    return ENCOMAP.value(name, QStringConverter::Utf8);
}

/**
 * @brief 处理用新编码重新打开事件
 * @param[in] encoding 新的编码
 */
void MainWindow::_handleReopenAction(const QString &encoding)
{
    if (_current_path.isEmpty())
    {
        QMessageBox::warning(this, tr("提示"), tr("未打开文件。"));
        return;
    }
    QFile file(_current_path);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, tr("错误"), tr("文件无法打开:") + file.errorString());
        return;
    }

    QTextStream stream(&file);
    stream.setEncoding(_stringToEncoding(encoding));
    QString content = stream.readAll();
    file.close();

    ui -> markdownEdit -> setPlainText(content);
    _current_encoding = encoding;
    encoding_btn -> setText(encoding);

    ui -> markdownEdit -> document() -> setModified(false);
    _is_untitled = false;
    setWindowModified(false);
}

/**
 * @brief 处理用新编码保存事件
 * @param[in] encoding 新的编码
 */
void MainWindow::_handleResaveAction(const QString &encoding)
{
    if (_current_path.isEmpty())
    {
        QString file_name = QFileDialog::getSaveFileName(this, QObject::tr("保存文件"), QString(), "All file (*.*)");
        if (file_name.isEmpty())
        {
            return;
        }
        _current_path = file_name;
    }
    QFile file(_current_path);
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(this, tr("错误"), tr("文件无法更改:") + file.errorString());
        return;
    }
    QTextStream stream(&file);
    stream.setEncoding(_stringToEncoding(encoding));
    stream << (ui -> markdownEdit -> toPlainText());
    file.close();

    encoding_btn -> setText(encoding);
    ui -> markdownEdit -> document() -> setModified(false);
    _is_untitled = false;
    setWindowModified(false);
}

/**
 * @brief 处理编码按钮交互事件入口
 * @param[in] encoding 新的编码
 * @param[in] reopen 是否选择“重新打开”操作(否即代表选择“保存”操作)
 */
void MainWindow::_handleEncodingAction(const QString &encoding, bool reopen)
{
    if (reopen)
    {
        _handleReopenAction(encoding);
    }
    else
    {
        _handleResaveAction(encoding);
    }
}
