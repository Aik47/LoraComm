/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

//! [0]
#include <QtWidgets>
#if defined(QT_PRINTSUPPORT_LIB)
#include <QtPrintSupport/qtprintsupportglobal.h>
#if QT_CONFIG(printdialog)
#include <QtPrintSupport>
#endif
#endif

#include "mainwindow.h"
//! [0]
//!

//! [1]
MainWindow::MainWindow()
    : textEdit(new QTextEdit)
{
    //setCentralWidget(textEdit);

    createActions();
    createStatusBar();
    //createDatabase();
    createDockWindows();
    //setCentralWidget();


    m_pushBtn_GetAddr->setEnabled(FALSE);
    setWindowTitle(QStringLiteral("LoRaComm服务端调试工具 V0.1"));

    setUnifiedTitleAndToolBarOnMac(true);
    this->setMinimumSize(800,600);
    this->setMaximumSize(1600,1200);

    m_menu = new QMenu();
    m_pact_read = m_menu->addAction(QStringLiteral("抄读"));
    m_pact_copy = m_menu->addAction((QStringLiteral("复制")));

    connect(this->customerTree, SIGNAL(itemPressed(QTreeWidgetItem*,int)),
            this, SLOT(itemPressSlot(QTreeWidgetItem*, int)));
//    connect(this->customerTree, SIGNAL(customContextMenuRequested(const QPoint&)),
//            this,SLOT(popMenu(const QPoint&)));//检测鼠标右键
    connect(m_pact_read, SIGNAL(triggered()), this, SLOT(on_act_read()));
    connect(m_pact_copy, SIGNAL(triggered()), this, SLOT(on_act_copy()));

    connect(m_pushBtn_Listen, SIGNAL(clicked(bool)), this, SLOT(on_pushBtn_Listen_Clicked()));
    connect(m_pushBtn_GetAddr, SIGNAL(clicked(bool)), this, SLOT(on_pushBtn_GetAddr_clicked()));

    server = new QTcpServer();
    connect(server, SIGNAL(newConnection()), this, SLOT(server_New_Connect()));

    connect(textEdit_Disp, SIGNAL(textChanged()), this, SLOT(autoScroll()));

    thread = new QThread();
    object = new myObject();
    object->moveToThread(thread);
    connect(thread, SIGNAL(started()), object, SLOT(process()));
    //connect(object, SIGNAL(finished()), thread, SLOT(quit()));
    //connect(object, SIGNAL(finished()), object, SLOT(deleteLater()));
    //connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    thread->start();
}

MainWindow::~MainWindow()
{
    server->close();
    server->deleteLater();
    //delete MainWindow;
}
//! [1]

//! [3]
void MainWindow::print()
{
#if QT_CONFIG(printdialog)
    QTextDocument *document = textEdit->document();
    QPrinter printer;

    QPrintDialog dlg(&printer, this);
    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    document->print(&printer);
    statusBar()->showMessage(tr("Ready"), 2000);
#endif
}
//! [3]

//! [4]
void MainWindow::save()
{
    QMimeDatabase mimeDatabase;
    QString fileName = QFileDialog::getSaveFileName(this,
                        tr("Choose a file name"), ".",
                        mimeDatabase.mimeTypeForName("text/html").filterString());
    if (fileName.isEmpty())
        return;
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Dock Widgets"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    out << textEdit->toHtml();
    QApplication::restoreOverrideCursor();

    statusBar()->showMessage(tr("Saved '%1'").arg(fileName), 2000);
}
//! [4]

//! [5]
void MainWindow::undo()
{
    QTextDocument *document = textEdit->document();
    document->undo();
}
//! [5]

//! [6]
void MainWindow::insertCustomer(const QString &customer)
{
    if (customer.isEmpty())
        return;
    QStringList customerList = customer.split(", ");
    QTextDocument *document = textEdit->document();
    QTextCursor cursor = document->find("NAME");
    if (!cursor.isNull()) {
        cursor.beginEditBlock();
        cursor.insertText(customerList.at(0));
        QTextCursor oldcursor = cursor;
        cursor = document->find("ADDRESS");
        if (!cursor.isNull()) {
            for (int i = 1; i < customerList.size(); ++i) {
                cursor.insertBlock();
                cursor.insertText(customerList.at(i));
            }
            cursor.endEditBlock();
        }
        else
            oldcursor.endEditBlock();
    }
}
//! [6]

