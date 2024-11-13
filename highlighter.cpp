#include "highlighter.h"

Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    // Keywords - Ocean blue
    keywordFormat.setForeground(QColor("#64B5F6"));  
    keywordFormat.setFontWeight(QFont::Bold);
    const QString keywordPatterns[] = {
        QStringLiteral("\\bclass\\b"), QStringLiteral("\\bconst\\b"),
        QStringLiteral("\\benum\\b"), QStringLiteral("\\bexplicit\\b"),
        QStringLiteral("\\bfriend\\b"), QStringLiteral("\\binline\\b"),
        QStringLiteral("\\bnamespace\\b"), QStringLiteral("\\boperator\\b"),
        QStringLiteral("\\bprivate\\b"), QStringLiteral("\\bprotected\\b"),
        QStringLiteral("\\bpublic\\b"), QStringLiteral("\\bsignals\\b"),
        QStringLiteral("\\bsigned\\b"), QStringLiteral("\\bslots\\b"),
        QStringLiteral("\\bstatic\\b"), QStringLiteral("\\bstruct\\b"),
        QStringLiteral("\\btemplate\\b"), QStringLiteral("\\btypedef\\b"),
        QStringLiteral("\\btypename\\b"), QStringLiteral("\\bunion\\b"),
        QStringLiteral("\\bunsigned\\b"), QStringLiteral("\\bvirtual\\b"),
        QStringLiteral("\\bvolatile\\b"), QStringLiteral("\\busing\\b"),
        QStringLiteral("\\bif\\b"), QStringLiteral("\\belse\\b"),
        QStringLiteral("\\bfor\\b"), QStringLiteral("\\bwhile\\b"),
        QStringLiteral("\\bdo\\b"), QStringLiteral("\\bswitch\\b"),
        QStringLiteral("\\bcase\\b"), QStringLiteral("\\bbreak\\b"),
        QStringLiteral("\\breturn\\b"), QStringLiteral("\\bcontinue\\b"),
        QStringLiteral("\\bnew\\b"), QStringLiteral("\\bdelete\\b"),
        QStringLiteral("\\btry\\b"), QStringLiteral("\\bcatch\\b"),
        QStringLiteral("\\bthrow\\b"), QStringLiteral("\\bthis\\b"),
        QStringLiteral("\\btrue\\b"), QStringLiteral("\\bfalse\\b"),
        QStringLiteral("\\bnullptr\\b"), QStringLiteral("\\bvoid\\b"),
        QStringLiteral("\\bint\\b"), QStringLiteral("\\bfloat\\b"),
        QStringLiteral("\\bdouble\\b"), QStringLiteral("\\bchar\\b"),
        QStringLiteral("\\bbool\\b"), QStringLiteral("\\bstring\\b"),
        QStringLiteral("\\bauto\\b"), QStringLiteral("\\boverride\\b")
    };

    for (const QString &pattern : keywordPatterns) {
        HighlightingRule rule;
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // Class - Moonlit water
    classFormat.setForeground(QColor("#81D4FA"));  
    classFormat.setFontWeight(QFont::Bold);
    HighlightingRule rule;
    rule.pattern = QRegularExpression(QStringLiteral("\\bQ[A-Za-z]+\\b"));
    rule.format = classFormat;
    highlightingRules.append(rule);

    // Functions - Seafoam
    functionFormat.setForeground(QColor("#4DB6AC"));  
    rule.pattern = QRegularExpression(QStringLiteral("\\b[A-Za-z0-9_]+(?=\\()"));
    rule.format = functionFormat;
    highlightingRules.append(rule);

    // Numbers - Sandy gold
    numberFormat.setForeground(QColor("#FFD54F"));  
    rule.pattern = QRegularExpression(QStringLiteral("\\b\\d+(\\.\\d+)?\\b"));
    rule.format = numberFormat;
    highlightingRules.append(rule);

    // Operators - Coral
    operatorFormat.setForeground(QColor("#FF8A65"));  
    rule.pattern = QRegularExpression(QStringLiteral("[\\+\\-\\*\\/\\=\\<\\>\\!\\&\\|\\^\\~\\%]+"));
    rule.format = operatorFormat;
    highlightingRules.append(rule);

    // Preprocessor - Shell pink
    preprocessorFormat.setForeground(QColor("#F48FB1"));  
    rule.pattern = QRegularExpression(QStringLiteral("#[a-zA-Z]+\\b"));
    rule.format = preprocessorFormat;
    highlightingRules.append(rule);

    // Single line comment - Soft sand
    singleLineCommentFormat.setForeground(QColor("#D7CCC8"));  
    rule.pattern = QRegularExpression(QStringLiteral("//[^\n]*"));
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // Quotation - Pearl white
    quotationFormat.setForeground(QColor("#E0E0E0"));  
    rule.pattern = QRegularExpression(QStringLiteral("\".*\""));
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // Multi-line comments - Soft sand
    multiLineCommentFormat.setForeground(QColor("#D7CCC8"));  
    commentStartExpression = QRegularExpression(QStringLiteral("/\\*"));
    commentEndExpression = QRegularExpression(QStringLiteral("\\*/"));
}

void Highlighter::highlightBlock(const QString &text)
{
    // Apply regular highlighting rules
    for (const HighlightingRule &rule : qAsConst(highlightingRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    // Handle multi-line comments
    setCurrentBlockState(0);
    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(commentStartExpression);

    while (startIndex >= 0) {
        QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
        int endIndex = match.capturedStart();
        int commentLength = 0;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + match.capturedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
    }
}
