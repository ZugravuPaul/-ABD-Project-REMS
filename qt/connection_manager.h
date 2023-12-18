#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include <QThread>
#include <QTcpServer>
#include <QTcpSocket>

#include "mainwindow.h"

class ConnectionManager : public QThread
{
    Q_OBJECT

private:
    MainWindow* mainwindow;
    QTcpServer *server;
    bool isListening;

public:
    ConnectionManager(QObject *parent, MainWindow *mainwindow);
    ~ConnectionManager() {}

    void run() override;
    void setListening(bool isListening);

private slots:
    //void onNewConnection();
signals:
    void ModifyLabel(const QString string);
};



#endif // CONNECTION_MANAGER_H
