// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QLineEdit>
#include<QLabel>
#include<QVBoxLayout>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    QLineEdit *textBox;
    QLabel* labelPort;
    QTcpServer *server;
    QVBoxLayout *mainLayout = new QVBoxLayout();

private slots:
    void onNewConnection();
    void onReadyRead();

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void handleClientRequest(QTcpSocket *clientSocket);
    char *extractTargetURL(const char *request);
};

#endif // MAINWINDOW_H
