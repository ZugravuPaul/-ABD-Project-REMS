#include "mainwindow.h"
#include "ui_mainwindow.h"

#define MAX_BUFFER_SIZE 4096
#define PORT 9097

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Proxy Server");
    resize(1700, 1000);
    ui->textEdit->setFixedSize(1820, 600);
    isListening=true;
    ui->btnStartStop->setText("Stop listening!");
    ui->btnStartStop->setStyleSheet("background-color: red;");

    server = new QTcpServer(this);
    connect(server, &QTcpServer::newConnection, this, &MainWindow::onNewConnection);

    if (!server->listen(QHostAddress::Any, PORT))
    {
        qDebug() << "Failed to start server.";
        close();
    }
    else
    {
        qDebug() << "Server is listening on port " << PORT;
        ui->labelPort->setText("Server is listening on port " + QString::number(PORT));
    }
}

void MainWindow::onNewConnection()
{
    while (server->hasPendingConnections())
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
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket *>(sender());
    if (clientSocket)
    {
        handleClientRequest(clientSocket);
    }
}

char *MainWindow::extractTargetURL(const char *request)
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
                strncpy(targetUrl, hostname + 6, len);
                targetUrl[len] = '\0';
            }
        }

        free(hostname);
    }

    return targetUrl;
}

void MainWindow::handleClientRequest(QTcpSocket *clientSocket)
{
    if (!isListening)
    {
        qDebug() << "Server is not listening. Ignoring request.";
        return;
    }
    char buffer[MAX_BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    qint64 bytesRead = clientSocket->read(buffer, sizeof(buffer));
    if (bytesRead > 0)
    {
        char *request = strdup(buffer);
        qDebug()<<"Request: " << request;
        ui->textEdit->append(QString(QByteArray(request, bytesRead).replace("\n\n\n", " ")));
        ui->textEdit->repaint();
        char *targetUrl = extractTargetURL(request);
        //targetUrl[strlen(targetUrl) - 1] = '\0';
        qDebug() << "Target URL : " << targetUrl;

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
        QHostAddress targetAddress(reinterpret_cast<const sockaddr *>(&targetAddr.sin_addr));
        //qDebug() << "\nHostName of " << host->h_name << " is: " << targetAddress.toString();

        int targetSocket = socket(AF_INET, SOCK_STREAM, 0);

        if (targetSocket == -1)
        {
            qDebug("Failed to create target socket");
            free(request);
            free(targetUrl);
            return;
        }

        ::connect(targetSocket, (struct sockaddr *)&targetAddr, sizeof(targetAddr));
        write(targetSocket, request, strlen(request));
        memset(buffer, 0, sizeof(buffer));

        while ((bytesRead = read(targetSocket, buffer, sizeof(buffer))) > 0)
        {
            if (!isListening)
            {
                qDebug() << "Server stopped listening. Ignoring response.";
                break;
            }
            clientSocket->write(buffer, bytesRead);
            QByteArray requestData(buffer, bytesRead);
            qDebug().noquote() << QByteArray(buffer, bytesRead);
            QMetaObject::invokeMethod(this, "updateUI", Qt::QueuedConnection, Q_ARG(QString, QString::fromUtf8(requestData)));
            memset(buffer, 0, sizeof(buffer));
        }

        ::close(targetSocket);
        clientSocket->close();

        free(request);
        free(targetUrl);
    }
}

void MainWindow::updateUI(const QString &requestData)
{
    QString cleanedData = requestData.trimmed().replace('\n', ' ') + "\n";
    qDebug() << "Received data from target server: " << cleanedData;
    ui->textEdit->append(cleanedData);
    ui->textEdit->repaint();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnStartStop_clicked()
{
    if (isListening)
    {
        isListening = false;
        ui->btnStartStop->setText("Start Listening");
        ui->btnStartStop->setStyleSheet("background-color: green;");
    }
    else
    {
        isListening = true;
        ui->btnStartStop->setText("Stop Listening");
        ui->btnStartStop->setStyleSheet("background-color: red;");
    }
}


