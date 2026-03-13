/**
 ****************************************************************************
 * Copyright (C), 2026, MyslZhao 159505988+MyslZhao@users.noreply.github.com
 *
 * @file    mainwindow.cpp
 * @brief   主窗口入口和槽的建立
 * @details
 * 对窗口执行相关初始化操作，
 * 并绑定事件与对应控件/组件。
 *
 * @author  MyslZhao
 */
#include "mainwindow.hpp"
#include "highlighter.hpp"

#include "./ui_mainwindow.h"

#include <QSplitter>
#include <QToolBar>
#include <QDockWidget>
#include <QScrollBar>
#include <QTreeView>
#include <QDesktopServices>
#include <QMessageBox>
// -*- encoding: utf-8 -*-

const QHash<QString, QStringConverter::Encoding> MainWindow::ENCOMAP = {
    {"UTF-8", QStringConverter::Utf8},
    {"UTF-16", QStringConverter::Utf16},
    {"UTF-16BE", QStringConverter::Utf16BE},
    {"UTF-16LE", QStringConverter::Utf16LE},
    {"ISO-8859-1", QStringConverter::Latin1},
    {"ASCII", QStringConverter::Latin1}
};

/**
 * @brief ui界面处理入口
 * @param parent
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //初始化相关变量
    _current_path = QString();
    _is_untitled = true;
    setWindowTitle("Markedit[*]");
    new Highlighter(ui -> markdownEdit -> document());
    _current_encoding = "UTF-8";
    _notes_path = QApplication::applicationDirPath() + "/notes";
    QDir src_dir(_notes_path);
    if (!src_dir.exists())
    {
        src_dir.mkdir(_notes_path);
    }

    // HACK:无法在.ui中添加Qsplliter组件，手动设置Qsplitter
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    splitter -> addWidget(ui -> markdownEdit);
    splitter -> addWidget(ui -> previewBrowser);
    setCentralWidget(splitter);
    splitter -> setSizes({400, 200});

    ui -> previewBrowser -> setHtml(QObject::tr(
                                 "<h2 style = 'color: #545555'>"
                                 "预览区域"
                                 "</h2>"
                                 "<p style = 'color: #656565'>"
                                 "在左边开始写Markdown代码"
                                 "</p>"
        ));

    // 侧边栏部分
    activity_dock = new QDockWidget(this);
    activity_dock -> setFeatures(QDockWidget::NoDockWidgetFeatures);
    activity_dock -> setTitleBarWidget(new QWidget());
    activity_dock -> setFixedWidth(50);
    addDockWidget(Qt::LeftDockWidgetArea, activity_dock);

    activity_bar = new QToolBar(activity_dock);
    activity_bar -> setOrientation(Qt::Vertical);
    activity_bar -> setIconSize(QSize(32, 32));
    activity_bar -> setToolButtonStyle(Qt::ToolButtonIconOnly);
    activity_dock -> setWidget(activity_bar);

    // 侧边浮动栏
    side_dock = new QDockWidget(this);
    side_dock -> setFeatures(QDockWidget::DockWidgetClosable);
    side_dock -> setTitleBarWidget(new QWidget());
    side_dock -> hide();
    addDockWidget(Qt::LeftDockWidgetArea, side_dock);

    splitDockWidget(activity_dock, side_dock, Qt::Horizontal);

    side_stack = new QStackedWidget(side_dock);
    side_dock -> setWidget(side_stack);

    // "大纲"功能按钮及侧边显示
    outline_action = activity_bar -> addAction("大纲");
    outline_action -> setCheckable(true);
    outline_action -> setChecked(false);

    outline_tree = new QTreeView;
    _outline_system = new QStandardItemModel(this);
    outline_tree -> setModel(_outline_system);
    outline_tree -> setHeaderHidden(true);
    outline_tree -> setIndentation(12);

    side_stack -> addWidget(outline_tree);

    // "笔记"功能按钮及侧边显示
    notes_action = activity_bar -> addAction("笔记");
    notes_action -> setCheckable(true);
    notes_action -> setChecked(false);

    notes_tree = new QTreeView;
    _notes_system = new QFileSystemModel(this);

    _notes_system -> setRootPath("");
    _notes_system -> setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
    _notes_system -> setNameFilters(QStringList() << "*.md");
    _notes_system -> setNameFilterDisables(false);

    notes_tree -> setModel(_notes_system);
    notes_tree -> setHeaderHidden(true);
    notes_tree -> hideColumn(1);
    notes_tree -> hideColumn(2);
    notes_tree -> hideColumn(3);
    notes_tree -> setRootIndex(_notes_system -> index(_notes_path));
    notes_tree -> setContextMenuPolicy(Qt::CustomContextMenu);

    QWidget *note_page = new QWidget;
    QVBoxLayout *notes_layout = new QVBoxLayout(note_page);
    notes_layout -> setContentsMargins(0, 5, 0, 0);
    notes_layout -> setSpacing(2);

    _note_name = new QLabel(QObject::tr("当前未打开笔记"));
    _note_name -> setAlignment(Qt::AlignCenter);
    _note_name -> setStyleSheet("font-weight: bold; color: #888;");

    notes_layout -> addWidget(_note_name);

    notes_layout -> addWidget(notes_tree);

    side_stack -> addWidget(note_page);

    // "设置"功能按钮
    settings_action = activity_bar -> addAction("设置");
    settings_action -> setCheckable(false);

    // 状态栏
    QStatusBar *status = statusBar();

    cursor_pos = new QLabel(this);
    cursor_pos -> setText(QObject::tr("行 1, 列 1   "));
    status -> addPermanentWidget(cursor_pos);

    encoding_btn = new QPushButton("UTF-8", this);
    status -> addPermanentWidget(encoding_btn);

    // 全局事件连接
    connect(ui -> markdownEdit, &QPlainTextEdit::textChanged, this,
            &MainWindow::_documentWasModified);
    connect(ui -> markdownEdit, &QPlainTextEdit::textChanged, this,
            &MainWindow::_updatePreview);
    connect(ui -> markdownEdit, &QPlainTextEdit::cursorPositionChanged,
            this, &MainWindow::_syncPreviewScroll);

    // "文件"菜单连接
    connect(ui -> action_new, &QAction::triggered, this, &MainWindow::newFile);
    connect(ui -> action_open, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui -> action_save, &QAction::triggered, this, &MainWindow::saveFile);
    connect(ui -> action_saveas, &QAction::triggered, this, &MainWindow::saveFileAs);
    connect(ui -> action_mkdir, &QAction::triggered, this, &MainWindow::newNote);
    connect(ui -> action_cp, &QAction::triggered, this, &MainWindow::addToNote);
    connect(ui -> action_rmdir, &QAction::triggered, this, &MainWindow::deleteNote);
    connect(ui -> action_mov, &QAction::triggered, this, &MainWindow::removeFromNote);
    connect(ui -> action_exit, &QAction::triggered, this, &QWidget::close);

    // "编辑"菜单连接
    connect(ui -> action_undo, &QAction::triggered, ui -> markdownEdit, &QPlainTextEdit::undo);
    connect(ui -> action_redo, &QAction::triggered, ui -> markdownEdit, &QPlainTextEdit::redo);
    connect(ui -> action_find, &QAction::triggered, this, &MainWindow::showFindDialog);
    connect(ui -> action_replace, &QAction::triggered, this, &MainWindow::showReplaceDialog);

    // "帮助"菜单连接
    connect(ui -> action_guideline, &QAction::triggered, []()
        {
            QDesktopServices::openUrl(QUrl("https://markdown.com.cn/basic-syntax/"));
        }
    );
    connect(ui -> action_docs, &QAction::triggered, []()
        {
        QDesktopServices::openUrl(QUrl("https://github.com/MyslZhao/Examwork/blob/main/README.md"));
        }
    );
    connect(ui -> action_license, &QAction::triggered, []()
        {
            QDesktopServices::openUrl(QUrl("https://github.com/MyslZhao/Examwork/blob/main/LICENSE.md"));
        }
    );
    connect(ui -> action_report, &QAction::triggered, []()
        {
            QDesktopServices::openUrl(QUrl("https://github.com/MyslZhao/Examwork/issues"));
        }
    );
    connect(ui->action_about, &QAction::triggered, [this]() {
        QString aboutText = QObject::tr(
                             "<h2>Markedit</h2>"
                             "<p>版本 0.1</p>"
                             "<p>一个简单的 Markdown 编辑器，使用 Qt 6 和 C++17 编写。</p>"
                             "<p>项目主页：<a href='https://github.com/yourusername/yourrepo'>GitHub</a></p>"
                             "<p>Copyright © 2026 MyslZhao</p>"
                                        );
        QMessageBox::about(this, QObject::tr("关于 Markedit"), aboutText);
    });

    // “大纲”功能连接
    connect(outline_action, &QAction::toggled, this, [this](bool checked)
        {
            if (checked)
            {
                notes_action -> setChecked(false);
                side_stack -> setCurrentWidget(outline_tree);
                side_dock -> show();
            }
            else
            {
                side_dock -> hide();
            }
        }
    );

    // “笔记”功能连接
    connect(notes_action, &QAction::toggled, this, [this, note_page](bool checked)
        {
            if (checked)
            {
                outline_action -> setChecked(false);
                side_stack -> setCurrentWidget(note_page);
                side_dock -> show();
            }
            else
            {
                side_dock -> hide();
            }
        }
    );

    // 侧边栏相关连接
    connect(side_dock, &QDockWidget::visibilityChanged, this, [this](bool visible)
        {
            if (!visible)
            {
                outline_action -> setChecked(false);
            }
        }
    );

    // 主窗口全局事件连接
    connect(ui -> markdownEdit, &QPlainTextEdit::textChanged,
            this, &MainWindow::_updateOutline);

    // 光标行高亮事件
    connect(ui->markdownEdit, &QPlainTextEdit::cursorPositionChanged, this, [this](){
        QList<QTextEdit::ExtraSelection> extraSelections;
        if (!ui->markdownEdit->isReadOnly()) {
            QTextEdit::ExtraSelection selection;
            selection.format.setBackground(QColor(0x4C, 0x4C, 0x4C));
            selection.format.setProperty(QTextFormat::FullWidthSelection, true);
            selection.cursor = ui->markdownEdit->textCursor();
            selection.cursor.clearSelection();
            extraSelections.append(selection);
        }
        ui->markdownEdit->setExtraSelections(extraSelections);
    });

    // 侧边窗口点击事件
    connect(outline_tree, &QTreeView::clicked,
            this, &MainWindow::onOutlineItemClicked);

    connect(notes_tree, &QTreeView::clicked,
            this, &MainWindow::onNoteTreeClicked);

    connect(settings_action, &QAction::triggered,
            this, &MainWindow::showSettingsDialog);

    // 右键事件
    connect(notes_tree,
            &QTreeView::customContextMenuRequested,
            this, &MainWindow::showNotesMenu);

    // 编码按钮连接
    connect(encoding_btn, &QPushButton::clicked, this, &MainWindow::onEncodingBtnClicked);
    connect(ui -> markdownEdit, &QPlainTextEdit::cursorPositionChanged, this, &MainWindow::_updateCursorPos);

    // 初始更新大纲
    _updateOutline();
}

/**
 * @brief 析构主窗口对象
 */
MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * @brief 更新Markdown预览
 */
void MainWindow::_updatePreview()
{
    QString markdown_text = ui -> markdownEdit -> toPlainText();
    if (markdown_text.isEmpty()) {
        ui -> previewBrowser -> setHtml(QObject::tr(
            "<p style = 'color: #656565'>"
             "<i>没有文本可以预览...</i>"
             "</p>"
            ));
        return;
    }

    ui -> previewBrowser -> setMarkdown(markdown_text);
}

/**
 * @brief 跟随光标跳转预览事件
 */
void MainWindow::_syncPreviewScroll()
{
    int total_blocks = ui -> markdownEdit -> document() -> blockCount();
    if (total_blocks <= 1) return;

    int current_block = ui -> markdownEdit -> textCursor().blockNumber();

    double ratio = static_cast<double>(current_block) / (total_blocks - 1);

    QScrollBar *preview_vertiscroll = ui -> previewBrowser -> verticalScrollBar();
    int max_scroll = preview_vertiscroll -> maximum();
    preview_vertiscroll -> setValue(static_cast<int>(ratio * max_scroll));
}

