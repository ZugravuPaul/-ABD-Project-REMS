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
#include "mainwindow.h"

class ProxyThread : public QThread
{
    Q_OBJECT
public:
    ProxyThread(QTcpSocket *clientSocket, int threadID, bool to_blocked, QObject *parent = nullptr);
    char* extractTargetURL(const char *request);
    void run() override;

private:
    QTcpSocket *clientSocket;
    MainWindow *win;
    int targetSocket_fd;
    bool blocked;
    int currentStep;
    int threadID;
    static QMutex mutex;

public slots:
    void onClientDisconnected();
    void onNextPressed(QString message);

signals:
    void SetListening();
    void updateUI_TableHosts(const QString &requestData, qint64 connectionId, const QString& host, const QString &targetAddress_IP, const QString &sourceIP);
    void insertResponse(const QString &responseData, qint64 connectionId);
};


#endif // PROXYTHREAD_H
