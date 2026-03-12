/**
 ****************************************************************************
 * Copyright (C), 2026, MyslZhao 159505988+MyslZhao@users.noreply.github.com
 *
 * @file    menuEditHandle.cpp
 * @brief   MenuBar导航栏“Edit”下拉项操作相关方法的实现
 *
 * @author  MyslZhao
 */
#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QRegularExpression>
#include <QMessageBox>
#include <QPushButton>

/**
 * @brief 通过位掩码把多个标志整合为标志组
 * @param[in] case_check “大小写区分”复选框指针
 * @param[in] whole_check “全字匹配”复选框指针
 * @return 标志组
 */
QTextDocument::FindFlags MainWindow::_getFlags(QCheckBox *case_check, QCheckBox *whole_check)
{
    QTextDocument::FindFlags flags;

    // NOTE: 此处使用位掩码组合标志，下同
    if (case_check -> isChecked())
    {
        // |= : 按位或赋值运算符
        flags |= QTextDocument::FindCaseSensitively;
    }
    if (whole_check -> isChecked())
    {
        flags |= QTextDocument::FindWholeWords;
    }

    return flags;
}

/**
 * @brief 显示“查找”对话框
 * @details
 * 一个查找对话框，具有：
 * + 大小写区分查找
 * + 全词匹配
 * + RegEx查找
 *
 * 内部“查找”按钮与findNext绑定
 * 相关额外搜索功能标志通过位掩码传递
 *
 * 为保证搜索结果正确，在查找前强制要求文件已保存
 *
 * NOTE: 该对话框架构基本确定，尽量避免架构上的改动
 * NOTE: 考虑后续按钮改用图标
 */
