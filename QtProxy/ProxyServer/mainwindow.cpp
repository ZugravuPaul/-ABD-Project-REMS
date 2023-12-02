#include "mainwindow.h"
#include "ui_mainwindow.h"

#define MAX_BUFFER_SIZE 4096
#define PORT 9097

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
     ui->setupUi(this);
     setWindowTitle("Proxy Server");
     resize(1700, 1000);
     ui->textEdit->setFixedSize(1820, 600);
     server = new QTcpServer(this);
     connect(server, &QTcpServer::newConnection, this, &MainWindow::onNewConnection);
     if (!server->listen(QHostAddress::Any, PORT))
     {
        qDebug() << "Failed to start server.";
        close();
     }
     else
     {
        qDebug() << "Server started on port " << PORT;
        ui->labelPort->setText("Server started on port " + QString::number(PORT));
     }
}

void MainWindow::onNewConnection()
{
    while (server->hasPendingConnections())  //coada de asteptare la server
    {
        QTcpSocket *clientSocket = server->nextPendingConnection();
        if (clientSocket)
        {
            connect(clientSocket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
        }
    }
}

void MainWindow::onReadyRead()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket *>(sender()); //sender() obtine obiectul care a emis semnalul si a declansat functia
    if (clientSocket)
    {
        handleClientRequest(clientSocket);
    }
}

char* MainWindow::extractTargetURL(const char* request)
{
    char* hostname = (char*)malloc(MAX_BUFFER_SIZE);
    char* targetUrl = NULL;

    if (hostname)
    {
        if (sscanf(request, "%*[^\n]\n%[^\n]", hostname) == 1)
        {
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

    if (bytesRead > 0)
    {
        char *request = strdup(buffer);
       // qDebug() << "\n" <<"Request: "<<request;
        if (strstr(request, "http://detectportal.firefox.com/canonical.html HTTP/1.1") == nullptr)
                {
                    qDebug() << "\n" << "Request: " << request;
                    ui->textEdit->append(QString(request));
                    ui->textEdit->repaint();
                }
        //ui->textEdit->append(QString(request));
        //ui->textEdit->repaint();
        QByteArray requestQ(request,bytesRead);
        //HTTPtraffic(requestQ);
        char *targetUrl = extractTargetURL(request);
        targetUrl[strlen(targetUrl) - 2] = '\0';
        qDebug() << "Setting target URL text: " << QString::fromUtf8(targetUrl);
        //ui->textEdit->append(targetUrl);
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

            clientSocket->close();
            free(request);
            free(targetUrl);
            return;
        }
        memcpy(&targetAddr.sin_addr, host->h_addr_list[0], host->h_length);
                QHostAddress targetAddress(reinterpret_cast<const sockaddr*>(&targetAddr.sin_addr));
                qDebug() << "\nHostName of " << host->h_name << " is: " << targetAddress.toString();

                int targetSocket = socket(AF_INET, SOCK_STREAM, 0);

                if (targetSocket == -1)
                {
                    qDebug("Failed to create target socket");
                    return;
                }

                ::connect(targetSocket, (struct sockaddr *)&targetAddr, sizeof(targetAddr));
                write(targetSocket, request, strlen(request));
                memset(buffer, 0, sizeof(buffer));
                while ((bytesRead = read(targetSocket, buffer, sizeof(buffer))) > 0)
                {
                    clientSocket->write(buffer, bytesRead);

                    // buffer[(int)bytesRead+1]='\0';
                    qDebug()<<"Headere \n";
                    QByteArray requestData(buffer, bytesRead);
                    if(strstr(requestData, "HTTP/1.1 302 Found")==nullptr)
                    {
                    qDebug().noquote() << QByteArray(buffer, bytesRead);
                    QMetaObject::invokeMethod(this, "updateUI", Qt::QueuedConnection,Q_ARG(QString, QString::fromUtf8(requestData)));
                    }
                    //HTTPtraffic(requestData);
                    qDebug()<<"\n gata hheadere";

                    memset(buffer, 0, sizeof(buffer));
                }


                ::close(targetSocket);
                clientSocket->close();

                free(request);
                free(targetUrl);
        }
}

void MainWindow::HTTPtraffic(QByteArray &response)
{
    //QMetaObject::invokeMethod(ui->textEdit, "append", Qt::QueuedConnection, Q_ARG(QVariant, QVariant(response)));
}

void MainWindow::updateUI(const QString &requestData)
{
    qDebug() << "Received data from target server: " << requestData;
    ui->textEdit->append(requestData);
    ui->textEdit->repaint();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_btnRequests_clicked()
{

}

