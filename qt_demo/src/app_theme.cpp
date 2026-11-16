#include "app_theme.h"

#include <QApplication>
#include <QPalette>
#include <QStyleFactory>

void apply_qtcreator_theme() {
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    // Dark palette, Fusion recipe: almost everything follows from these.
    QPalette palette;
    const QColor window(0x2b, 0x2b, 0x2b);
    const QColor base(0x1e, 0x1e, 0x1e);
    const QColor text(0xd4, 0xd4, 0xd4);
    const QColor dim(0x8a, 0x8a, 0x8a);
    const QColor highlight(0x2a, 0x82, 0xda);

    palette.setColor(QPalette::Window, window);
    palette.setColor(QPalette::WindowText, text);
    palette.setColor(QPalette::Base, base);
    palette.setColor(QPalette::AlternateBase, window);
    palette.setColor(QPalette::ToolTipBase, window);
    palette.setColor(QPalette::ToolTipText, text);
    palette.setColor(QPalette::Text, text);
    palette.setColor(QPalette::Button, window);
    palette.setColor(QPalette::ButtonText, text);
    palette.setColor(QPalette::BrightText, Qt::white);
    palette.setColor(QPalette::Link, highlight);
    palette.setColor(QPalette::Highlight, highlight);
    palette.setColor(QPalette::HighlightedText, Qt::white);
    palette.setColor(QPalette::Disabled, QPalette::Text, dim);
    palette.setColor(QPalette::Disabled, QPalette::WindowText, dim);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, dim);
    QApplication::setPalette(palette);

    // Flat chrome on top: no borders on bars and panes, hairline
    // separators, flat tabs with an accent underline, subtle hovers.
    auto* app = qobject_cast<QApplication*>(QCoreApplication::instance());
    app->setStyleSheet(R"(
        QMenuBar { background: palette(window); border: none; }
        QToolBar { border: none; background: palette(window); padding: 2px; spacing: 2px; }
        QToolButton { border: none; border-radius: 3px; padding: 4px 12px; background: transparent; }
        QToolButton:hover { background: rgba(255, 255, 255, 26); }
        QToolButton:pressed, QToolButton:checked { background: palette(highlight); color: palette(highlightedText); }
        QTabWidget::pane { border: none; border-top: 1px solid rgba(255, 255, 255, 26); }
        QTabBar::tab { background: transparent; border: none; padding: 5px 14px; }
        QTabBar::tab:selected { border-bottom: 2px solid palette(highlight); }
        QSplitter::handle { background: palette(window); }
        QSplitter::handle:horizontal { width: 2px; }
        QSplitter::handle:vertical { height: 2px; }
        QSplitter::handle:hover { background: palette(highlight); }
        QListWidget { border: none; background: palette(window); }
        QPlainTextEdit { border: none; background: palette(base); selection-background-color: palette(highlight); selection-color: palette(highlightedText); }
        QLineEdit { border: 1px solid rgba(255, 255, 255, 38); border-radius: 3px; background: palette(base); selection-background-color: palette(highlight); selection-color: palette(highlightedText); }
        QStatusBar { background: palette(window); border-top: 1px solid rgba(255, 255, 255, 26); }
    )");
}
