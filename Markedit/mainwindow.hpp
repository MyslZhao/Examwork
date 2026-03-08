/**
 ****************************************************************************
 * Copyright (C), 2026, MyslZhao 159505988+MyslZhao@users.noreply.github.com
 *
 * @file mainwindow.hpp
 * @brief 主窗口所有方法和涉及到的变量声明文件
 * @details
 * 主窗口类(MainWindow)下相关方法和变量包括了：
 * + 全局/主窗口事件，即编辑框语法渲染、预览框实时渲染和相关快捷键的响应。
 * + 相关控件事件，包括导航栏按钮绑定事件等。
 * + 部分功能复杂的控件，如行号控件。
 * + 当前编辑文件的相关信息，包括文件名、文件路径等。
 * @author MyslZhao
 */

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QListWidgetItem>
#include <QStackedWidget>
#include <QTextDocument>
#include <QCheckBox>
// -*- encoding: utf-8 -*-

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 主窗口槽事件
    /// 预览渲染
    void _updatePreview();
    /// 跟随光标预览
    void _syncPreviewScroll();

    // 导航栏File下拉菜单槽事件
    /// 新建文件
    void newFile();
    /// 打开文件
    void openFile();
    /// 保存文件
    bool saveFile();
    /// 另存为
    bool saveFileAs();
    /// 编辑中断询问
    bool _maybeSave();
    /// 载入目标文件
    void _loadFile(const QString &file_name);
    /// 设置/更改文件名
    void _setCurrentFileName(const QString &file_name);
    /// 检测文件更改
    void _documentWasModified();
    /// 保存至
    bool _saveTo(const QString &file_name);

    // 导航栏Edit下拉菜单槽事件
    // NOTE: “撤销”与“重做”使用QPlainTextEdit自带功能，
    //      不再另设方法。
    /// 显示“查找”对话框
    void showFindDialog();
    /// 显示“替换”对话框
    void showReplaceDialog();
    /// 查找下一项
    void findNext(const QString &text, QTextDocument::FindFlags flags, bool use_regex);
    /// 替换
    void replace(const QString &target_text, const QString &new_text, QTextDocument::FindFlags flags, bool use_regex);
    /// 全部替换
    void replaceAll(const QString &target_text, const QString &new_text, QTextDocument::FindFlags flags, bool use_regex);
    /// 获取标志组
    QTextDocument::FindFlags _getFlags(QCheckBox *case_check, QCheckBox *whole_check);
    /// 显示RegEx表达式错误
    void _showRegexError(const QString &errormsg);

    // 侧边栏"Outline"相关事件
    /// 更新“大纲”
    void _updateOutline();
    /// “大纲”按钮触发事件
    void onOutlineItemClicked(QListWidgetItem *item);

    // 状态栏
    void _updateCursorPos();
    void onEncodingBtnClicked();
    void _handleEncodingAction(const QString &encoding, bool reopen);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::MainWindow *ui;

    // 活动栏
    /// 活动栏停靠窗口
    QDockWidget *activity_dock;
    /// 活动栏功能条
    QToolBar *activity_bar;

    // 侧边栏
    /// 侧边栏停靠窗口
    QDockWidget *side_dock;
    /// 侧边栏堆叠面板
    QStackedWidget *side_stack;

    // 活动栏按钮动作
    /// 大纲按钮事件
    QAction *outline_action;

    // 可堆叠面板
    /// 大纲面板
    QListWidget *outline_list;

    // 其他相关变量
    /// 文件路径
    QString _current_path;
    /// 是否为新文件
    bool _is_untitled;
    /// 条目与块号映射
    QMap<QListWidgetItem*, int> outline_pos;


};
#endif // MAINWINDOW_HPP
