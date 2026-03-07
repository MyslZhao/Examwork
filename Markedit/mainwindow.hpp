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
    //主窗口槽事件
    /// 预览渲染
    void _updatePreview();
    /// 跟随光标预览
    void _syncPreviewScroll();

    //导航栏File下拉菜单槽事件
    ///新建文件
    void newFile();
    ///打开文件
    void openFile();
    ///保存文件
    bool saveFile();
    ///另存为
    bool saveFileAs();
    ///编辑中断询问
    bool _maybeSave();
    ///载入目标文件
    void _loadFile(const QString &file_name);
    ///设置/更改文件名
    void _setCurrentFileName(const QString &file_name);
    ///检测文件更改
    void _documentWasModified();
    ///保存至
    bool _saveTo(const QString &file_name);

    //导航栏Edit下拉菜单槽事件
    void _showFindDialog();
    void _showReplaceDialog();
    void findNext(const QString &text, QTextDocument::FindFlags flags);
    void replace(const QString &findText, const QString &replaceText, QTextDocument::FindFlags flags);
    void replaceAll(const QString &findText, const QString &replaceText, QTextDocument::FindFlags flags);

    //侧边栏"Outline"相关事件
    void _updateOutline();
    void onOutlineItemClicked(QListWidgetItem *item);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::MainWindow *ui;

    /// 文件路径
    QString _current_path;
    /// 是否为新文件
    bool isUntitled;

    QDockWidget *activity_dock;      // 活动栏停靠窗口
    QToolBar *activity_bar;              // 活动栏工具栏
    QDockWidget *sideBarDock;           // 侧边栏停靠窗口
    QStackedWidget *sideStack;          // 堆叠面板（存放多个页面）
    QListWidget *outlineList;           // 大纲列表
    QMap<QListWidgetItem*, int> outlinePositions; // 条目与块号映射

    // 活动栏按钮动作
    QAction *outlineAction;
};
#endif // MAINWINDOW_HPP
