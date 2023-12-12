#include "client_thread.h"

#define MAX_BUFFER_SIZE 4096

Client_Thread::Client_Thread(MainWindow *mainwindow, QTcpSocket *clientSocket, QObject *parent, bool isListening) : QThread(parent), clientSocket(clientSocket), isListening(isListening)
{
    this->targetSocket_fd = -1;
    this->mainwindow = mainwindow;
    connect(clientSocket, &QTcpSocket::readyRead, this, &Client_Thread::onStartRequest);
    connect(clientSocket, &QTcpSocket::disconnected, this, &Client_Thread::onClientDisconnected);
}

void Client_Thread::onStartRequest() {

    //this->clientSocket = qobject_cast<QTcpSocket *>(sender());

    if (this->clientSocket)
        handleClientRequest();
}

void Client_Thread::onClientDisconnected() {

    qDebug() << "Client disconnected. Cleaning up thread.";
    isListening = false;

    if (this->clientSocket)
        clientSocket->deleteLater();
    if(targetSocket_fd != -1)
        ::close(targetSocket_fd);
    targetSocket_fd = -1;
    quit();
}

void Client_Thread::onStartStopPressed(bool isListening) {

    this->isListening = isListening;
}


char* Client_Thread::extractTargetURL(const char *request) {

    char *hostname = (char *)malloc(MAX_BUFFER_SIZE);
    char *targetUrl = NULL;

    if (hostname)
    {
        if (sscanf(request, "%*[^\n]\n%[^\n]", hostname) == 1)
        {
            size_t len = strlen(hostname);
            targetUrl = (char *)malloc(len + 1);
            if (targetUrl)
            {
                strcpy(targetUrl, hostname + 6);
                targetUrl[len-7] = '\0';
            }
        }

        free(hostname);
    }

    return targetUrl;
}

void Client_Thread::handleClientRequest() {  
    qDebug() << "Thread ID:" << QThread::currentThreadId()<<"\n";

    if (!isListening)
    {
        qDebug() << "Server is not listening. Ignoring request.";
        return;
    }
    char buffer[MAX_BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    qint64 bytesRead = clientSocket->read(buffer, sizeof(buffer));    //!!! se face doar o citire (posibil de modif)
    if (bytesRead > 0)
    {
        buffer[bytesRead]='\0';
        char *request = strdup(buffer);
        qDebug()<<"\n"<<request;
        char *targetUrl = extractTargetURL(request);
        qDebug() <<"host/"<<targetUrl<<"\\host\n";

        //emit printRequest(QString(QByteArray(request, bytesRead).replace("\n\n\n", " ")));
        mutex.lock();
        mainwindow->updateUI(QString::fromUtf8(request));
        mutex.unlock();

        if (targetSocket_fd == -1) {
            struct sockaddr_in targetAddr;
            memset(&targetAddr, 0, sizeof(targetAddr));
            targetAddr.sin_family = AF_INET;
            targetAddr.sin_port = htons(80);

            struct hostent *host = gethostbyname(targetUrl);

            if (host == nullptr)
            {
                switch (h_errno)
                {
                case HOST_NOT_FOUND:
                    qDebug() << "Host not found. Error " << h_errno;
                    break;
                case TRY_AGAIN:
                    qDebug() << "Non-Authoritative. Host not found. Error " << h_errno;
                    break;
                case NO_DATA:
                    qDebug() << "Valid name, no data record of requested type. Error " << h_errno;
                    break;
                case NO_RECOVERY:
                    qDebug() << "Non-recoverable error. Error " << h_errno;
                    break;
                }

                free(request);
                free(targetUrl);
                clientSocket->close();
                return;
            }

            memcpy(&targetAddr.sin_addr, host->h_addr_list[0], host->h_length);
            //QHostAddress targetAddress(reinterpret_cast<const sockaddr *>(&targetAddr.sin_addr));
            //qDebug() << "\nHostName of " << host->h_name << " is: " << targetAddress.toString();
            targetSocket_fd = socket(AF_INET, SOCK_STREAM, 0);

            if (targetSocket_fd == -1)
            {
                qDebug("Failed to create target socket");
                free(request);
                free(targetUrl);
                return;
            }

            ::connect(targetSocket_fd, (struct sockaddr *)&targetAddr, sizeof(targetAddr));
        }

        write(targetSocket_fd, request, strlen(request));

        memset(buffer, 0, sizeof(buffer));

        //clientul primeste raspunsul
        mutex.lock();
        while ((bytesRead = read(targetSocket_fd, buffer, sizeof(buffer))) > 0)
        {

            if (!isListening)
            {
                qDebug() << "Server stopped listening. Ignoring response.";
                break;
            }
            buffer[bytesRead]='\0';
            clientSocket->write(buffer, bytesRead);
            clientSocket->flush();

            mainwindow->updateUI(QString::fromUtf8(buffer));

            memset(buffer, 0, sizeof(buffer));
        }
        mutex.unlock();
        //::close(targetSocket_fd);  //TO_DO conexiune keep-alive
        //clientSocket->close();

        free(request);
        free(targetUrl);

    }
}

