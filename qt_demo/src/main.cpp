#include <QApplication>
#include <QLabel>
#include <QMainWindow>
#include <QTimer>

#include "filtered_receiver_dock.h"
#include "receiver_dock.h"
#include "sender_dock.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("loomchannel-qtdemo");

    QMainWindow window;
    window.setWindowTitle(QApplication::translate(
        "main", "LoomChannel — Dock Windows"));

    // Passing a file path as the first argument saves a screenshot and
    // exits, so the demo can be verified without a display (offscreen).
    QString screenshot_path;
    if (argc > 1)
        screenshot_path = QString::fromLocal8Bit(argv[1]);

    auto* hint = new QLabel(&window);
    hint->setAlignment(Qt::AlignCenter);
    hint->setWordWrap(true);
    hint->setText(QApplication::translate(
        "main",
        "O transmissor publica mensagens de texto no canal \"chat\".\n"
        "Dois receptores estão inscritos no bus e acompanham o estado do\n"
        "canal: recebem Broadcast ao abrir/publicar e Stop ao fechar.\n"
        "O receptor filtrado assina com uma Constraint e só aceita\n"
        "mensagens que começam com \"jdbc:\". Mensagem enviada com o\n"
        "canal fechado é armazenada no canal e entregue quando ele reabrir."));
    window.setCentralWidget(hint);

    auto* sender = new SenderDock(&window);
    auto* receiver = new ReceiverDock(&window);
    auto* filtered = new FilteredReceiverDock(&window);
    window.addDockWidget(Qt::LeftDockWidgetArea, sender);
    window.addDockWidget(Qt::RightDockWidgetArea, receiver);
    window.addDockWidget(Qt::BottomDockWidgetArea, filtered);

    window.resize(860, 620);
    window.show();

    if (!screenshot_path.isEmpty()) {
        QTimer::singleShot(200, &window, [&window, screenshot_path] {
            window.grab().save(screenshot_path);
            QApplication::quit();
        });
    }

    return QApplication::exec();
}
