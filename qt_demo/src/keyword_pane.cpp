#include "keyword_pane.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

namespace {

constexpr char kDefaultKeyword[] = "erro";
constexpr std::size_t kMaxLength = 80;

} // namespace

KeywordPane::KeywordPane(QWidget* parent)
    : ReceiverPane(
          QObject::tr("só entrega mensagens contendo \"%1\" e com até %2 "
                      "caracteres")
              .arg(kDefaultKeyword)
              .arg(kMaxLength),
          parent) {
    // Keyword editor, shown between the note and the log.
    auto* row = new QWidget(this);
    auto* row_layout = new QHBoxLayout(row);
    row_layout->setContentsMargins(0, 0, 0, 0);
    auto* label = new QLabel(QObject::tr("palavra:"), row);
    keyword_edit_ = new QLineEdit(row);
    keyword_edit_->setText(kDefaultKeyword);
    keyword_edit_->setClearButtonEnabled(true);
    row_layout->addWidget(label);
    row_layout->addWidget(keyword_edit_);
    insert_header_widget(row);

    // Named constraint instead of a lambda: it carries state, so the UI
    // retargets it live and the subscription never needs to change.
    constraint_ = std::make_shared<loomdemo::KeywordConstraint>(
        keyword_edit_->text().toStdString());
    connect(keyword_edit_, &QLineEdit::textChanged, this, [this](const QString& text) {
        constraint_->set_keyword(text.toStdString());
        set_note(QObject::tr("só entrega mensagens contendo \"%1\" e com até "
                             "%2 caracteres")
                     .arg(text)
                     .arg(kMaxLength));
    });

    // Composition: both constraints must pass (AND semantics).
    subscribe({constraint_,
               std::make_shared<loomdemo::LengthConstraint>(kMaxLength)});
}
