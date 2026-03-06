/**
 ****************************************************************************
 * Copyright (C), 2026, MyslZhao 159505988+MyslZhao@users.noreply.github.com
 *
 * @file    highlighter.cpp
 * @brief   Highlighter类下方法的具体实现
 *
 * @author  MyslZhao
 */
#include "highlighter.hpp"
#include <QTextDocument>

Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    _set_highlight_style();
    _set_match_rule();
}