//! [7]
void MainWindow::addParagraph(const QString &paragraph)
{
    if (paragraph.isEmpty())
        return;
    QTextDocument *document = textEdit->document();
    QTextCursor cursor = document->find(tr("Yours sincerely,"));
    if (cursor.isNull())
        return;
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor, 2);
    cursor.insertBlock();
    cursor.insertText(paragraph);
    cursor.insertBlock();
    cursor.endEditBlock();

}
//! [7]

void MainWindow::about()
{
   QMessageBox::about(this, tr("About Dock Widgets"),
            tr("The <b>Dock Widgets</b> example demonstrates how to "
               "use Qt's dock widgets. You can enter your own text, "
               "click a customer to add a customer name and "
               "address, and click standard paragraphs to add them."));
}

void MainWindow::createActions()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    QToolBar *fileToolBar = addToolBar(tr("File"));


    const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(":/images/save.png"));
    QAction *saveAct = new QAction(saveIcon, tr("&Save..."), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the current form letter"));
    connect(saveAct, &QAction::triggered, this, &MainWindow::save);
    fileMenu->addAction(saveAct);
    fileToolBar->addAction(saveAct);

    const QIcon printIcon = QIcon::fromTheme("document-print", QIcon(":/images/print.png"));
    QAction *printAct = new QAction(printIcon, tr("&Print..."), this);
    printAct->setShortcuts(QKeySequence::Print);
    printAct->setStatusTip(tr("Print the current form letter"));
    connect(printAct, &QAction::triggered, this, &MainWindow::print);
    fileMenu->addAction(printAct);
    fileToolBar->addAction(printAct);

    fileMenu->addSeparator();

    QAction *quitAct = fileMenu->addAction(tr("&Quit"), this, &QWidget::close);
    quitAct->setShortcuts(QKeySequence::Quit);
    quitAct->setStatusTip(tr("Quit the application"));


    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    QToolBar *editToolBar = addToolBar(tr("Edit"));
    const QIcon undoIcon = QIcon::fromTheme("edit-undo", QIcon(":/images/undo.png"));
    QAction *undoAct = new QAction(undoIcon, tr("&Undo"), this);
    undoAct->setShortcuts(QKeySequence::Undo);
    undoAct->setStatusTip(tr("Undo the last editing action"));
    connect(undoAct, &QAction::triggered, this, &MainWindow::undo);
    editMenu->addAction(undoAct);
    editToolBar->addAction(undoAct);

    viewMenu = menuBar()->addMenu(tr("&View"));

    menuBar()->addSeparator();

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

    QAction *aboutAct = helpMenu->addAction(tr("&About"), this, &MainWindow::about);
    aboutAct->setStatusTip(tr("Show the application's About box"));

    QAction *aboutQtAct = helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
}

//! [8]
void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}
//! [8]

