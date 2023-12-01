// mainwindow.cpp
#include "mainwindow.h"
#include <QDebug>

#define MAX_BUFFER_SIZE 4096
#define PORT 9097

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Proxy Server");
    textBox = new QLineEdit();
    labelPort=new QLabel();
    resize(1000, 900);
    server = new QTcpServer(this);
    connect(server, &QTcpServer::newConnection, this, &MainWindow::onNewConnection);

    if (!server->listen(QHostAddress::Any, PORT)) {
        qDebug() << "Failed to start server.";
        close();
    } else {
        qDebug() << "Server started on port " << PORT;
        labelPort->setText("Server started on port " + QString::number(PORT));
        labelPort->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        labelPort->setAlignment(Qt::AlignTop);

        mainLayout->addWidget(labelPort);
        QWidget *centralWidget = new QWidget(this);
        centralWidget->setLayout(mainLayout);
        setCentralWidget(centralWidget);

    }
}

void MainWindow::onNewConnection()
{
    while (server->hasPendingConnections()) {
        QTcpSocket *clientSocket = server->nextPendingConnection();
        if (clientSocket) {
            connect(clientSocket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
        }
    }
}

void MainWindow::onReadyRead()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket *>(sender());
    if (clientSocket) {
        handleClientRequest(clientSocket);
    }
}

char* MainWindow::extractTargetURL(const char* request)
{
    char* hostname = (char*)malloc(MAX_BUFFER_SIZE);
    char* targetUrl = NULL;

    if (hostname)
    {
        // Use %ms to allocate memory and store the scanned string
        if (sscanf(request, "%*[^\n]\n%[^\n]", hostname) == 1)
        {
            // Allocate memory for targetUrl and copy the content of hostname
            targetUrl = (char*)malloc(strlen(hostname) + 1);
            if (targetUrl)
            {
                strcpy(targetUrl, hostname + 6);
            }
        }

        free(hostname);
    }

    return targetUrl;
}


void MainWindow::handleClientRequest(QTcpSocket *clientSocket)
{
    char buffer[MAX_BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    qint64 bytesRead = clientSocket->read(buffer, sizeof(buffer));

    if (bytesRead > 0) {
        char *request = strdup(buffer);
        qDebug() << "\n" << request;

        char *targetUrl = extractTargetURL(request);
        targetUrl[strlen(targetUrl) - 1] = '\0';
        qDebug() << "\n" << targetUrl;

        // Open a connection to the target server
        struct sockaddr_in targetAddr;
        memset(&targetAddr, 0, sizeof(targetAddr));
        targetAddr.sin_family = AF_INET;
        targetAddr.sin_port = htons(80);

        struct hostent *host = gethostbyname(targetUrl);

        if (host == nullptr) {
            switch (h_errno) {
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

            clientSocket->close();
            free(request);
            free(targetUrl);
            return;
        }

        memcpy(&targetAddr.sin_addr, host->h_addr_list[0], host->h_length);
        QHostAddress targetAddress(reinterpret_cast<const sockaddr*>(&targetAddr.sin_addr));
        qDebug() << "\nHostName of " << host->h_name << " is: " << targetAddress.toString();

        int targetSocket = socket(AF_INET, SOCK_STREAM, 0);

        if (targetSocket == -1) {
            qDebug("Failed to create target socket");
            return;
        }

        ::connect(targetSocket, (struct sockaddr *)&targetAddr, sizeof(targetAddr));

        // Forward the client's request
        write(targetSocket, request, strlen(request));

        // Forward the target server's response
        memset(buffer, 0, sizeof(buffer));
        while ((bytesRead = read(targetSocket, buffer, sizeof(buffer))) > 0) {
            clientSocket->write(buffer, bytesRead);

            // buffer[(int)bytesRead+1]='\0';
            qDebug().noquote() << QByteArray(buffer, bytesRead);
            memset(buffer, 0, sizeof(buffer));
        }

        ::close(targetSocket);
        clientSocket->close();

        free(request);
        free(targetUrl);
    }
}

MainWindow::~MainWindow()
{
}
