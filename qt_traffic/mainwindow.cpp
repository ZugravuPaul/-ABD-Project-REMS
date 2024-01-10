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
    isListening=true;
    ui->btnStartStop->setText("Stop listening!");
    ui->btnStartStop->setStyleSheet("background-color: red;");
    ui->textEditFilter->setFixedWidth(1800);
    ui->tableWidgetDetails->setFixedWidth(1810-ui->tableWidgetDetails->pos().x());
    ui->textEditReqResp->setFixedWidth(ui->tableWidgetDetails->width());
    ui->tableWidgetHosts->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidgetHosts->horizontalHeader()->setVisible(true);
    ui->tableWidgetHosts->setColumnCount(1);
    ui->tableWidgetHosts->setHorizontalHeaderLabels(QStringList() << "Hosts");
    ui->textEditFilter->installEventFilter(this);
    ui->textEditFilter->setText("Filter...");

    ui->tableWidgetDetails->setColumnCount(3);
    for (int i = 0; i < 3; ++i)
    {
        ui->tableWidgetDetails->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
    }

    actionRequest = new QAction("Request", this);
    actionResponse = new QAction("Response", this);
    menubar = menuBar();
    menubar->addAction(actionRequest);
    menubar->addAction(actionResponse);
    ui->horizontalLayout->addWidget(menubar);

    responseMenu = new QMenu("Response", this);
    actionFullResponse= new QAction("Full Response", this);
    actionStatusLine = new QAction("Status Line", this);
    actionGeneralHeaders = new QAction("General Headers", this);
    actionResponseHeaders = new QAction("Response Headers", this);
    actionEntityHeaders = new QAction("Entity Headers", this);
    actionMessageBody = new QAction("Message Body", this);

    //requestMenu=new QMenu("Request", this);
//    actionRequestHeader= new QAction("Request Header", this);
//    actionRequestBody= new QAction("Request Body", this);
    responseMenu->addAction(actionFullResponse);
    responseMenu->addAction(actionStatusLine);
    responseMenu->addAction(actionGeneralHeaders);
    responseMenu->addAction(actionResponseHeaders);
    responseMenu->addAction(actionEntityHeaders);
    responseMenu->addAction(actionMessageBody);
//    requestMenu->addAction(actionRequestHeader);
//    requestMenu->addAction(actionRequestBody);
    actionResponse->setMenu(responseMenu);
//    actionRequest->setMenu(requestMenu);


    // signal-slot
    connect(actionRequest, &QAction::triggered, this, &MainWindow::handleRequestAction);
    connect(actionResponse, &QAction::triggered, this, &MainWindow::handleResponseAction);
    connect(actionFullResponse, &QAction::triggered, this, &MainWindow::handleResponseAction);
    connect(actionStatusLine, &QAction::triggered, this, &MainWindow::handleStatusLine);
    connect(actionGeneralHeaders, &QAction::triggered, this, &MainWindow::handleGeneralHeaders);
    connect(actionResponseHeaders, &QAction::triggered, this, &MainWindow::handleResponseHeaders);
    connect(actionEntityHeaders, &QAction::triggered, this, &MainWindow::handleEntityHeaders);
    connect(actionMessageBody, &QAction::triggered, this, &MainWindow::handleMessageBody);
//    connect(actionRequestHeader, &QAction::triggered, this, &MainWindow::handleRequestHeader);
//    connect(actionRequestBody, &QAction::triggered, this, &MainWindow::handleRequestBody);
    connect(ui->tableWidgetHosts, &QTableWidget::itemClicked, this, &MainWindow::onHostItemClicked);



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


void MainWindow::processConnection()
{
    QTcpSocket *clientSocket = server->nextPendingConnection();
    qint64 connectionId = clientSocket->socketDescriptor();
    trafficData[connectionId] = TrafficData();

    // Create a worker thread to handle the connection
    ProxyThread *proxyThread = new ProxyThread(clientSocket);
    connect(proxyThread, &ProxyThread::updateUI_TableHosts, this, &MainWindow::updateUI_TableHosts);
    connect(proxyThread, &ProxyThread::insertResponse, this, &MainWindow::insertResponse);
    connect(clientSocket, &QTcpSocket::disconnected, proxyThread, &ProxyThread::onClientDisconnected);
    connect(proxyThread, &ProxyThread::finished, proxyThread, &ProxyThread::deleteLater);
    proxyThread->start();
}


