/**
 ****************************************************************************
 * Copyright (C), 2026, MyslZhao 159505988+MyslZhao@users.noreply.github.com
 *
 * @file    barSettingsHandle.cpp
 * @brief   SideBar侧边栏“Settings”功能相关方法的实现
 *
 * @author  MyslZhao
 */
#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <QDialog>
#include <QFontComboBox>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QSettings>

/**
 * @brief 显示设置窗口
 */
void MainWindow::showSettingsDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle("设置");

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QFontComboBox *font_combo = new QFontComboBox;
    font_combo -> setCurrentFont(ui -> markdownEdit -> font());
    layout -> addWidget(new QLabel("字体:"));
    layout -> addWidget(font_combo);

    QSpinBox *size_spin = new QSpinBox;
    size_spin -> setRange(6, 72);
    size_spin -> setValue(ui -> markdownEdit -> font().pointSize());

    layout -> addWidget(new QLabel("字号:"));
    layout -> addWidget(size_spin);

    QDialogButtonBox *btn_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout -> addWidget(btn_box);

    connect(btn_box, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(btn_box, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted)
    {
        QFont selected_font = font_combo -> currentFont();
        selected_font.setPointSize(size_spin -> value());
        _applyFontSettings(selected_font);
        _saveSettings(selected_font);
    }
}

/**
 * @brief 应用设置
 * @param font[in] 应用字体
 */
void MainWindow::_applyFontSettings(const QFont &font)
{
    ui -> markdownEdit -> setFont(font);
    ui -> previewBrowser -> setFont(font);
    outline_tree -> setFont(font);
}

/**
 * @brief 加载设置
 */
void MainWindow::_loadSettings()
{
    QSettings settings("config.ini", QSettings::IniFormat);
    QString family = settings.value("font/family", ui -> markdownEdit -> font().family()).toString();
    int point_size = settings.value("font/size", ui -> markdownEdit -> font().pointSize()).toInt();

    QFont font(family, point_size);
    _applyFontSettings(font);
}

/**
 * @brief 保存设置
 * @param[in] font 设置的字体
 */
void MainWindow::_saveSettings(const QFont &font)
{
    QSettings settings("config.ini", QSettings::IniFormat);
    settings.setValue("font/family", font.family());
    settings.setValue("font/size", font.pointSize());
}
