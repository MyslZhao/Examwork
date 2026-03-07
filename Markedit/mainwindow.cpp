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

// TODO: 实现导航栏"编辑"菜单按钮功能

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
    isUntitled = true;
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
    sideBarDock = new QDockWidget(this);
    sideBarDock->setFeatures(QDockWidget::DockWidgetClosable);
    sideBarDock->setTitleBarWidget(new QWidget());
    sideBarDock->hide();
    addDockWidget(Qt::LeftDockWidgetArea, sideBarDock);

    splitDockWidget(activity_dock, sideBarDock, Qt::Horizontal);

    sideStack = new QStackedWidget(sideBarDock);
    sideBarDock->setWidget(sideStack);

    // "大纲"功能
    outlineAction = activity_bar->addAction("大纲");
    outlineAction->setCheckable(true);
    outlineAction->setChecked(false);

    outlineList = new QListWidget;
    sideStack->addWidget(outlineList);

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
    connect(ui->action_find, &QAction::triggered, this, &MainWindow::_showFindDialog);
    connect(ui->action_replace, &QAction::triggered, this, &MainWindow::_showReplaceDialog);

    // “大纲”功能连接
    connect(outlineAction, &QAction::toggled, this, [this](bool checked) {
        if (checked) {
            sideBarDock->show();
            sideStack->setCurrentWidget(outlineList);
        } else {
            sideBarDock->hide();
        }
    });

    connect(sideBarDock, &QDockWidget::visibilityChanged, this, [this](bool visible) {
        if (!visible) {
            outlineAction->setChecked(false);
        }
    });

    connect(ui->markdownEdit, &QPlainTextEdit::textChanged,
            this, &MainWindow::_updateOutline);

    connect(outlineList, &QListWidget::itemClicked,
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
    QString markdownText = ui->markdownEdit->toPlainText();
    if (markdownText.isEmpty()) {
        ui->previewBrowser->setHtml("<p style = 'color: #656565'>"
                                    "<i>Nothing to preview...</i>"
                                    "</p>");
        return;
    }

    ui->previewBrowser->setMarkdown(markdownText);
}

void MainWindow::_syncPreviewScroll()
{
    // 获取编辑区的总块数（行数）
    int totalBlocks = ui->markdownEdit->document()->blockCount();
    if (totalBlocks <= 1) return;

    // 获取当前光标所在的块号
    int currentBlock = ui->markdownEdit->textCursor().blockNumber();

    // 计算比例
    double ratio = static_cast<double>(currentBlock) / (totalBlocks - 1);

    // 获取预览区的垂直滚动条
    QScrollBar *previewVScroll = ui->previewBrowser->verticalScrollBar();
    int maxScroll = previewVScroll->maximum();
    previewVScroll->setValue(static_cast<int>(ratio * maxScroll));
}

/**
 * @brief 更新文件大纲内容
 */
void MainWindow::_updateOutline()
{
    outlineList->clear();
    outlinePositions.clear();

    QString text = ui->markdownEdit->toPlainText();
    QStringList lines = text.split('\n');
    QRegularExpression re("^(\\s*)(#{1,6})\\s+(.*)$");
    int blockNumber = 0;
    for (const QString &line : lines) {
        QRegularExpressionMatch match = re.match(line);
        if (match.hasMatch()) {
            QString levelStr = match.captured(2);
            QString title = match.captured(3);
            int level = levelStr.length();

            // 根据级别缩进（每级两个空格）
            QString indent = QString(" ").repeated(level * 2);
            QListWidgetItem *item = new QListWidgetItem(indent + title, outlineList);
            QFont font = item->font();
            font.setBold(true);
            item->setFont(font);

            outlinePositions[item] = blockNumber;
        }
        ++blockNumber;
    }
}

void MainWindow::onOutlineItemClicked(QListWidgetItem *item)
{
    if (!outlinePositions.contains(item)) return;
    int blockNumber = outlinePositions[item];
    QTextBlock block = ui->markdownEdit->document()->findBlockByNumber(blockNumber);
    if (block.isValid()) {
        QTextCursor cursor(block);
        ui->markdownEdit->setTextCursor(cursor);
        ui->markdownEdit->ensureCursorVisible();
    }
}

