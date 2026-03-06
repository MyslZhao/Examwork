/**
 ****************************************************************************
 * Copyright (C), 2026, MyslZhao 159505988+MyslZhao@users.noreply.github.com
 *
 * @file highlighter.hpp
 * @brief markdown语法高亮类
 * @details
 * 语法高亮类(Highlighter)下声明的相关方法和变量包括了：
 * + 由匹配规则和语法渲染构成的高亮规则类(HighlightRule)
 * + 分别批量设置匹配规则和语法渲染的方法_set_match_rule和_set_highlight_style
 * + 语法高亮执行方法start
 * @author MyslZhao
 */

#ifndef HIGHLIGHTER_HPP
#define HIGHLIGHTER_HPP

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QVector>

class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit Highlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    QVector<HighlightRule> _highlight_rules; // 存储所有规则

    // 标题高亮
    QTextCharFormat heading1_format;
    QTextCharFormat heading2_format;
    QTextCharFormat heading3_format;
    QTextCharFormat heading4_format;
    QTextCharFormat heading5_format;
    QTextCharFormat heading6_format;

    /// 粗体高亮
    QTextCharFormat bold_format;
    /// 斜体高亮
    QTextCharFormat italic_format;
    /// 引用高亮
    QTextCharFormat quote_format;

    /// 链接高亮/图片链接高亮
    QTextCharFormat link_format;
    /// 有序列表高亮
    QTextCharFormat orderedlist_format;
    /// 无序列表高亮
    QTextCharFormat unorderedlist_format;

    /// 内嵌代码高亮
    QTextCharFormat code_format;
    /// 代码块高亮
    QTextCharFormat codeblock_format;

    /// 转义字符高亮
    QTextCharFormat slash_format;
    /// html实体字符高亮
    QTextCharFormat htmlentity_format;
    /// html标签高亮
    QTextCharFormat htmltag_format;

    /// 设置高亮样式
    void _set_highlight_style();
    /// 设置匹配规则
    void _set_match_rule();

    /// 执行高亮
    void start(const QString &text);

};

#endif // HIGHLIGHTER_HPP
