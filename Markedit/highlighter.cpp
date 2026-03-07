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

#include <utility>

Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    _set_highlight_style();
    _set_match_rule();
}

/**
 * @brief 设置高亮样式
 * @details
 * 样式设计标准参考vscode插件
 * "Forest Focus" by A.J Bale的markdown样式。
 *
 * 此外又以原版vscode markdown高亮样式填补未被高亮的语法
 */
void Highlighter::_set_highlight_style(){
    // 标题1-6 火红色
    QColor _title_color = QColor(0xBE, 0x56, 0x3A);

    heading1_format.setForeground(_title_color);

    heading2_format.setForeground(_title_color);

    heading3_format.setForeground(_title_color);

    heading4_format.setForeground(_title_color);

    heading5_format.setForeground(_title_color);

    heading6_format.setForeground(_title_color);

    // 分割线 浅灰
    hr_format.setForeground(Qt::darkGray);

    // 粗体 加粗
    bold_format.setFontWeight(QFont::Bold);

    // 斜体 加斜
    italic_format.setFontItalic(true);

    // 引用块 蓝色
    quote_format.setForeground(Qt::blue);

    // 链接 文本:浅绿色
    linktext_format.setForeground(QColor(0x90, 0xEE, 0x90));
    linkurl_format.setFontUnderline(true);

    // 有序列表 金色
    orderedlist_format.setForeground(QColor(0xCB, 0xC5, 0x1B));

    // 无序列表 黄色
    unorderedlist_format.setForeground(QColor(0xD9, 0xD2, 0x0F));

    // 内联代码 浅灰色背景 Courier new字体
    code_format.setBackground(QColor(0xEA, 0xEA, 0xEA));
    code_format.setFontFamilies(QStringList() << "Courier New");

    // 转义字符 深青色 斜体
    slash_format.setForeground(Qt::darkCyan);
    slash_format.setFontItalic(true);

    // HTML 实体字符 浅蓝色 斜体
    htmlentity_format.setForeground(QColor(0x5E, 0x97, 0xCB));
    htmlentity_format.setFontItalic(true);

    // HTML 标签 深绿色
    htmltag_format.setForeground(Qt::darkGreen);
}

/**
 * @brief 设置匹配规则
 * @details
 * 使用RegEx对文本文法匹配
 * ps:链接文法匹配详见highlightBlock方法。
 */
void Highlighter::_set_match_rule(){
    HighlightRule rule;

    // 标题
    rule.pattern = QRegularExpression("^#{1}\\s+.*$");
    rule.format = heading1_format;
    _highlight_rules.append(rule);

    rule.pattern = QRegularExpression("^#{2}\\s+.*$");
    rule.format = heading2_format;
    _highlight_rules.append(rule);

    rule.pattern = QRegularExpression("^#{3}\\s+.*$");
    rule.format = heading3_format;
    _highlight_rules.append(rule);

    rule.pattern = QRegularExpression("^#{4}\\s+.*$");
    rule.format = heading4_format;
    _highlight_rules.append(rule);

    rule.pattern = QRegularExpression("^#{5}\\s+.*$");
    rule.format = heading5_format;
    _highlight_rules.append(rule);

    rule.pattern = QRegularExpression("^#{6}\\s+.*$");
    rule.format = heading6_format;
    _highlight_rules.append(rule);

    // 分割线
    rule.pattern = QRegularExpression("^\\s*-{3,}\\s*$");
    rule.format = hr_format;
    _highlight_rules.append(rule);

    rule.pattern = QRegularExpression("^\\s*\\*{3,}\\s*$");
    _highlight_rules.append(rule);

    rule.pattern = QRegularExpression("^\\s*_{3,}\\s*$");
    _highlight_rules.append(rule);

    // 粗体
    rule.pattern = QRegularExpression("\\*\\*[^\\*\\n]+\\*\\*");
    rule.format = bold_format;
    _highlight_rules.append(rule);

    rule.pattern = QRegularExpression("__[^_\\n]+__");
    rule.format = bold_format;
    _highlight_rules.append(rule);

    // 斜体
    rule.pattern = QRegularExpression("\\*[^\\*\\n]+\\*");
    rule.format = italic_format;
    _highlight_rules.append(rule);

    rule.pattern = QRegularExpression("_[^_\\n]+_");
    rule.format = italic_format;
    _highlight_rules.append(rule);

    // 引用块
    rule.pattern = QRegularExpression("^>\\s+.*$");
    rule.format = quote_format;
    _highlight_rules.append(rule);

    // 有序列表
    rule.pattern = QRegularExpression("^\\s*\\d+\\.\\s+");
    rule.format = orderedlist_format;
    _highlight_rules.append(rule);

    // 无序列表
    rule.pattern = QRegularExpression("^\\s*[-+*]\\s+");
    rule.format = unorderedlist_format;
    _highlight_rules.append(rule);

    // 内联代码
    rule.pattern = QRegularExpression("`[^`\\n]+`");
    rule.format = code_format;
    _highlight_rules.append(rule);

    // 转义字符
    rule.pattern = QRegularExpression("\\\\.");
    rule.format = slash_format;
    _highlight_rules.append(rule);

    // HTML 实体
    rule.pattern = QRegularExpression("&[a-zA-Z]+;");
    rule.format = htmlentity_format;
    _highlight_rules.append(rule);

    // HTML 标签
    rule.pattern = QRegularExpression("<[^>]+>");
    rule.format = htmltag_format;
    _highlight_rules.append(rule);
}

/**
 * @brief 启动高亮
 * @param[in] text 编辑框文本内容
 */
void Highlighter::highlightBlock(const QString &text)
{
    for (const HighlightRule &rule : std::as_const(_highlight_rules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    // HACK: 由于链接语法高亮规则特殊，由highlightBlock单独处理语法高亮规则
    //      优先级在其他语法高亮之后
    QRegularExpression linkRegex("\\[([^\\]]+)\\]\\(([^\\)]+)\\)");
    QRegularExpressionMatchIterator it = linkRegex.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        int textStart = match.capturedStart(1);
        int textLength = match.capturedLength(1);
        if (textStart != -1) {
            setFormat(textStart, textLength, linktext_format);
        }
        int urlStart = match.capturedStart(2);
        int urlLength = match.capturedLength(2);
        if (urlStart != -1) {
            setFormat(urlStart, urlLength, linkurl_format);
        }
    }
}