void MainWindow::updateUI_TableHosts(const QString &requestData, qint64 connectionId, const QString& host)
{

    //QString cleanedData = requestData.trimmed().replace('\n', ' ') + "\n";
    qDebug() << "Received data:" << requestData << "\n";
    trafficData[connectionId].addSentData(requestData);

    int newRow = ui->tableWidgetHosts->rowCount();
    ui->tableWidgetHosts->insertRow(newRow);
    QTableWidgetItem *newItem = new QTableWidgetItem(host);
    newItem->setData(Qt::UserRole, QVariant::fromValue(connectionId)); // Row from table -> connectionId
    ui->tableWidgetHosts->setItem(newRow, 0, newItem);

    QString  method ,url;
    QStringList newlines = requestData.split("\r\n");
    if (!newlines.isEmpty())
    {

        QString firstLine = newlines.first();
        QStringList parts = firstLine.split(" ");
        if (parts.size() > 0)
        {
            method = parts[0];
        }
        if (parts.size() > 1)
        {
            url = parts[1];
        }
    }

    newRow = ui->tableWidgetDetails->rowCount();
    ui->tableWidgetDetails->insertRow(newRow);
    ui->tableWidgetDetails->setItem(newRow, 0, new QTableWidgetItem(host));
    ui->tableWidgetDetails->setItem(newRow, 1, new QTableWidgetItem(method));
    ui->tableWidgetDetails->setItem(newRow, 2, new QTableWidgetItem(url));
}

void MainWindow::insertResponse(const QString &responseData, qint64 connectionId)
{

    trafficData[connectionId].addReceivedData(responseData);
}