//! [9]
void MainWindow::createDockWindows()
{
    uint16_t i;
    int tmp = 0;

    //增加第1个DockWidget,用于TCP连接设置
    QDockWidget *dock = new QDockWidget(QStringLiteral("网络设置"), this);
    dock->setFixedWidth(150);
    //QListWidget *settingTree = new QListWidget(dock);
    QGroupBox *groupBox1 = new QGroupBox(QStringLiteral("TCP设置"));
    QGridLayout *gridLayout1 = new QGridLayout;
    m_lineEdit_Port = new QLineEdit(this);
    m_lineEdit_Port->setFixedHeight(20);
    m_lineEdit_IP = new QLineEdit(this);
    m_pushBtn_Listen = new QPushButton(QStringLiteral("侦听"), this);
    gridLayout1->addWidget(new QLabel(QStringLiteral(" 端口:")),0,0,1,1);
    gridLayout1->addWidget(m_lineEdit_Port,1,0,1,1);
    gridLayout1->addWidget(new QLabel(QStringLiteral(" IP地址:")),2,0,1,1);
    gridLayout1->addWidget(m_lineEdit_IP,3,0);
    gridLayout1->addWidget(m_pushBtn_Listen,4,0);
    //gridLayout1->setRowStretch(8,1);
    //gridLayout1->setVerticalSpacing(5);
    groupBox1->setLayout(gridLayout1);

    QGroupBox *groupBox2 = new QGroupBox(QStringLiteral("LoRa单元"));
    QGridLayout *gridLayout2 = new QGridLayout;
    m_lineEdit_LoRaAddr = new QLineEdit(this);
    gridLayout2->addWidget(new QLabel(QStringLiteral(" LoRa模块地址:")),0,0,1,1);
    gridLayout2->addWidget(m_lineEdit_LoRaAddr,1,0,1,1);
    groupBox2->setLayout(gridLayout2);

    QGroupBox *groupBox3 = new QGroupBox(QStringLiteral("电表单元"));
    QGridLayout *gridLayout3 = new QGridLayout;
    m_lineEdit_addr = new QLineEdit(this);
    m_pushBtn_GetAddr = new QPushButton(QStringLiteral("获取从设备地址"), this);
    gridLayout3->addWidget(new QLabel(QStringLiteral(" 电表地址:")),0,0,1,1);
    gridLayout3->addWidget(m_lineEdit_addr,6,0);
    gridLayout3->addWidget(m_pushBtn_GetAddr,7,0);
    groupBox3->setLayout(gridLayout3);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(groupBox1);
    mainLayout->addWidget(groupBox2);
    mainLayout->addWidget(groupBox3);
    mainLayout->addWidget(new QTextEdit);
    mainLayout->setContentsMargins(0,0,0,0);

    QGroupBox *maingroup = new QGroupBox;
    maingroup->setLayout(mainLayout);

    //groupBox->show();
    dock->setWidget(maingroup);
    dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    addDockWidget(Qt::TopDockWidgetArea, dock);

    //增加第2个DockWidget
    dock = new QDockWidget(QStringLiteral("可选抄读项"), this);
    //dock->setAllowedAreas(Qt::LeftDockWidgetArea);
    customerTree = new QTreeWidget(dock);
    //customerTree->setHeaderHidden(true);
    customerTree->setColumnCount(4);
    customerTree->setColumnWidth(0,250);
    QStringList head;
    head << QStringLiteral("抄读项名称") << QStringLiteral("功能码") << QStringLiteral("附加数据") << QStringLiteral("数据单位");
    customerTree->setHeaderLabels(head);
    {
        database = QSqlDatabase::addDatabase("QSQLITE");


        database.setDatabaseName("MyDataBase.db");
        query = new QSqlQuery(database);

        if(database.open())
        {
            qDebug()<<"Database Opened";

            QString select_sql = "select * from DLT645_2007";
            query->prepare(select_sql);
            //query.exec("select * from student") 也是可以的
            if(!query->exec())
            {
                qDebug()<<query->lastError();
                return ;
            }
        }
    }

    QTreeWidgetItem * item01 = new QTreeWidgetItem(customerTree,QStringList(QStringLiteral("基本参数")));
    {
        tmp = 0;
    }
        //database.close();
    QTreeWidgetItem * item02 = new QTreeWidgetItem(customerTree,QStringList(QStringLiteral("时区、时段")));

    QTreeWidgetItem * item03 = new QTreeWidgetItem(customerTree,QStringList(QStringLiteral("显示")));
    QTreeWidgetItem * item04 = new QTreeWidgetItem(customerTree,QStringList(QStringLiteral("通信速率")));
    QTreeWidgetItem * item05 = new QTreeWidgetItem(customerTree,QStringList(QStringLiteral("电能量")));
    qDebug()<<"query OK";
    {
        tmp = 0;
        for(i = 0; i < 114; i++)
        {
            if(query->next())
            {
                QByteArray dataID = query->value(0).toByteArray();
                QString desp = query->value(1).toString();
                QByteArray aux = query->value(2).toByteArray();
                QByteArray unit = query->value(3).toByteArray();
                //QTreeWidgetItem *items_tr1_01 = new QTreeWidgetItem(item05);
                new QTreeWidgetItem(item05);
                item05->child(tmp)->setText(0, desp);
                item05->child(tmp)->setText(1, dataID);
                item05->child(tmp)->setText(2, aux);
                item05->child(tmp++)->setText(3, unit);
            }
        }
    }
    database.close();

    QTreeWidgetItem * item06 = new QTreeWidgetItem(customerTree,QStringList(QStringLiteral("最大需量")));
//    {
//        tmp = 0;
//        for( ; i < 11; i++)
//        {
//            if(query->next())
//            {
//                QString desp = query->value(0).toString();
//                QString funcode = query->value(1).toString();
//                QString aux = query->value(2).toString();
//                QString unit = query->value(3).toString();
//                QTreeWidgetItem *items02_00 = new QTreeWidgetItem(item02);
//                item02->child(tmp)->setText(0, desp);
//                item02->child(tmp)->setText(1, funcode);
//                item02->child(tmp)->setText(2, aux);
//                item02->child(tmp++)->setText(3, unit);
//                //QTreeWidgetItem *items_tr1_01 = new QTreeWidgetItem(item02, QStringList(desp));
//            }
//        }
//    }
    QTreeWidgetItem * item07 = new QTreeWidgetItem(customerTree,QStringList(QStringLiteral("瞬时量")));
    QTreeWidgetItem * item08 = new QTreeWidgetItem(customerTree,QStringList(QStringLiteral("冻结数据")));
    QTreeWidgetItem * item09 = new QTreeWidgetItem(customerTree,QStringList(QStringLiteral("负荷曲线记录")));
    QTreeWidgetItem * item010 = new QTreeWidgetItem(customerTree,QStringList(QStringLiteral("事件记录")));
    QTreeWidgetItem * item011 = new QTreeWidgetItem(customerTree,QStringList(QStringLiteral("事件记录2")));
    QTreeWidgetItem * item012 = new QTreeWidgetItem(customerTree,QStringList(QStringLiteral("事件记录-单独项块抄")));
    //QTreeWidgetItem * item013 = new QTreeWidgetItem(customerTree,QStringList(QStringLiteral("事件记录-单独项块抄")));


    //QList<QTreeWidgetItem *> pitems;
    //QStringList()
//    QTreeWidgetItem *items_tr1 = new QTreeWidgetItem(customerTree,
//                                                  QStringList(QStringLiteral("[01] 复位控制")));
//    QTreeWidgetItem *items_tr1_01 = new QTreeWidgetItem(items_tr1,
//                                                  QStringList(QStringLiteral("[F1] 硬件初始化")));

//    QTreeWidgetItem *items_tr2 = new QTreeWidgetItem(customerTree,
//                                                  QStringList(QStringLiteral("[03] 硬件")));
//    QTreeWidgetItem *items_tr3 = new QTreeWidgetItem(customerTree,
//                                                  QStringList(QStringLiteral("[04] 软件")));
//    customerList = new QListWidget(dock);
//    customerList->addItems(QStringList()
//            << "John Doe, Harmony Enterprises, 12 Lakeside, Ambleton"
//            << "Jane Doe, Memorabilia, 23 Watersedge, Beaton");
    dock->setWidget(customerTree);
    dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    addDockWidget(Qt::TopDockWidgetArea, dock);

    viewMenu->addAction(dock->toggleViewAction());

    dock = new QDockWidget(QStringLiteral("抄读数据"), this);
    dock->setFeatures(QDockWidget::DockWidgetVerticalTitleBar);
    paragraphsTree = new QTableWidget(dock);
    paragraphsTree->setHidden(true);
    paragraphsTree->setColumnCount(5);
    QStringList m_Header;
    m_Header << QStringLiteral("电表编号") << QStringLiteral("通讯地址")
             << QStringLiteral("数据项名称") << QStringLiteral("数据")
             << QStringLiteral("单位");
    paragraphsTree->setHorizontalHeaderLabels(m_Header);
//    paragraphsTree->insertRow(0);
//    paragraphsTree->setItem(0,0,new QTableWidgetItem(QStringLiteral("00")));
//    paragraphsTree->setItem(0,1,new QTableWidgetItem(QStringLiteral("配置")));
//    paragraphsTree->setItem(0,2,new QTableWidgetItem(QStringLiteral("int")));
//    paragraphsTree->setItem(0,3,new QTableWidgetItem(QStringLiteral("0x68")));

    //    paragraphsList = new QListWidget(dock);
//    paragraphsList->addItems(QStringList()
//            << "Thank you for your payment which we have received today."
//            << "Your order has been dispatched and should be with you "
//               "within 28 days.");
    dock->setWidget(paragraphsTree);
    dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    addDockWidget(Qt::TopDockWidgetArea, dock);


    //增加一个TextEdit控件，显示发送和接收的数据
    textEdit_Disp = new QTextEdit(this);
    textEdit_Disp->setFontPointSize(14);
    textEdit_Disp->setText("Hello World!");
    textEdit_Disp->setMaximumHeight(300);
    textEdit_Disp->setMinimumHeight(100);
    setCentralWidget(textEdit_Disp);

    //txetshowTree->setHidden(true);

    //connect(customerList, &QListWidget::currentTextChanged,
            //this, &MainWindow::insertCustomer);
    //connect(paragraphsList, &QListWidget::currentTextChanged,
           // this, &MainWindow::addParagraph);
}
//! [9]
//!
//void MainWindow::createDatabase()
//{
//    database = QSqlDatabase::addDatabase("QSQLITE");

