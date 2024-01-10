#include "proxythread.h"

QMutex ProxyThread::mutex;

ProxyThread::ProxyThread(QTcpSocket *clientSocket, QObject *parent) : QThread(parent)
{
    // connect(clientSocket, &QTcpSocket::disconnected, this, &ProxyThread::onClientDisconnected);
    // connect(this, &ProxyThread::finished, this, &ProxyThread::deleteLater);
    this->clientSocket = clientSocket;
    this->targetSocket_fd = -1;
}

void ProxyThread::run()
{
    qDebug() << "Thread ID:" << QThread::currentThreadId() << "\n";

    char buffer[MAX_BUFFER_SIZE];
    qint64 bytesRead;

    do
    {
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
                emit updateUI_TableHosts(QString::fromUtf8(request), clientSocket->socketDescriptor(), QString::fromUtf8(targetUrl));

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

                    free(request);
                    free(targetUrl);
                    clientSocket->close();
                    return;
                }
                memcpy(&targetAddr.sin_addr, host->h_addr_list[0], host->h_length);
                targetSocket_fd = socket(AF_INET, SOCK_STREAM, 0);

                if (targetSocket_fd == -1)
                {
                    qDebug("Failed to create target socket");
                    free(request);
                    free(targetUrl);
                    clientSocket->close();
                    return;
                }

                ::connect(targetSocket_fd, (struct sockaddr *)&targetAddr, sizeof(targetAddr));
            }

            write(targetSocket_fd, buffer, bytesRead);

            char responseBuffer[MAX_BUFFER_SIZE];
            mutex.lock();
            while ((bytesRead = read(targetSocket_fd, responseBuffer, sizeof(responseBuffer))) > 0)
            {

                responseBuffer[bytesRead] = '\0';
                clientSocket->write(responseBuffer, bytesRead);
                clientSocket->flush();
                qDebug()<<responseBuffer<<"\n";
                emit insertResponse(QString::fromUtf8(responseBuffer), clientSocket->socketDescriptor());

                memset(responseBuffer, 0, sizeof(responseBuffer));
            }
            //mutex.unlock();

            free(targetUrl);
            free(request);
        }
    } while (bytesRead > 0);

    // clientSocket->close();
    //::close(targetSocket_fd);
}

void ProxyThread::onClientDisconnected()
{

    qDebug() << "Client disconnected. Cleaning up thread.";
    // isListening = false;

    if (this->clientSocket)
        clientSocket->deleteLater();
    if (targetSocket_fd != -1)
        ::close(targetSocket_fd);
    targetSocket_fd = -1;
    quit();
}

char *ProxyThread::extractTargetURL(const char *request)
{

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
                targetUrl[len - 7] = '\0';
            }
        }

        free(hostname);
    }

    return targetUrl;
}
