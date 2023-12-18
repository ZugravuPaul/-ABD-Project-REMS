#ifndef CLIENT_THREAD_H
#define CLIENT_THREAD_H

#include <QThread>
#include <QTcpSocket>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <unistd.h>
#include <QHostAddress>
#include <QMutex>
#include <mainwindow.h>

class Client_Thread: public QThread
{
    Q_OBJECT

    QTcpSocket *clientSocket;
    MainWindow* mainwindow;
    int targetSocket_fd;
    bool isListening;

public:
    Client_Thread(QTcpSocket *clientSocket, MainWindow *mainwindow, QObject *parent = nullptr);
    char *extractTargetURL(const char *request);
    void handleClientRequest();
    static QMutex mutex;

private slots:
    void onStartRequest();
    void onClientDisconnected();

public slots:
    void onStartStopPressed(bool isListening);
};

#endif // CLIENT_THREAD_H
