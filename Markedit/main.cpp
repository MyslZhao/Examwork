/**
 ****************************************************************************
 * Copyright (C), 2026, MyslZhao 159505988+MyslZhao@users.noreply.github.com
 *
 * @file    main.cpp
 * @brief   主程序入口文件
 *
 * @author  MyslZhao
 */
#include "mainwindow.hpp"

#include <fstream>

#include <QApplication>
#include <QLocale>
#include <QTranslator>
using namespace std;

int main(int argc, char *argv[])
{
    try
    {
        QApplication a(argc, argv);

        QTranslator translator;
        const QStringList uiLanguages = QLocale::system().uiLanguages();
        for (const QString &locale : uiLanguages) {
            const QString baseName = "Markedit_" + QLocale(locale).name();
            if (translator.load(":/i18n/" + baseName)) {
                a.installTranslator(&translator);
                break;
            }
        }
        MainWindow w;
        w.show();
        return a.exec();
    }
    catch(const exception& e) // XXX:宽泛的错误处理, 后续会在测试阶段进一步精确
    {
        ofstream fout;
        fout.open("issues.log", ios::out | ios::app);

        fout << e.what() << endl;
        fout.close();
        return 0;
    }
}
