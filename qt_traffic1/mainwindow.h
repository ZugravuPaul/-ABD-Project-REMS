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
#include<QFocusEvent>
#include<QPushButton>
#include<QMenu>
#include<QAction>
#include<QTableWidget>
#include <QString>
#include <QByteArray>
#include <QDateTime>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnStartStop_clicked();
    void processConnection();
    void updateUI_TableHosts(const QString &requestData, int threadID, const QString& host, const QString &targetAddress_IP, const QString &sourceIP);
    void insertResponse(const QString &responseData, int threadID);
    void onHostItemClicked(QTableWidgetItem *item);
    void handleStatusLine();
    void handleGeneralHeaders();
    void handleResponseHeaders();
    void handleEntityHeaders();
    void handleMessageBody();
    void handleHeaders();
    void on_btnNext_clicked();
    void on_btnIntercept_clicked();
    void on_btnExport_clicked();

public slots:
    void handleRequestAction();
    void handleResponseAction();
    //void handleSetIntercept();

signals:
    void onStartStopPressed(bool isListening);
    void setNextPressed(QString message, int step);


private:
    void setupUI();
    void createActions();
    void createToolBar();

    QTcpServer *server;
    static int Id;
    int nrStep=0;

    Ui::MainWindow *ui;
    QLabel *labelPort;
    QTextEdit *textEditFilter;
    QTextEdit* textEditReqResp;
    QTableWidget* tableWidgetHosts;


    //ACTIONS
    QMenuBar *menubar;
    QMenu *responseMenu;
   // QMenu *requestMenu;
    QAction *actionRequest;
    QAction *actionFullResponse;
    QAction *actionResponse;
    QAction *actionStatusLine;
    QAction *actionGeneralHeaders;
    QAction *actionResponseHeaders;
    QAction *actionEntityHeaders;
    QAction *actionMessageBody;
    QAction *headers;
//    QAction *actionRequestHeader;
//    QAction  *actionRequestBody;

    bool requestPressed=false;
    bool  responsePresssed=false;
    bool isListening;
    bool isIntercepting;
protected:
    bool eventFilter(QObject *object, QEvent *event) override;  //https://forum.qt.io/topic/110350/how-to-make-an-eventfilter
};


#endif