//    database.setDatabaseName("MyDataBase.db");

//    if(database.open())
//    {
//        qDebug()<<"Database Opened";

//        //QSqlQuery sql_query;
//        //QString create_sql = "create table student (desc varchar(30), funcode varchar(8), aux varchar(8), unit varvhar(10))"; //创建数据表
//        QSqlQuery query(database);
//        QString select_sql = "select * from DLT645_2007";
//        query.prepare(select_sql);
//        //query.exec("select * from student") 也是可以的
//        if(!query.exec())
//        {
//            qDebug()<<query.lastError();
//        }
//        else
//        {
//            while(query.next())
//            {
//                QString desp = query.value(0).toString();
//                QString funcode = query.value(1).toString();
//                QString aux = query.value(2).toString();
//                QString unit = query.value(3).toString();
//                qDebug()<<QString("desc:%1  funcode:%2  aux:%3  uint:%4").arg(desp).arg(funcode).arg(aux).arg(unit);
//            }
//        }
//    }
//    database.close();
//}

void MainWindow::testSlot()
{
    qDebug() << "Hello Action";
}

void MainWindow::itemPressSlot(QTreeWidgetItem *pressItem, int column)
{
    if(qApp->mouseButtons() == Qt::RightButton)
    {

        //QMenu *menu = new QMenu(this->customerTree);
        //if(pressItem->text(column))
        QPoint pos;
        pos = QCursor::pos();
        //QTreeWidgetItem *curitem = customerTree->itemAt(pos);
        for(int i = 0; i < pressItem->text(1).length(); i++)
        {
            dataID.data()[i] = pressItem->text(1).at(i).toLatin1();
        }
        //QString strtmp = curitem->text(1);
        //textEdit_Disp->setText(curitem->values(1).toString());

//        QMenu menu(ui.treeWidget);
//        menu.addAction(&deleteWell);
//        menu.addAction(&reNameWell);
        m_menu->exec(pos);  //在当前鼠标位置显示
    }
}

