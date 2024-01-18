#include "proxythread.h"


QMutex ProxyThread::mutex;
char buffer[MAX_BUFFER_SIZE];
qint64 bytesRead=1;


ProxyThread::ProxyThread(QTcpSocket *clientSocket,int threadID, bool to_blocked, QObject *parent) : QThread(parent)
{
    // connect(clientSocket, &QTcpSocket::disconnected, this, &ProxyThread::onClientDisconnected);
    // connect(this, &ProxyThread::finished, this, &ProxyThread::deleteLater);
    this->clientSocket = clientSocket;
    this->targetSocket_fd = -1;
    this->threadID = threadID;
    this->blocked=to_blocked;
    this->currentStep = 0;
    this->win = static_cast<MainWindow*>(parent);
}

void ProxyThread::run()
{
    qDebug() << "Thread ID:" << QThread::currentThreadId() << "\n";

    //char buffer[MAX_BUFFER_SIZE];

    do
    {   currentStep = 0;
        bytesRead = clientSocket->read(buffer, MAX_BUFFER_SIZE);
        if (bytesRead > 0)
        {
            buffer[bytesRead] = '\0';
            char *request = strdup(buffer);
            char *targetUrl = NULL;

            qDebug() << "\n"
                     << request;

            if (targetSocket_fd == -1)
            {
                targetUrl = extractTargetURL(request);
                qDebug() << "host/" << targetUrl << "\\host\n";

                // Forward request to the target server
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

                    if(targetUrl != NULL)
                        free(targetUrl);
                    if(request != NULL)
                        free(request);
                    //clientSocket->close();
                    return;
                }

                memcpy(&targetAddr.sin_addr, host->h_addr_list[0], host->h_length);
                struct in_addr *addr = (struct in_addr *)host->h_addr_list[0];
                char *ip = inet_ntoa(*addr);
                QString destinationIP = QString::fromUtf8(ip);
                qDebug() << "\nHostName of " << host->h_name << " is: " << destinationIP;


                targetSocket_fd = socket(AF_INET, SOCK_STREAM, 0);

                if (targetSocket_fd == -1)
                {
                    qDebug("Failed to create target socket");
                    if(targetUrl != NULL)
                        free(targetUrl);
                    if(request != NULL)
                        free(request);
                    //clientSocket->close();
                    return;
                }
                QHostAddress address = clientSocket->peerAddress();
                QString sourceIP = address.toString();
                sourceIP = sourceIP.split(":").last();
                sourceIP[sourceIP.length() - 2] = '\0';
                if(win->findIP(sourceIP) == true || win->findIP(destinationIP) == true)
                {   qDebug()<<"L AM BLOCAT!";
                    if(targetUrl != NULL)
                        free(targetUrl);
                    if(request != NULL)
                        free(request);
                    return;
                }

                emit updateUI_TableHosts(QString::fromUtf8(request), this->threadID, QString::fromUtf8(targetUrl), destinationIP, sourceIP);

                ::connect(targetSocket_fd, (struct sockaddr *)&targetAddr, sizeof(targetAddr));
                while(blocked==true){
                    //sleep(100);
                   // qDebug()<<"blocat\n";
                };

            }

            write(targetSocket_fd, buffer, bytesRead);

            char responseBuffer[MAX_BUFFER_SIZE];
            //mutex.lock();
            while ((bytesRead = read(targetSocket_fd, responseBuffer, sizeof(responseBuffer))) > 0)
            {
               if(currentStep == 0 && blocked==false){
                    clientSocket->write(responseBuffer, bytesRead);
                    clientSocket->flush();
                }
                responseBuffer[bytesRead] = '\0';
                qDebug()<<responseBuffer<<"\n";
                emit insertResponse(QString::fromUtf8(responseBuffer), this->threadID);

                memset(responseBuffer, 0, sizeof(responseBuffer));
            }
            //mutex.unlock();
            if(blocked == false && currentStep == 1)
              blocked = true;

            while(blocked == true) {};
            if(currentStep == 2)
            {
                clientSocket->write(buffer, bytesRead);
                clientSocket->flush();
                blocked =true;
            }
//            if(blocked == false && currentStep == 2)
//              blocked = true;

            if(targetUrl != NULL)
                free(targetUrl);
            if(request != NULL)
                free(request);
        }
    } while(bytesRead);

     //clientSocket->close();
    //::close(targetSocket_fd);
}


void ProxyThread::onClientDisconnected()
{

    qDebug() << "Client disconnected. Cleaning up thread.";
    // isListening = false;
    if(this->blocked == true)
        emit SetListening();

    if (this->clientSocket)
        clientSocket->deleteLater();
    if (targetSocket_fd != -1)
        ::close(targetSocket_fd);
    targetSocket_fd = -1;

    quit();
}

char *ProxyThread::extractTargetURL(const char *request)
{

    //char *hostname = (char *)malloc(MAX_BUFFER_SIZE);
    char *method = (char *)malloc(MAX_BUFFER_SIZE);
    char *targetUrl = NULL;

    if (method)
    {   //Preiau host ul din header
        targetUrl = (char *)malloc(MAX_BUFFER_SIZE);

        sscanf(request,"%[^\n ]", method);

        if(strcmp(method,"CONNECT") == 0)
        {
            if (targetUrl)
                 sscanf(request, "%*[^ ] %[^:]", targetUrl);
        }
        else
            sscanf(request, "%*[^\n]\nHost: %[^\n\r]", targetUrl);

        free(method);
    }

    return targetUrl;
}

void ProxyThread::onNextPressed(QString message){

        this->currentStep++;
        memset(buffer, 0, MAX_BUFFER_SIZE);
        strcpy(buffer, message.toUtf8());
        bytesRead=strlen(buffer);
        if(blocked == true)
            blocked = false;
        else
            blocked = true;

        //write(targetSocket_fd,message.toUtf8(), message.length());
}
