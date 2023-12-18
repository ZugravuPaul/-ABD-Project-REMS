#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QLineEdit>
#include<QLabel>
#include<QVBoxLayout>
#include<QPlainTextEdit>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<QCoreApplication>
#include <QMutex>
#include <QThread>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
private slots:
    void on_btnStartStop_clicked();
    void onModifyLabel(const QString string);

public:
    MainWindow(QWidget *parent = nullptr);
    void updateUI(const QString &requestData);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QLabel*labelPort=new QLabel();
    QTextEdit*textEdit=new QTextEdit();

    bool isListening;
signals:
    void onStartStopPressed(bool isListening);
};
#endif