void MainWindow::showFindDialog()
{
    if (!_maybeSave())
    {
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("查找");

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLineEdit *find_edit = new QLineEdit;
    QCheckBox *case_check = new QCheckBox("大小写区分");
    QCheckBox *whole_check = new QCheckBox("全词匹配");
    QCheckBox *regex_check = new QCheckBox("RegEx模式");

    QDialogButtonBox *btn_box = new QDialogButtonBox(QDialogButtonBox::Ok |
                                                       QDialogButtonBox::Cancel);

    layout -> addWidget(new QLabel("查找内容(支持正则):"));

    layout -> addWidget(find_edit);
    layout -> addWidget(case_check);
    layout -> addWidget(whole_check);
    layout -> addWidget(regex_check);

    layout -> addWidget(btn_box);

    connect(btn_box, &QDialogButtonBox::accepted, [&]()
        {

            QTextDocument::FindFlags flags = _getFlags(case_check, whole_check);

            findNext(find_edit -> text(), flags, regex_check -> isChecked());
            dialog.accept();

        }
    );
    connect(btn_box, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    dialog.exec();
}

/**
 * @brief 显示“替换”对话框
 * @details
 * 一个替换对话框，包括：
 * + 大小写区分查找
 * + 全词匹配
 * + RegEx查找
 *
 * 内部三个按钮分别与findNext、replace和replaceAll绑定
 *
 * NOTE: 该对话框架构基本确定，尽量避免架构上的改动
 * NOTE: 考虑后续按钮改用图标
 */
void MainWindow::showReplaceDialog()
{
    if (!_maybeSave())
    {
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("替换");

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLineEdit *find_edit = new QLineEdit;
    QLineEdit *replace_edit = new QLineEdit;

    QCheckBox *case_check = new QCheckBox("大小写区分");
    QCheckBox *whole_check = new QCheckBox("全局匹配");
    QCheckBox *regex_check = new QCheckBox("RegEx模式");

    QHBoxLayout *btn_layout = new QHBoxLayout;

    QPushButton *next_btn = new QPushButton("下一项");
    QPushButton *replace_btn = new QPushButton("替换");
    QPushButton *replacall_btn = new QPushButton("全部替换");
    QPushButton *close_btn = new QPushButton("关闭");

    layout -> addWidget(new QLabel("替换内容:"));
    layout -> addWidget(find_edit);
    layout -> addWidget(new QLabel("替换为:"));
    layout -> addWidget(replace_edit);

    layout -> addWidget(case_check);
    layout -> addWidget(whole_check);
    layout -> addWidget(regex_check);

    btn_layout -> addWidget(next_btn);
    btn_layout -> addWidget(replace_btn);
    btn_layout -> addWidget(replacall_btn);
    btn_layout -> addWidget(close_btn);

    layout -> addLayout(btn_layout);

    connect(next_btn, &QPushButton::clicked, [&]()
        {
            QTextDocument::FindFlags flags = _getFlags(case_check, whole_check);

            findNext(find_edit -> text(), flags, regex_check -> isChecked());
        }
    );

    connect(replace_btn, &QPushButton::clicked, [&]()
        {
            QTextDocument::FindFlags flags = _getFlags(case_check, whole_check);

            replace(find_edit -> text(), replace_edit -> text(), flags, regex_check -> isChecked());
        }
    );

    connect(replacall_btn, &QPushButton::clicked, [&]()
        {
            QTextDocument::FindFlags flags = _getFlags(case_check, whole_check);
            replaceAll(find_edit -> text(), replace_edit -> text(), flags, regex_check -> isChecked());
        }
    );

    connect(close_btn, &QPushButton::clicked, &dialog, &QDialog::reject);

    dialog.exec();
}

/**
 * @brief Regex 语法错误警告框
 * @param[in] errormsg Regex错误信息
 */
void MainWindow::_showRegexError(const QString &errormsg)
{
    QMessageBox::warning(this, "表达式错误", "无效的RegEx: " + errormsg);
    return;
}

/**
 * @brief “查找下一项”按钮功能
 * @param[in] text 匹配表达式/RegEx表达式
 * @param[in] flags 查找标志组
 * @param[in] use_regex 是否启用RegEx模式
 * @details
 * 带有光标指示的“下一项”功能
 *
 * NOTE: 该方法基本完成，应消极改动
 */
void MainWindow::findNext(const QString &text, QTextDocument::FindFlags flags, bool use_regex)
{
    if (text.isEmpty())
    {
        return;
    }

    bool found = false;

    if (use_regex)
    {
        QRegularExpression regex(text);

        if (!regex.isValid())
        {
            return _showRegexError(regex.errorString());
        }

        QPlainTextEdit *edit = ui->markdownEdit;
        QTextCursor cursor = edit -> textCursor();

        QTextCursor result = edit -> document() -> find(
            regex,
            cursor.selectionStart(),
            flags
            );

        if (!result.isNull())
        {
            edit -> setTextCursor(cursor);
            found = true;
        }
    }
    else
    {
        found = ui -> markdownEdit -> find(text, flags);
    }

    if (!found)
    {
        QMessageBox::information(this, "查找", "未找到目标项");
    }
}

/**
 * @brief 替换（一处）
 * @param[in] target_text 目标文本
 * @param[in] new_text 新文本
 * @param[in] flags 查找标志组
 * @param[in] use_regex 是否启用RegEx模式
 */
void MainWindow::replace(const QString &target_text, const QString &new_text, QTextDocument::FindFlags flags, bool use_regex)
{
    QTextCursor cursor = ui -> markdownEdit -> textCursor();
    bool has_selection = cursor.hasSelection();

    if (use_regex)
    {
        QRegularExpression regex(target_text);
        if (!regex.isValid())
        {
            return _showRegexError(regex.errorString());
        }

        if (has_selection)
        {
            QString selected = cursor.selectedText();
            QRegularExpressionMatch match = regex.match(selected);
            if (match.hasMatch() && match.captured(0) == selected)
            {
                cursor.insertText(new_text);
            }
        }

        findNext(target_text, flags, use_regex);
    }
    else
    {
        if (has_selection && cursor.selectedText() == target_text)
        {
            cursor.insertText(new_text);
        }
        findNext(target_text, flags, false);
    }
}

/**
 * @brief 替换（全部）
 * @param[in] target_text 目标文本
 * @param[in] new_text 新文本
 * @param[in] flags 查找标志组
 * @param[in] use_regex 是否启用RegEx模式
 */
void MainWindow::replaceAll(const QString &target_text, const QString &new_text, QTextDocument::FindFlags flags, bool use_regex)
{
    if (target_text.isEmpty())
    {
        return;
    }

    QTextCursor cursor(ui -> markdownEdit -> document());
    cursor.beginEditBlock();

    int counter = 0;
    if (use_regex)
    {
        QRegularExpression regex(target_text);
        if (!regex.isValid())
        {
            return _showRegexError(regex.errorString());
        }

        cursor = ui -> markdownEdit -> document() -> find(regex, 0, flags);
        while (!cursor.isNull())
        {
            cursor.insertText(new_text);
            counter += 1;
            cursor = ui -> markdownEdit -> document() -> find(target_text, cursor.position(), flags);
        }
    }

    cursor.endEditBlock();
    QMessageBox::information(this, "全局替换", QString("共替换 %1 处").arg(counter));
}
