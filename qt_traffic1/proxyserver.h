#ifndef PROXYSERVER_H
#define PROXYSERVER_H

#define MAX_BUFFER_SIZE 4096
#define PORT 9097

#include <QThread>
#include <QTcpServer>
#include <QTcpSocket>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <QMutex>


//#include "mainwindow.h"

class ProxyServer : public QTcpServer
{
public:
    ProxyServer(QObject *parent = nullptr);

protected:
    void incomingConnection(int socketDescriptor);
};



#endif // PROXYSERVER_H
