#include "myobject.h"
#include<QDebug>
#include<QTextEdit>

extern QString dataID;
extern QTextEdit *textEdit_Disp;

myObject::myObject(QObject *parent)
{

}

myObject::~myObject()
{

}

void myObject::process()
{
    qDebug() << QString("my object start!");

    emit finished();
}
