#ifndef PROXYTHREAD_H
#define PROXYTHREAD_H


#define MAX_BUFFER_SIZE 4096
#define PORT 9097

#include <QThread>
#include <QTcpServer>
#include <QTcpSocket>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <QMutex>


class ProxyThread : public QThread
{
    Q_OBJECT
public:
    ProxyThread(QTcpSocket *clientSocket, QObject *parent = nullptr);
    char* extractTargetURL(const char *request);
    void run() override;

private:
    QTcpSocket *clientSocket;
    int targetSocket_fd;
    static QMutex mutex;

public slots:
    void onClientDisconnected();

signals:
    void updateUI(const QString &requestData, qint64 connectionId);
};


#endif // PROXYTHREAD_H