void MainWindow::on_act_read()
{
    //QMessageBox::information(NULL,"Tip","1-1 menu");
    qDebug() << QTime::currentTime();
    SendCommand(dataID);
}

void MainWindow::on_act_copy()
{
    QMessageBox::information(NULL,"Tip","1-2 menu");
}

void MainWindow::SendCommand(QByteArray &strdataID)
{
    //QString strtmp;

    QByteArray dataSend ;//= new QByteArray;
    QByteArray sumtmp;
    QByteArray sumtmpstr;
    QByteArray strdataID_ex;
    strdataID_ex = add33H(&strdataID, 8);
    dl645_data += "68";
    dl645_data += m_lineEdit_addr->text();
    dl645_data += "681104";
    dl645_data += strdataID_ex;
    //dl645_data += "00010000";
    //dl645_data += strdataID;
    sumtmp = dl645_SumCal(&dl645_data);
    hex2doubleASC(&sumtmp, sumtmp.length(), &sumtmpstr);
    dl645_data += sumtmpstr;
    dl645_data += "16";


    dataSend += "AT+TXH=";
    dataSend += m_lineEdit_LoRaAddr->text();
    dataSend += ",";
    dataSend += dl645_data;
    dataSend += "\r\n";

//    dataSend->append("AT+TXH=111,68");
//    dataSend->append(m_lineEdit_addr->text());
//    dataSend->append("681104");
//    dataSend->append(strdataID);
//    dataSend->append("00");
//    dataSend->append("16\r\n");

    strDisp.append(QStringLiteral("\r\n发送数据:\r\n"));
    strDisp.append(dataSend);
    textEdit_Disp->setText(strDisp);

    socket_SendData(&dataSend);
    dataSend.clear();
    dl645_data.clear();
}

