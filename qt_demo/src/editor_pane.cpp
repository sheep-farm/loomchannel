#include "editor_pane.h"

#include <QFontDatabase>
#include <QPainter>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextFormat>
#include <QTextEdit>
#include <QVBoxLayout>

namespace {

// Toy econometric language: small vocabulary, # comments, "strings".
// Colors tuned for the dark editor background.
class EconoHighlighter : public QSyntaxHighlighter {
public:
    explicit EconoHighlighter(QTextDocument* doc) : QSyntaxHighlighter(doc) {
        QTextCharFormat keyword_format;
        keyword_format.setFontWeight(QFont::Bold);
        keyword_format.setForeground(QColor("#569cd6"));

        QTextCharFormat number_format;
        number_format.setForeground(QColor("#b5cea8"));

        QTextCharFormat string_format;
        string_format.setForeground(QColor("#ce9178"));

        QTextCharFormat comment_format;
        comment_format.setForeground(QColor("#6a9955"));
        comment_format.setFontItalic(true);

        // Later rules overwrite earlier ones on overlap, so comments come
        // last to swallow keywords and numbers inside them.
        rules_ = {
            {QRegularExpression(R"(\b(?:regress|ols|logit|probit|summary|plot|hist|load|save|select|filter|if|else|for|while|print|mean|median|sd|var|cov|corr|lag|diff)\b)"), keyword_format},
            {QRegularExpression(R"(\b\d+(\.\d+)?\b)"), number_format},
            {QRegularExpression(R"("[^"\n]*")"), string_format},
            {QRegularExpression(R"(#[^\n]*)"), comment_format},
        };
    }

protected:
    void highlightBlock(const QString& text) override {
        for (const auto& [pattern, format] : rules_) {
            auto matches = pattern.globalMatch(text);
            while (matches.hasNext()) {
                auto match = matches.next();
                setFormat(match.capturedStart(), match.capturedLength(), format);
            }
        }
    }

private:
    std::vector<std::pair<QRegularExpression, QTextCharFormat>> rules_;
};

// The line-number gutter, painting itself from the editor's block list.
class CodeEdit;

class LineNumberArea : public QWidget {
public:
    explicit LineNumberArea(CodeEdit* editor);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    CodeEdit* editor_;
};

// Plain text edit with the gutter wired in; no Q_OBJECT needed because
// everything is connected through lambdas.
class CodeEdit : public QPlainTextEdit {
public:
    explicit CodeEdit(QWidget* parent = nullptr)
        : QPlainTextEdit(parent) {
        setLineWrapMode(QPlainTextEdit::NoWrap);
        setFrameShape(QFrame::NoFrame);
        setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

        line_area_ = new LineNumberArea(this);
        connect(this, &QPlainTextEdit::blockCountChanged, this, [this](int) {
            update_area_width();
            update_area_geometry();
        });
        connect(this, &QPlainTextEdit::updateRequest, this, [this](const QRect& rect, int dy) {
            update_area(rect, dy);
        });
        connect(this, &QPlainTextEdit::cursorPositionChanged, this, [this] {
            highlight_current_line();
            line_area_->update();
        });

        update_area_width();
        update_area_geometry();
        highlight_current_line();
    }

    int area_width() const {
        int digits = 1;
        for (int max = qMax(1, blockCount()); max >= 10; max /= 10)
            ++digits;
        return 12 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    }

    // Called from the gutter's paint event: numbers aligned right, the
    // cursor's line number brighter and bold, like the Creator's.
    void paint_area(QPaintEvent* event) {
        QPainter painter(line_area_);
        painter.fillRect(event->rect(), QColor("#191919"));

        QTextBlock block = firstVisibleBlock();
        int block_number = block.blockNumber();
        int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
        int bottom = top + qRound(blockBoundingRect(block).height());
        const int current = textCursor().blockNumber();

        while (block.isValid() && top <= event->rect().bottom()) {
            if (block.isVisible() && bottom >= event->rect().top()) {
                QFont number_font = font();
                if (block_number == current) {
                    number_font.setBold(true);
                    painter.setPen(QColor("#c8c8c8"));
                } else {
                    painter.setPen(QColor("#6b6b6b"));
                }
                painter.setFont(number_font);
                painter.drawText(0, top, line_area_->width() - 6,
                                 fontMetrics().height(), Qt::AlignRight,
                                 QString::number(block_number + 1));
            }
            block = block.next();
            top = bottom;
            bottom = top + qRound(blockBoundingRect(block).height());
            ++block_number;
        }
    }

protected:
    void resizeEvent(QResizeEvent* event) override {
        QPlainTextEdit::resizeEvent(event);
        update_area_geometry();
    }

private:
    void update_area_width() {
        setViewportMargins(area_width(), 0, 0, 0);
    }

    void update_area_geometry() {
        const QRect rect = contentsRect();
        line_area_->setGeometry(rect.left(), rect.top(), area_width(), rect.height());
    }

    void update_area(const QRect& rect, int dy) {
        if (dy)
            line_area_->scroll(0, dy);
        else
            line_area_->update(0, rect.y(), line_area_->width(), rect.height());
        if (rect.contains(viewport()->rect()))
            update_area_geometry();
    }

    void highlight_current_line() {
        QList<QTextEdit::ExtraSelection> selections;
        QTextEdit::ExtraSelection selection;
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.format.setBackground(QColor("#252526"));
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        selections.append(selection);
        setExtraSelections(selections);
    }

    LineNumberArea* line_area_ = nullptr;
};

LineNumberArea::LineNumberArea(CodeEdit* editor)
    : QWidget(editor), editor_(editor) {
    setFixedWidth(editor->area_width());
}

QSize LineNumberArea::sizeHint() const {
    return {editor_->area_width(), 0};
}

void LineNumberArea::paintEvent(QPaintEvent* event) {
    editor_->paint_area(event);
}

} // namespace

EditorPane::EditorPane(QWidget* parent) : QWidget(parent) {
    auto* code = new CodeEdit(this);
    edit_ = code;
    edit_->setPlainText(
        "# linguagem econométrica de exemplo\n"
        "load \"dados.csv\"\n"
        "regress y x1 x2\n"
        "summary\n"
        "plot y\n");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(edit_);

    new EconoHighlighter(code->document());
}

QString EditorPane::text() const {
    return edit_->toPlainText();
}
