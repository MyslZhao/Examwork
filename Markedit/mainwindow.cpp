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
using namespace std;
// -*- encoding: utf-8 -*-

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
    new Highlighter(ui->markdownEdit->document());

    // HACK:无法在.ui中添加Qsplliter组件，手动设置Qsplitter
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(ui->markdownEdit);
    splitter->addWidget(ui->previewBrowser);
    setCentralWidget(splitter);
    splitter->setSizes({400, 200});

    ui->previewBrowser->setHtml("<h2 style = 'color: #545555'>"
                                "Preview Area"
                                "</h2>"
                                "<p style = 'color: #656565'>"
                                "Start writing Markdown on the left..."
                                "</p>");

    // 侧边栏部分
    activity_dock = new QDockWidget(this);
    activity_dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    activity_dock->setTitleBarWidget(new QWidget());
    activity_dock->setFixedWidth(50);
    addDockWidget(Qt::LeftDockWidgetArea, activity_dock);

    activity_bar = new QToolBar(activity_dock);
    activity_bar->setOrientation(Qt::Vertical);
    activity_bar->setIconSize(QSize(32, 32));
    activity_bar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    activity_dock->setWidget(activity_bar);

    // 侧边浮动栏
    side_dock = new QDockWidget(this);
    side_dock->setFeatures(QDockWidget::DockWidgetClosable);
    side_dock->setTitleBarWidget(new QWidget());
    side_dock->hide();
    addDockWidget(Qt::LeftDockWidgetArea, side_dock);

    splitDockWidget(activity_dock, side_dock, Qt::Horizontal);

    side_stack = new QStackedWidget(side_dock);
    side_dock->setWidget(side_stack);

    // "大纲"功能
    outline_action = activity_bar->addAction("大纲");
    outline_action->setCheckable(true);
    outline_action->setChecked(false);

    outline_list = new QListWidget;
    side_stack->addWidget(outline_list);

    // 全局事件连接
    connect(ui->markdownEdit, &QPlainTextEdit::textChanged, this,
            &MainWindow::_documentWasModified);
    connect(ui->markdownEdit, &QPlainTextEdit::textChanged, this,
            &MainWindow::_updatePreview);
    connect(ui->markdownEdit, &QPlainTextEdit::cursorPositionChanged,
            this, &MainWindow::_syncPreviewScroll);

    // "文件"菜单连接
    connect(ui->action_new, &QAction::triggered, this, &MainWindow::newFile);
    connect(ui->action_open, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->action_save, &QAction::triggered, this, &MainWindow::saveFile);
    connect(ui->action_saveas, &QAction::triggered, this, &MainWindow::saveFileAs);
    connect(ui->action_exit, &QAction::triggered, this, &QWidget::close);

    // "编辑"菜单连接
    connect(ui->action_undo, &QAction::triggered, ui->markdownEdit, &QPlainTextEdit::undo);
    connect(ui->action_redo, &QAction::triggered, ui->markdownEdit, &QPlainTextEdit::redo);
    connect(ui->action_find, &QAction::triggered, this, &MainWindow::showFindDialog);
    connect(ui->action_replace, &QAction::triggered, this, &MainWindow::showReplaceDialog);

    // “大纲”功能连接
    connect(outline_action, &QAction::toggled, this, [this](bool checked)
        {
            if (checked)
            {
                side_dock->show();
                side_stack->setCurrentWidget(outline_list);
            }
            else
            {
                side_dock->hide();
            }
        }
    );

    connect(side_dock, &QDockWidget::visibilityChanged, this, [this](bool visible)
        {
            if (!visible)
            {
                outline_action->setChecked(false);
            }
        }
    );

    connect(ui->markdownEdit, &QPlainTextEdit::textChanged,
            this, &MainWindow::_updateOutline);

    connect(outline_list, &QListWidget::itemClicked,
            this, &MainWindow::onOutlineItemClicked);

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
    QString markdown_text = ui->markdownEdit->toPlainText();
    if (markdown_text.isEmpty()) {
        ui->previewBrowser->setHtml("<p style = 'color: #656565'>"
                                    "<i>Nothing to preview...</i>"
                                    "</p>");
        return;
    }

    ui->previewBrowser->setMarkdown(markdown_text);
}

void MainWindow::_syncPreviewScroll()
{
    // 获取编辑区的总块数（行数）
    int total_blocks = ui->markdownEdit->document()->blockCount();
    if (total_blocks <= 1) return;

    // 获取当前光标所在的块号
    int current_block = ui->markdownEdit->textCursor().blockNumber();

    // 计算比例
    double ratio = static_cast<double>(current_block) / (total_blocks - 1);

    // 获取预览区的垂直滚动条
    QScrollBar *preview_vertiscroll = ui->previewBrowser->verticalScrollBar();
    int max_scroll = preview_vertiscroll->maximum();
    preview_vertiscroll->setValue(static_cast<int>(ratio * max_scroll));
}