QByteArray MainWindow::dl645_SumCal(QByteArray *datatosum)
{
    QByteArray strtmp;
    QByteArray returnBA;
    uint8_t tmpValue = 0;
    //uint8_t tmpH,tmpL;
    strtmp = doublestr2hex(datatosum, datatosum->length());

    for(int i = 0; i < strtmp.length(); i++)
        tmpValue += strtmp.data()[i];
    returnBA.append(tmpValue);
    return returnBA;



}

QByteArray MainWindow::doublestr2hex(QByteArray *pdata, int lens)
{
    QByteArray tmpstr;
    QByteArray tmpstr2;
    uint8_t tmpH,tmpL;
    for(int i = 0; i < lens; i++)
    {
        if((pdata->data()[i] >= '0') && (pdata->data()[i]<= '9'))
        {
            tmpstr +=  (pdata->data()[i] - 0x30);
        }
        else if((pdata->data()[i] >= 'A') && (pdata->data()[i]<= 'F'))
        {
            tmpstr += (pdata->data()[i] - 0x37);//0x0a、0x0A都转成A的ASCII
        }
        else
        {
            tmpstr += (pdata->data()[i] - 0x57);//0x0a、0x0A都转成A的ASCII
        }
    }
    for(int i = 0; i < tmpstr.length(); )
    {
        tmpH = (tmpstr.data()[i++] << 4);
        tmpL = (tmpstr.data()[i++]);
        tmpstr2 += (tmpH + tmpL);
    }
    return tmpstr2;
}
QByteArray MainWindow::add33H(QByteArray *pdata,int lens)
{
    QByteArray temp;
    QByteArray returnvalue;
    uint8_t ucdata;
    temp = doublestr2hex(pdata, lens);
    for(int i = 0; i < 4; i++)
    {
        temp.data()[i] = temp.data()[i] + 0x33;
    }
    ucdata = temp.data()[0];
    temp.data()[0] = temp.data()[3];
    temp.data()[3] = ucdata;
    ucdata = temp.data()[1];
    temp.data()[1] = temp.data()[2];
    temp.data()[2] = ucdata;
    hex2doubleASC(&temp, 4, &returnvalue);
    return returnvalue;
}

void MainWindow::on_pushBtn_Listen_Clicked()
{
    if(m_pushBtn_Listen->text() == QStringLiteral("侦听"))
    {
        int port = m_lineEdit_Port->text().toInt();

        if(server->listen(QHostAddress::Any, port))
        {
            qDebug() << server->errorString();
            return;
        }

        //修改按键文字
        m_pushBtn_Listen->setText(QStringLiteral("取消侦听"));
        strDisp.append("\r\n");
        strDisp.append(QStringLiteral("TCP侦听成功！\r\n"));
        textEdit_Disp->setText(strDisp);
    }
    else
    {
        if(socket->state() == QAbstractSocket::ConnectedState)
        {
            socket->disconnectFromHost();
        }
        server->close();
        m_pushBtn_Listen->setText(QStringLiteral("侦听"));
    }
}

void MainWindow::server_New_Connect()
{
    qDebug() << "program run to ...";
    socket = server->nextPendingConnection();

    QObject::connect(socket, &QTcpSocket::readyRead, this, &MainWindow::socket_ReadData);
    QObject::connect(socket, &QTcpSocket::disconnected, this, &MainWindow::socket_Disconnect);

    strDisp.append("\r\n");
    strDisp.append(QStringLiteral("TCP客户端连接成功!\r\n"));
    textEdit_Disp->setText(strDisp);
    m_pushBtn_GetAddr->setEnabled(TRUE);
}

