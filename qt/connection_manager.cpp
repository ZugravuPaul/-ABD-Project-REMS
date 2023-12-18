#include "connection_manager.h"
#include "client_thread.h"

#define PORT 9097

ConnectionManager::ConnectionManager(QObject *parent, MainWindow *mainwindow) : QThread(parent)
{
    this->mainwindow =mainwindow;
    isListening = true;
    //connect(server, &QTcpServer::newConnection, this, &ConnectionManager::onNewConnection);

}

void ConnectionManager::setListening(bool isListening)
{

    this->isListening = isListening;
}

void ConnectionManager::run()
{
    server = new QTcpServer();

    if (!server->listen(QHostAddress::Any, PORT)) {
        qDebug() << "Failed to start server";
        return;
    }
    else
    {
       qDebug() << "Server is listening on port " << PORT << " on address: " << server->serverAddress().toString();
       //se va face o singura data inainte de rewuesturi, deci nu e nevoie de mutex

       emit ConnectionManager::ModifyLabel(QString::fromUtf8("Server is listening on port ") + QString::number(PORT) + QString::fromUtf8(" , address ") + server->serverAddress().toString());
    }


    while (server->hasPendingConnections()) {

       if ( isListening == true) {
           QTcpSocket *clientSocket = server->nextPendingConnection();

           if (clientSocket) {
               // Create a new thread for each connection
               Client_Thread *clientThread = new Client_Thread(clientSocket,this->mainwindow, this);

               connect(mainwindow, &MainWindow::onStartStopPressed, clientThread, &Client_Thread::onStartStopPressed);
               clientThread->start();
           }
       }
    }
    exec();

   /* while (server->hasPendingConnections()) {
        QTcpSocket *clientSocket = server->nextPendingConnection();
        if (clientSocket) {
            Client_Thread *clientThread = new Client_Thread(clientSocket, this);

            connect(clientThread, &Client_Thread::printRequest, this, &ConnectionManager::updateUI);
            clientThread->start();
        }
    } */
}
