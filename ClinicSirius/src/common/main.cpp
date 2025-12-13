#include <QApplication>
#include <QFile>
#include <QStyleFactory>
#include "authwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Отключаем использование системной палитры
    app.setDesktopSettingsAware(false);
    
    // Используем Fusion стиль для консистентности и отключаем ялобые темы
    app.setStyle(QStyleFactory::create("Fusion"));
    
    // Создаем кастомную палитру с нашими цветами
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(255, 255, 255));
    darkPalette.setColor(QPalette::WindowText, QColor(31, 41, 55));
    darkPalette.setColor(QPalette::Base, QColor(255, 255, 255));
    darkPalette.setColor(QPalette::AlternateBase, QColor(245, 245, 245));
    darkPalette.setColor(QPalette::ToolTipBase, QColor(255, 255, 255));
    darkPalette.setColor(QPalette::ToolTipText, QColor(31, 41, 55));
    darkPalette.setColor(QPalette::Text, QColor(31, 41, 55));
    darkPalette.setColor(QPalette::Button, QColor(255, 255, 255));
    darkPalette.setColor(QPalette::ButtonText, QColor(31, 41, 55));
    darkPalette.setColor(QPalette::BrightText, QColor(255, 255, 255));
    darkPalette.setColor(QPalette::Link, QColor(59, 130, 246));
    darkPalette.setColor(QPalette::Highlight, QColor(59, 130, 246));
    darkPalette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    app.setPalette(darkPalette);
    
    // Загрузка стилей из QSS файла
    QFile styleFile(":/resources/styles.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = QLatin1String(styleFile.readAll());
        app.setStyleSheet(style);
        styleFile.close();
    }
    
    AuthWindow window;
    window.show();
    
    return app.exec();
}
