#include "mainwindow.h"
#include "ui_mainwindow.h"
//#include "connection_manager.h"
#include "proxyserver.h"
#include "proxythread.h"
#include "trafficdata.h"

#define MAX_BUFFER_SIZE 4096
#define PORT 9097

QTcpServer* server;
QHash<qint64, TrafficData> trafficData;

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
    if (!server->listen(QHostAddress::Any, PORT))
    {
        qDebug() << "Failed to start server.";
        close();
    }
    else
    {
        qDebug() << "Server is listening on port " << PORT;
        ui->labelPort->setText("Server is listening on port " + QString::number(PORT) + " , address " + server->serverAddress().toString());
    }

    connect(server, &QTcpServer::newConnection, this, &MainWindow::processConnection);




}


void MainWindow::processConnection(){
    QTcpSocket *clientSocket = server->nextPendingConnection();
    qint64 connectionId = clientSocket->socketDescriptor();
    trafficData[connectionId] = TrafficData();


        // Create a worker thread to handle the connection
    ProxyThread *proxyThread = new ProxyThread(clientSocket);
    connect(proxyThread, &ProxyThread::updateUI, this, &MainWindow::updateUI);
    connect(clientSocket, &QTcpSocket::disconnected, proxyThread, &ProxyThread::onClientDisconnected);
    connect(proxyThread, &ProxyThread::finished, proxyThread, &ProxyThread::deleteLater);
    proxyThread->start();
}


void MainWindow::updateUI(const QString &requestData, qint64 connectionId)
{

    //QString cleanedData = requestData.trimmed().replace('\n', ' ') + "\n";
    qDebug() << "Received data:" << requestData << "\n";
     trafficData[connectionId].addReceivedData(requestData);


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


/*    QTcpServer *server = new QTcpServer(this);
    //int flag = 1; // 1 enables TCP_NODELAY, 0 disables it
    //setsockopt(server->socketDescriptor(), IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char *>(&flag), sizeof(flag));

    //connect(server, &QTcpServer::newConnection, this, &MainWindow::onNewConnection);

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
*/

