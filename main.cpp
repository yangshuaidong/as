#include <QApplication>
#include <QIcon>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Serial Assistant");
    app.setOrganizationName("SerialTool");
    app.setWindowIcon(QIcon(":/resources/app_icon.png"));

    MainWindow w;
    w.show();

    return app.exec();
}
