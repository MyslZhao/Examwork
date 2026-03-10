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

    ui -> previewBrowser -> setHtml("<h2 style = 'color: #545555'>"
                                "Preview Area"
                                "</h2>"
                                "<p style = 'color: #656565'>"
                                "Start writing Markdown on the left..."
                                "</p>");

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

    // "大纲"功能按钮
    outline_action = activity_bar -> addAction("大纲");
    outline_action -> setCheckable(true);
    outline_action -> setChecked(false);

    outline_tree = new QTreeView;
    _outline_system = new QStandardItemModel(this);
    outline_tree -> setModel(_outline_system);
    outline_tree -> setHeaderHidden(true);
    outline_tree -> setIndentation(12);

    side_stack -> addWidget(outline_tree);

    // "笔记"功能按钮
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
    notes_tree -> hideColumn(1);
    notes_tree -> hideColumn(2);
    notes_tree -> hideColumn(3);
    notes_tree -> setRootIndex(_notes_system -> index(_notes_path));

    side_stack -> addWidget(notes_tree);

    // 状态栏
    QStatusBar *status = statusBar();

    cursor_pos = new QLabel(this);
    cursor_pos -> setText("行 1, 列 1   ");
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
    connect(ui -> action_exit, &QAction::triggered, this, &QWidget::close);

    // "编辑"菜单连接
    connect(ui -> action_undo, &QAction::triggered, ui -> markdownEdit, &QPlainTextEdit::undo);
    connect(ui -> action_redo, &QAction::triggered, ui -> markdownEdit, &QPlainTextEdit::redo);
    connect(ui -> action_find, &QAction::triggered, this, &MainWindow::showFindDialog);
    connect(ui -> action_replace, &QAction::triggered, this, &MainWindow::showReplaceDialog);

    // “大纲”功能连接
    connect(outline_action, &QAction::toggled, this, [this](bool checked)
        {
            if (checked)
            {
                side_stack -> setCurrentWidget(outline_tree);
                side_dock -> show();
                notes_action -> setChecked(false);
            }
            else
            {
                side_dock -> hide();
            }
        }
    );

    // “笔记”功能连接
    connect(notes_action, &QAction::toggled, this, [this](bool checked)
        {
            if (checked)
            {
                side_stack -> setCurrentWidget(notes_tree);
                side_dock -> show();
                outline_action -> setChecked(false);
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

    connect(outline_tree, &QTreeView::clicked,
            this, &MainWindow::onOutlineItemClicked);

    connect(notes_tree, &QTreeView::clicked,
            this, &MainWindow::onNoteTreeClicked);

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
        ui -> previewBrowser -> setHtml("<p style = 'color: #656565'>"
                                    "<i>Nothing to preview...</i>"
                                    "</p>");
        return;
    }

    ui -> previewBrowser -> setMarkdown(markdown_text);
}

/**
 * @brief 跟随光标跳转预览事件
 */
void MainWindow::_syncPreviewScroll()
{
    // 获取编辑区的总块数（行数）
    int total_blocks = ui -> markdownEdit -> document() -> blockCount();
    if (total_blocks <= 1) return;

    // 获取当前光标所在的块号
    int current_block = ui -> markdownEdit -> textCursor().blockNumber();

    // 计算比例
    double ratio = static_cast<double>(current_block) / (total_blocks - 1);

    // 获取预览区的垂直滚动条
    QScrollBar *preview_vertiscroll = ui -> previewBrowser -> verticalScrollBar();
    int max_scroll = preview_vertiscroll -> maximum();
    preview_vertiscroll -> setValue(static_cast<int>(ratio * max_scroll));
}