void MainWindow::onHostItemClicked(QTableWidgetItem *itemValue)
{

    if (itemValue==NULL)
       return;

    qint64 connectionId = itemValue->data(Qt::UserRole).toLongLong();
    if (trafficData.contains(connectionId))
    {
        QString sentData = trafficData[connectionId].getSentData();
        ui->textEditReqResp->setText(sentData);
    }


    //parsing Response
    QStringList lines = trafficData[connectionId].getReceivedData().split("\r\n", Qt::SkipEmptyParts);
    QString statusLine = lines[0];
    QStringList generalHeaders, responseHeaders, entityHeaders;
    QString body;

    int i= 1;
    bool bodyStarted = false;
    while (i < lines.size())
    {
        QString line = lines[i];
        if (bodyStarted)
        {
            body += line + "\n";
        }
        else
        {
            if (line.isEmpty())
            {
                bodyStarted = true;
            }
            else
            {
                if (line.startsWith("Content-Type:") || line.startsWith("Content-Length:") || line.startsWith("Last-Modified:"))
                {
                    entityHeaders << line;
                }
                else if (line.startsWith("Server:") || line.startsWith("Accept-Ranges:"))
                {
                    responseHeaders << line;
                }
                else
                {
                    generalHeaders << line;
                }
            }
        }
        i++;
    }
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

void MainWindow::handleRequestAction()
{
    //    requestPressed=true;
    //    responsePresssed=false;
    //    while(this->requestPressed==true)

    actionRequest->setProperty("style", "background-color: blue;");
    QTableWidgetItem *currentItem = ui->tableWidgetHosts->currentItem();
    if (currentItem)
    {
        qint64 connectionId = currentItem->data(Qt::UserRole).toLongLong();
        if (trafficData.contains(connectionId))
        {
            QString sentData = trafficData[connectionId].getSentData();
            ui->textEditReqResp->setText(sentData);
        }
        else
        {
            ui->textEditReqResp->setText("No data for selected connection.");
        }
    }
}

void MainWindow::handleResponseAction()
{
    //    responsePresssed=true;
    //    requestPressed=false;
    //    while(this->responsePresssed==true)
    actionResponse->setProperty("style", "background-color: blue;");
    QTableWidgetItem *currentItem = ui->tableWidgetHosts->currentItem();
    if (currentItem)
    {
        qint64 connectionId = currentItem->data(Qt::UserRole).toLongLong();
        if (trafficData.contains(connectionId))
        {
            QString receivedData = trafficData[connectionId].getReceivedData();
            ui->textEditReqResp->setText(receivedData);
        }
        else
        {
            ui->textEditReqResp->setText("No data for selected connection.");
        }
    }
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    if (object == ui->textEditFilter)
    {
        if (event->type() == QEvent::FocusIn)
        {
            if (ui->textEditFilter->toPlainText() == "Filter...")
            {
                ui->textEditFilter->clear();
            }
            return true;
        }
        else if (event->type() == QEvent::FocusOut)
        {
            if (ui->textEditFilter->toPlainText().isEmpty())
            {
                ui->textEditFilter->setText("Filter...");
            }
            return true;
        }
    }
    return QMainWindow::eventFilter(object, event);
}

void MainWindow::handleStatusLine()
{
    QTableWidgetItem *currentItem = ui->tableWidgetHosts->currentItem();
    if (currentItem)
    {
        qint64 connectionId = currentItem->data(Qt::UserRole).toLongLong();
        if (trafficData.contains(connectionId))
        {
            QStringList lines = trafficData[connectionId].getReceivedData().split("\r\n", Qt::SkipEmptyParts);
            if (!lines.isEmpty())
            {
                QString statusLine = lines[0];
                ui->textEditReqResp->setText(statusLine);
            }
        }
    }
}

void MainWindow::handleGeneralHeaders()
{
    QTableWidgetItem *currentItem = ui->tableWidgetHosts->currentItem();
    if (currentItem)
    {
        qint64 connectionId = currentItem->data(Qt::UserRole).toLongLong();
        if (trafficData.contains(connectionId))
        {
            QStringList lines = trafficData[connectionId].getReceivedData().split("\r\n", Qt::SkipEmptyParts);
            QStringList generalHeaders;

            int i = 1;
            while (i < lines.size())
            {
                QString line = lines[i];
                if (line.isEmpty())
                {
                    break;
                }
                if (line.startsWith("Date:") || line.startsWith("Connection:"))
                {
                    generalHeaders << line;
                }

                i++;
            }
            ui->textEditReqResp->setText(generalHeaders.join("\n"));
        }
    }
}

void MainWindow::handleResponseHeaders()
{
    QTableWidgetItem *currentItem = ui->tableWidgetHosts->currentItem();
    if (currentItem)
    {
        qint64 connectionId = currentItem->data(Qt::UserRole).toLongLong();
        if (trafficData.contains(connectionId))
        {
            QStringList lines = trafficData[connectionId].getReceivedData().split("\r\n", Qt::SkipEmptyParts);
            QStringList responseHeaders;

            int i = 1;
            while (i < lines.size())
            {
                QString line = lines[i];
                if (line.isEmpty())
                {
                    break;
                }
                if (line.startsWith("Server:") || line.startsWith("Accept-Ranges:"))
                {
                    responseHeaders << line;
                }

                i++;
            }
            ui->textEditReqResp->setText(responseHeaders.join("\n"));
        }
    }
}

void MainWindow::handleEntityHeaders()
{
    QTableWidgetItem *currentItem = ui->tableWidgetHosts->currentItem();
    if (currentItem)
    {
        qint64 connectionId = currentItem->data(Qt::UserRole).toLongLong();
        if (trafficData.contains(connectionId))
        {
            QStringList lines = trafficData[connectionId].getReceivedData().split("\r\n", Qt::SkipEmptyParts);
            QStringList entityHeaders;

            int i = 1;
            while (i < lines.size())
            {
                QString line = lines[i];
                if (line.isEmpty())
                {
                    break;
                }
                if (line.startsWith("Content-Type:") || line.startsWith("Content-Length:") || line.startsWith("Last-Modified:"))
                {
                    entityHeaders << line;
                }

                i++;
            }
            ui->textEditReqResp->setText(entityHeaders.join("\n"));
        }
    }
}

void MainWindow::handleMessageBody()
{
    QTableWidgetItem *currentItem = ui->tableWidgetHosts->currentItem();
    if (currentItem)
    {
        qint64 connectionId = currentItem->data(Qt::UserRole).toLongLong();
        if (trafficData.contains(connectionId))
        {
            QString responseData = trafficData[connectionId].getReceivedData();
            int payloadStart = responseData.indexOf("<html>");
            int bodyEnd = responseData.indexOf("</html>");
//            if (payloadStart != -1 && bodyEnd != -1)
//            {

//                payloadStart += 6;
//                QString bodyContent = responseData.mid(payloadStart, bodyEnd - payloadStart);

//                ui->textEditReqResp->setPlainText(bodyContent);
//            }
        }
    }
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

