#include "editor_pane.h"

#include <QFontDatabase>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QVBoxLayout>

namespace {

// Toy econometric language: small vocabulary, # comments, "strings".
class EconoHighlighter : public QSyntaxHighlighter {
public:
    explicit EconoHighlighter(QTextDocument* doc) : QSyntaxHighlighter(doc) {
        QTextCharFormat keyword_format;
        keyword_format.setFontWeight(QFont::Bold);
        keyword_format.setForeground(QColor("#0057ae"));

        QTextCharFormat number_format;
        number_format.setForeground(QColor("#aa5500"));

        QTextCharFormat string_format;
        string_format.setForeground(QColor("#008000"));

        QTextCharFormat comment_format;
        comment_format.setForeground(QColor("#808080"));
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

} // namespace

EditorPane::EditorPane(QWidget* parent) : QWidget(parent) {
    edit_ = new QPlainTextEdit(this);
    edit_->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    edit_->setPlainText(
        "# linguagem econométrica de exemplo\n"
        "load \"dados.csv\"\n"
        "regress y x1 x2\n"
        "summary\n"
        "plot y\n");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(edit_);

    new EconoHighlighter(edit_->document());
}

QString EditorPane::text() const {
    return edit_->toPlainText();
}
