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
#include <QLabel>
#include <QPushButton>
#include <QTextStream>
#include <QHash>
#include <QTreeView>
#include <QFileSystemModel>
#include <QStandardItemModel>
#include <QTranslator>
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
    // NOTE: 部分本不属于slots的方法也会放在此块内
    //      方便同一管理各个部分的功能

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
    /// 新建笔记
    void newNote();
    /// 添加到笔记
    void addToNote();
    /// 丢弃笔记
    void deleteNote();
    /// 从笔记中移除文件
    void removeFromNote();
    /// 询问怎么处理被移除的文件
    void _whetherSave(const QStringList &files);
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
    void onOutlineItemClicked(const QModelIndex &index);

    // 侧边栏"Notes"相关事件
    /// “笔记”按钮触发事件
    void onNoteTreeClicked(const QModelIndex &index);

    // 侧边栏"Settings"相关事件
    /// 显示设置窗口
    void showSettingsDialog();
    /// 加载设置
    void _loadSettings();
    /// 应用设置
    void _applyFontSettings(const QFont &font);
    /// 保存设置
    void _saveSettings(const QFont &font);
    /// 切换语言
    void _switchLanguage(const QString &langCode);

    // 状态栏
    /// 更新光标位置信息
    void _updateCursorPos();
    /// 编码按钮触发事件
    void onEncodingBtnClicked();
    /// 处理更改编码事件
    void _handleEncodingAction(const QString &encoding, bool reopen);
    /// 选择编码操作类型对话框
    bool _showEncodingOpe(bool &pass_status);
    /// 选择新编码对话框
    QString _showEncodingType(bool &pass_status);
    /// 执行重新打开操作
    void _handleReopenAction(const QString &encoding);
    /// 执行重新保存操作
    void _handleResaveAction(const QString &encoding);
    /// 从字符串加载编码类型
    QStringConverter::Encoding _stringToEncoding(const QString &name);

    // “笔记”右键菜单
    /// 右键菜单显示
    void showNotesMenu(const QPoint &pos);
    /// 移除文件
    void removeSelectedFile();
    /// 移动文件
    void moveSelectedFile();
    /// 重命名文件
    void renameSelectedFile();
    /// 另存为文件
    void saveAsSelectedFile();

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
    /// 笔记按钮事件
    QAction *notes_action;
    /// 设置按钮事件
    QAction *settings_action;

    // 可堆叠面板
    /// 大纲面板
    QTreeView *outline_tree;
    /// 笔记面板
    QTreeView *notes_tree;

    // 状态栏
    /// 光标位置组件
    QLabel *cursor_pos;
    /// 编码按钮
    QPushButton *encoding_btn;

    // 其他相关变量/辅助变量
    /// 翻译器
    QTranslator m_translator;
    /// 编码映射表
    static const QHash<QString, QStringConverter::Encoding> ENCOMAP;
    /// 文件路径(相对路径)
    QString _current_path;
    /// 是否为新文件
    bool _is_untitled;
    /// 文件编码
    QString _current_encoding;
    /// 笔记名字
    QLabel *_note_name;
    /// 储存所有笔记的目录(绝对路径)
    QString _notes_path;
    /// 当前笔记所在路径
    QString _current_note_path;
    /// 大纲层级模型
    QStandardItemModel *_outline_system;
    /// 文件系统模型
    QFileSystemModel *_notes_system;
};
#endif // MAINWINDOW_HPP
