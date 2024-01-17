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


//http://www.testingmcafeesites.com/testcat_au.html


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Proxy Server");
    resize(1700, 1000);
    isListening=true;
    isIntercepting=true;
    ui->btnNext->setVisible(false);
    ui->btnStartStop->setText("Stop listening!");
    ui->btnIntercept->setText("Intercept off");
    ui->btnIntercept->setStyleSheet("background-color: red;");
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
    headers= new QAction ("Headers", this);

    actionStatusLine = new QAction("Status Line", this);
    actionGeneralHeaders = new QAction("General Headers", this);
    actionResponseHeaders = new QAction("Response Headers", this);
    actionEntityHeaders = new QAction("Entity Headers", this);
    actionMessageBody = new QAction("Message Body", this);

    //requestMenu=new QMenu("Request", this);
//    actionRequestHeader= new QAction("Request Header", this);
//    actionRequestBody= new QAction("Request Body", this);
    responseMenu->addAction(actionFullResponse);
    responseMenu->addAction(headers);
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
    connect(headers, &QAction::triggered, this, &MainWindow::handleHeaders);
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
       if(isListening == true) {
           //isListening = false;
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
       else
           clientSocket->deleteLater(),
           clientSocket->close();

}


void MainWindow::updateUI_TableHosts(const QString &requestData, qint64 connectionId, const QString& host)
{

    //QString cleanedData = requestData.trimmed().replace('\n', ' ') + "\n";
    qDebug() << "Received data:" << requestData << "\n";
    trafficData[connectionId].addSentData(requestData);
    QString currentDateTime = QDateTime::currentDateTime().toString(Qt::ISODate);
    trafficData[connectionId].addDate(currentDateTime);
    qDebug()<<currentDateTime<<"\n";
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

    trafficData[connectionId].addReceivedData(QString::fromLatin1(responseData.toUtf8()));

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
}

void MainWindow::handleHeaders()
{
    QTableWidgetItem *currentItem = ui->tableWidgetHosts->currentItem();
    if (currentItem)
    {
        qint64 connectionId = currentItem->data(Qt::UserRole).toLongLong();
        if (trafficData.contains(connectionId))
        {
            int index=trafficData[connectionId].getReceivedData().indexOf("\r\n\r\n");
            QString bodyContent = trafficData[connectionId].getReceivedData().mid(0, index);
            ui->textEditReqResp->setPlainText(bodyContent);

        }
    }
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


//    if (object == ui->textEditFilter && event->type() == QEvent::KeyPress)
//        {
//            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
//            if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return)
//            {
//                QString filterText = ui->textEditFilter->toPlainText();
//                for (int i = 0; i < ui->tableWidgetHosts->rowCount(); ++i)
//                    {
//                        for (int j = 0; j < ui->tableWidgetHosts->columnCount(); ++j)
//                        {
//                            QTableWidgetItem *item1 = ui->tableWidgetHosts->item(i, j);
//                            if (item1 && item1->text().contains(filterText, Qt::CaseInsensitive))
//                            {

//                                item1->setBackground(Qt::yellow);
//                            }
//                            else
//                            {

//                                item1->setBackground(Qt::white);
//                            }
//                        }
//                    }

//            }
//        }
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
            responseData = QString::fromLatin1(responseData.toUtf8());
            int payloadStart = responseData.indexOf("<");

            int bodyEnd = responseData.indexOf("</html>");
            bodyEnd= bodyEnd+6;

            if (payloadStart != -1 && bodyEnd != -1)
            {
                QString bodyContent = responseData.mid(payloadStart, bodyEnd - payloadStart);
                ui->textEditReqResp->setPlainText(bodyContent);
            }
            else
            {
                int bodyStart = responseData.indexOf("\r\n\r\n");
                if (bodyStart != -1)
                {
                   bodyStart += 4;
                   ui->textEditReqResp->setPlainText(responseData.mid(bodyStart).toUtf8());
                }
            }
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


void MainWindow::on_btnNext_clicked()
{

}


void MainWindow::on_btnIntercept_clicked()
{
        if (isIntercepting==true)
        {
            isIntercepting=false;
            ui->btnIntercept->setText("Intercept On");
            ui->btnIntercept->setStyleSheet("background-color: green;");
            ui->btnNext->setVisible(true);
        }
        else
        {
            isIntercepting=true;
            ui->btnIntercept->setText("Intercept Off");
            ui->btnIntercept->setStyleSheet("background-color: red;");
            ui->btnNext->setVisible(false);
        }
}




void MainWindow::on_btnExport_clicked()
{
    QFile file(QCoreApplication::applicationDirPath() + "/output.txt");

        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return;

        QTextStream out(&file);

        for (int row = 0; row < ui->tableWidgetHosts->rowCount(); ++row)
        {

                QTableWidgetItem *item = ui->tableWidgetHosts->item(row, 0);
                if (item)
                {
                    qint64 connectionId = item->data(Qt::UserRole).toLongLong();
                    if (trafficData.contains(connectionId))
                    {
                        // Presupunând că trafficData este un dicționar sau un map
                        const auto &data = trafficData[connectionId];
                        out << "Date: " << data.getDate() << "\n";  // Schimbați getDate() conform implementării dvs.
                        out << "Request: " << data.getReceivedData() << "\n"; // Schimbați getRequest() conform implementării dvs.
                        out << "Response: " << data.getSentData() << "\n"; // Schimbați getResponse() conform implementării dvs.
                        out << "---------------------------------\n";
                    }
                }

        }

        file.close();
}


MainWindow::~MainWindow()
{
    delete ui;
}
