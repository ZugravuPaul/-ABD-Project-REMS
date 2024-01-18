#include "mainwindow.h"

#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    /*qDebug()<< "before server\n";
    ConnectionManager connectionManager(NULL,&w);
    connectionManager.startServer();
    qDebug()<< "after server\n"; */
    return a.exec();
}
