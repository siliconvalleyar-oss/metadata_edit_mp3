#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("MP3 Metadata Editor");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("MetadataMP3");

    MainWindow window;
    window.show();

    return app.exec();
}
