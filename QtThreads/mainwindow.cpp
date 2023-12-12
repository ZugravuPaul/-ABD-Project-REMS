#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "client_thread.h"

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
    //int flag = 1; // 1 enables TCP_NODELAY, 0 disables it
    //setsockopt(server->socketDescriptor(), IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char *>(&flag), sizeof(flag));
    connect(server, &QTcpServer::newConnection, this, &MainWindow::onNewConnection);

    if (!server->listen(QHostAddress::Any, PORT))
    {
        qDebug() << "Failed to start server.";
        close();
    }
    else
    {
        qDebug() << "Server is listening on port " << PORT << " on address: " << server->serverAddress().toString();
        ui->labelPort->setText("Server is listening on port " + QString::number(PORT) + " , address " + server->serverAddress().toString());
    }
}

void MainWindow::onNewConnection()  //se opreste cand se termina thread-urile!! TO_DO
{
    while (server->hasPendingConnections())
    {
        QTcpSocket *clientSocket = server->nextPendingConnection();
        if (clientSocket)
        {
            Client_Thread *client_Thread = new Client_Thread(this, clientSocket, this);
            //connect(client_Thread, &Client_Thread::printRequest, this, &MainWindow::updateUI);
            connect(this, &MainWindow::onStartStopPressed, client_Thread, &Client_Thread::onStartStopPressed);

            client_Thread->start();
        }
    }
}


void MainWindow::updateUI(const QString &requestData)
{

    //QString cleanedData = requestData.trimmed().replace('\n', ' ') + "\n";
    qDebug() << "Received data:" << requestData << "\n";
    ui->textEdit->append(requestData);
    ui->textEdit->repaint();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnStartStop_clicked()
{
    if (this->isListening)
    {
        isListening = false;
        ui->btnStartStop->setText("Start Listening");
        ui->btnStartStop->setStyleSheet("background-color: green;");
    }
    else
    {
        this->isListening = true;
        ui->btnStartStop->setText("Stop Listening");
        ui->btnStartStop->setStyleSheet("background-color: red;");
    }

    emit MainWindow::onStartStopPressed(this->isListening);
}


