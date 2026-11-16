#include <QApplication>
#include <QLabel>
#include <QMainWindow>
#include <QTimer>

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
        "O receptor está inscrito no bus e acompanha o estado do canal:\n"
        "recebe Broadcast ao abrir/publicar e Stop ao fechar.\n"
        "Mensagem enviada com o canal fechado é armazenada no canal\n"
        "e entregue quando ele reabrir."));
    window.setCentralWidget(hint);

    auto* sender = new SenderDock(&window);
    auto* receiver = new ReceiverDock(&window);
    window.addDockWidget(Qt::LeftDockWidgetArea, sender);
    window.addDockWidget(Qt::RightDockWidgetArea, receiver);

    window.resize(860, 480);
    window.show();

    if (!screenshot_path.isEmpty()) {
        QTimer::singleShot(200, &window, [&window, screenshot_path] {
            window.grab().save(screenshot_path);
            QApplication::quit();
        });
    }

    return QApplication::exec();
}
