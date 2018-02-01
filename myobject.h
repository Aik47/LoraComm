#ifndef MYOBJECT_H
#define MYOBJECT_H
#include <QObject>

class myObject:public QObject
{
    Q_OBJECT

public:
    explicit myObject(QObject *parent = 0);
    ~myObject();

public slots:
    void process();

signals:
    void finished();

};

#endif // MYOBJECT_H
