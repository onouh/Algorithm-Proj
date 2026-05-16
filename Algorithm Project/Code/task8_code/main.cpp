#include <QApplication>
#include <QStyleFactory>
#include <QPalette>
#include <QColor>
#include "mainwindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setStyle(QStyleFactory::create("Fusion"));

    QPalette pal;
    pal.setColor(QPalette::Window,          QColor("#F9FAFB"));
    pal.setColor(QPalette::WindowText,      QColor("#111827"));
    pal.setColor(QPalette::Base,            QColor("#FFFFFF"));
    pal.setColor(QPalette::AlternateBase,   QColor("#F3F4F6"));
    pal.setColor(QPalette::Button,          QColor("#F9FAFB"));
    pal.setColor(QPalette::ButtonText,      QColor("#111827"));
    pal.setColor(QPalette::Highlight,       QColor("#2563EB"));
    pal.setColor(QPalette::HighlightedText, QColor("#FFFFFF"));
    pal.setColor(QPalette::Text,            QColor("#111827"));
    app.setPalette(pal);

    MainWindow w;
    w.show();
    return app.exec();
}