void MainWindow::socket_ReadData()
{
    QByteArray buffer;
    //读取缓冲区数据
    buffer = socket->readAll();
    if(!buffer.isEmpty())
    {
        //str = ui->textEdit_Recv->toPlainText();
        //str+=tr(buffer);
        if(memcmp(buffer, "+RCV", 4) == 0)
        {
            uint8_t i;
            for(i = 5; i < 10; i++)
            {
                if(buffer.data()[i] == ',')
                {
                    loradata.addr.append(',');
                    break;
                }
                loradata.addr.append(buffer.data()[i]);
            }
            i++;
            for(; i < buffer.length(); i++)
            {
                if(buffer.data()[i] == 0x0d)
                {
                    break;
                }
                loradata.data.append(buffer.data()[i]);
            }

            hex2doubleASC(&loradata.data, loradata.data.length(), &baData);
            qDebug() << QTime::currentTime();
            //刷新显示
            strDisp.append("\r\n");
            strDisp.append(QStringLiteral("数据接收:\r\n"));
            strDisp.append(QStringLiteral("发送方LoRa地址:"));
            strDisp.append(loradata.addr);
            strDisp.append(QStringLiteral(" 数据:"));
            strDisp.append(baData);
            strDisp.append("\r\n");
            textEdit_Disp->setText(strDisp);
            loradata.addr.clear();
            loradata.data.clear();
            baData.clear();
        }
    }
}
//为了方便显示，需要把接收的一个数据(0x34)转成"34"字符串
void MainWindow::hex2doubleASC(QByteArray *pbaHex, uint8_t len, QByteArray *pbaASC)
{
    uint8_t i;
    uint8_t tmpH, tmpL;// index;

    for(i =0; i < len; i++)
    {
        tmpH = (pbaHex->data()[i] >> 4) & 0x0f;
        tmpL = pbaHex->data()[i] & 0x0f;
        tmpH = hex2asc(tmpH);
        tmpL = hex2asc(tmpL);

        //index = i << 1;
        pbaASC->append(tmpH);
        pbaASC->append(tmpL);
        //pbaASC->append(" ");
    }
}

uint8_t MainWindow::hex2asc(uint8_t udata)
{
    if((udata >= 0) && (udata <= 9))
    {
        return (udata + 0x30);
    }
    else
    {
        return (udata + 0x37);//0x0a、0x0A都转成A的ASCII
    }
}

//20180116 为了方便显示，需要把接收的一个字符串("34")转成数据(0x34)
//void MainWindow::str2hex(QByteArray *pbaHex, uint8_t len, QByteArray *pbaASC)
//{
//    uint8_t i;
//    uint8_t tmpH, tmpL, index;

//    for(i =0; i < len; i++)
//    {
//        tmpH = (pbaHex->data()[i] >> 4) & 0x0f;
//        tmpL = pbaHex->data()[i] & 0x0f;
//        tmpH = hex2asc(tmpH);
//        tmpL = hex2asc(tmpL);

//        index = i << 1;
//        pbaASC->append(tmpH);
//        pbaASC->append(tmpL);
//        pbaASC->append(" ");
//    }
//}

void MainWindow::socket_Disconnect()
{
    //发送按键失能
    //pushButton_Send->setEnabled(false);
    qDebug() << "Disconnected!";
    strDisp.append("\r\n");
    strDisp.append(QStringLiteral("TCP连接断开！\r\n"));
    textEdit_Disp->setText(strDisp);
    m_pushBtn_GetAddr->setEnabled(FALSE);
}

void MainWindow::socket_SendData(QByteArray *pstrSend)
{
    if(m_pushBtn_Listen->text() != QStringLiteral("取消侦听"))
    {
        return;
    }
    socket->write(*pstrSend);
    socket->flush();
}

void MainWindow::on_pushBtn_GetAddr_clicked()
{
    QByteArray batmp;
    batmp += "AT+TXH=";
    batmp += m_lineEdit_LoRaAddr->text();
    batmp += ",68AAAAAAAAAAAA681300DF16\r\n";
    strDisp.append(QStringLiteral("发送数据:\r\n"));
    strDisp.append(batmp);
    textEdit_Disp->setText(strDisp);
    socket_SendData(&batmp);
    //waitForReadyRead函数，有问题
    if(!(socket->QTcpSocket::waitForReadyRead(10000)))
    {
        strDisp.append(QStringLiteral("\r\n接收超时!\r\n"));
        textEdit_Disp->setText(strDisp);
    }
}

void MainWindow::autoScroll()
{
    if(textEdit_Disp->document()->lineCount() > 100)
            strDisp.clear();
    QTextCursor cursor = textEdit_Disp->textCursor();
    cursor.movePosition(QTextCursor::End);
    textEdit_Disp->setTextCursor(cursor);
}
