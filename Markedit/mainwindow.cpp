/**
 ****************************************************************************
 * Copyright (C), 2026, MyslZhao 159505988+MyslZhao@users.noreply.github.com
 *
 * @file    mainwindow.cpp
 * @brief   主窗口入口和槽方法
 *
 * @author  MyslZhao
 */
#include "mainwindow.hpp"
#include "./ui_mainwindow.h"
#include <QSplitter>
#include <cmark.h>
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

    connect(ui->markdownEdit, &QPlainTextEdit::textChanged,
            this, &MainWindow::updatePreview);

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
void MainWindow::updatePreview()
{
    QString markdownText = ui->markdownEdit->toPlainText();
    if (markdownText.isEmpty()) {
        ui->previewBrowser->setHtml("<p><i>Nothing to preview...</i></p>");
        return;
    }

    QByteArray utf8Text = markdownText.toUtf8();
    char *htmlOutput = cmark_markdown_to_html(utf8Text.constData(),
                                              utf8Text.size(),
                                              CMARK_OPT_DEFAULT);
    QString html = QString::fromUtf8(htmlOutput);
    ui->previewBrowser->setHtml(html);
    free(htmlOutput);
}

