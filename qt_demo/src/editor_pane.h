#pragma once

/**
 * @file editor_pane.h
 * @brief Editor tab with syntax highlighting for the toy econometric
 *        language (QPlainTextEdit + QSyntaxHighlighter).
 */

#include <QWidget>

class QPlainTextEdit;

class EditorPane : public QWidget {
    Q_OBJECT

public:
    explicit EditorPane(QWidget* parent = nullptr);

    QString text() const;

private:
    QPlainTextEdit* edit_;
};
