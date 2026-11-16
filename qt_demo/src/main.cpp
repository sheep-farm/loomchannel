#include <QAction>
#include <QApplication>
#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QMenuBar>
#include <QSplitter>
#include <QStatusBar>
#include <QTabWidget>
#include <QToolBar>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include "loomchannel/any_helpers.hpp"
#include "loomchannel/bus.hpp"

#include "channels.h"
#include "editor_pane.h"
#include "keyword_pane.h"
#include "receiver_pane.h"
#include "transmitter_pane.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("loomchannel-qtdemo");
    QApplication::setStyle("Fusion");

    QMainWindow window;
    window.setWindowTitle(QApplication::translate(
        "main", "LoomChannel — estilo Qt Creator"));

    // Passing a file path as the first argument saves a screenshot and
    // exits, so the demo can be verified without a display (offscreen).
    QString screenshot_path;
    if (argc > 1)
        screenshot_path = QString::fromLocal8Bit(argv[1]);

    auto* central = new QWidget(&window);
    auto* root = new QVBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // Fixed areas, Qt Creator style: splitters instead of docks.
    auto* vsplit = new QSplitter(Qt::Vertical);
    auto* hsplit = new QSplitter(Qt::Horizontal);

    // Left area: the project's files; double-click opens an editor tab.
    auto* project = new QListWidget;
    project->addItem("main.eco");
    project->addItem("dados.csv");
    project->addItem("simulacao.eco");

    // Center area: editor tabs, the only "sacred" space.
    auto* editor_tabs = new QTabWidget;
    editor_tabs->setDocumentMode(true);
    editor_tabs->setTabsClosable(true);
    editor_tabs->addTab(new EditorPane, "main.eco");

    // Right area: the transmitter as a tool pane.
    auto* right_tabs = new QTabWidget;
    right_tabs->setDocumentMode(true);
    auto* transmitter = new TransmitterPane;
    right_tabs->addTab(transmitter,
                       QApplication::translate("main", "Transmissor"));

    hsplit->addWidget(project);
    hsplit->addWidget(editor_tabs);
    hsplit->addWidget(right_tabs);
    hsplit->setStretchFactor(0, 0);
    hsplit->setStretchFactor(1, 1);
    hsplit->setStretchFactor(2, 0);
    hsplit->setSizes({170, 600, 230});

    // Bottom area: output panes in tabs; each one is a bus receiver and
    // tracks the channel state by itself (Broadcast / Stop).
    auto* output_tabs = new QTabWidget;
    output_tabs->setDocumentMode(true);
    output_tabs->addTab(new ConsolePane,
                        QApplication::translate("main", "Console"));
    output_tabs->addTab(new JdbcPane, "jdbc:");
    output_tabs->addTab(new KeywordPane,
                        QApplication::translate("main", "Palavra"));
    output_tabs->setMinimumHeight(160);

    vsplit->addWidget(hsplit);
    vsplit->addWidget(output_tabs);
    vsplit->setStretchFactor(0, 1);
    vsplit->setStretchFactor(1, 0);
    root->addWidget(vsplit);
    window.setCentralWidget(central);

    // Toolbar and menu reuse the transmitter's actions, Qt Creator style.
    auto* toolbar = window.addToolBar(QString());
    toolbar->setMovable(false);
    toolbar->addAction(transmitter->run_action());
    toolbar->addAction(transmitter->open_action());

    auto* file_menu = window.menuBar()->addMenu(
        QApplication::translate("main", "&Arquivo"));
    file_menu->addAction(QApplication::translate("main", "Sair"),
                         QKeySequence::Quit, qApp, &QCoreApplication::quit);
    auto* channel_menu = window.menuBar()->addMenu(
        QApplication::translate("main", "&Canal"));
    channel_menu->addAction(transmitter->open_action());
    channel_menu->addAction(transmitter->run_action());

    // "Executar" publishes the current editor tab, line by line.
    QObject::connect(transmitter, &TransmitterPane::run_requested,
                     [transmitter, editor_tabs] {
                         auto* editor =
                             qobject_cast<EditorPane*>(editor_tabs->currentWidget());
                         if (editor)
                             transmitter->publish_lines(editor->text().split('\n'));
                     });

    // The last editor tab stays: the central area is never empty.
    QObject::connect(editor_tabs, &QTabWidget::tabCloseRequested, editor_tabs,
                     [editor_tabs](int index) {
                         if (editor_tabs->count() > 1) {
                             QWidget* page = editor_tabs->widget(index);
                             editor_tabs->removeTab(index);
                             delete page;
                         }
                     });

    QObject::connect(project, &QListWidget::itemDoubleClicked,
                     [editor_tabs](QListWidgetItem* item) {
                         const QString name = item->text();
                         for (int i = 0; i < editor_tabs->count(); ++i) {
                             if (editor_tabs->tabText(i) == name) {
                                 editor_tabs->setCurrentIndex(i);
                                 return;
                             }
                         }
                         editor_tabs->setCurrentIndex(
                             editor_tabs->addTab(new EditorPane, name));
                     });

    // Even the status bar is just another subscriber of the channel.
    auto* status = new QLabel(&window);
    window.statusBar()->addWidget(status);
    auto set_status = [status](loomchannel::MessageEvent event,
                               const std::any& payload) {
        if (event == loomchannel::MessageEvent::Stop) {
            status->setText(QApplication::translate("main", " canal FECHADO "));
            status->setStyleSheet("color: #f85149; font-weight: bold;");
            return;
        }
        const auto text =
            loomchannel::any_cast_or<std::string>(payload, std::string{});
        status->setText(
            QApplication::translate("main", " canal ABERTO · última mensagem: %1 ")
                .arg(QString::fromStdString(text)));
        status->setStyleSheet("color: #3fb950; font-weight: bold;");
    };
    auto status_conn = loomchannel::Bus::instance().subscribe(
        loomdemo::kChat,
        [set_status](void*, loomchannel::MessageEvent event,
                     const std::any& payload) { set_status(event, payload); });
    set_status(loomchannel::Bus::instance().has_channel(loomdemo::kChat)
                   ? loomchannel::MessageEvent::Broadcast
                   : loomchannel::MessageEvent::Stop,
               {});

    window.resize(1000, 650);
    window.show();

    if (!screenshot_path.isEmpty()) {
        QTimer::singleShot(200, &window, [&window, screenshot_path] {
            window.grab().save(screenshot_path);
            QApplication::quit();
        });
    }

    const int result = QApplication::exec();

    // Close the channel on the way out so subscribers see a proper Stop.
    loomchannel::Bus::instance().close(loomdemo::kChat);
    return result;
}
