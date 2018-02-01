/****************************************************************************
**

**
****************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QLineEdit>
#include <QPushButton>
#include <QTcpServer>
#include <QTcpSocket>

#include "myobject.h"
//#include "tcpcomm.h"


QT_BEGIN_NAMESPACE
class QAction;
class QListWidget;
class QMenu;
class QTextEdit;
class QTreeWidget;
class QTreeWidgetItem;
class QTableWidget;
QT_END_NAMESPACE

typedef struct
{
    QByteArray addr;
    QByteArray data;
}LORA_RCV_DATA_T;

//! [0]
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

public:
    void hex2doubleASC(QByteArray *pbaHex, uint8_t len, QByteArray *pbaASC);
    uint8_t hex2asc(uint8_t ucdata);
    QByteArray dl645_SumCal(QByteArray *datatosum);
    QByteArray doublestr2hex(QByteArray *pdata, int lens);
    QByteArray add33H(QByteArray *pdata,int lens);

    LORA_RCV_DATA_T loradata;
    QByteArray baData;
    //QByteArray Dispdata;

private slots:
    void save();
    void print();
    void undo();
    void about();
    void insertCustomer(const QString &customer);
    void addParagraph(const QString &paragraph);

    void testSlot();
    void itemPressSlot(QTreeWidgetItem *, int);
    //void popMenu(const QPoint&);

    void on_act_read();

    void on_act_copy();

    void on_pushBtn_Listen_Clicked();
    void server_New_Connect();
    void socket_Disconnect();
    void socket_ReadData();
    void socket_SendData(QByteArray *pstrSend);
    void on_pushBtn_GetAddr_clicked();

    void autoScroll();


private:
    void createActions();
    void createStatusBar();
    void createDockWindows();
    //void createDatabase();
    void SendCommand(QByteArray &strdataID);

    QTextEdit *textEdit;
    QTreeWidgetItem *treeItem;
    QTreeWidget *customerTree;
    QTableWidget *paragraphsTree;
    QSqlDatabase database;
    QSqlQuery *query;

    QByteArray dataID;
    QTextEdit *textEdit_Disp;
    QString strDisp;
    QByteArray dl645_data;

    QMenu *m_menu;
    QAction *m_pact_read;
    QAction *m_pact_copy;

    QMenu *viewMenu;

    QThread *thread;
    myObject *object;

    QLineEdit *m_lineEdit_Port;
    QLineEdit *m_lineEdit_IP;
    QPushButton *m_pushBtn_Listen;
    QLineEdit *m_lineEdit_addr;
    QPushButton *m_pushBtn_GetAddr;
    QLineEdit *m_lineEdit_LoRaAddr;

    QTcpServer *server;
    QTcpSocket *socket;

};
//! [0]

#endif
