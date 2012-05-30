#ifndef EXAMPLECLASS_H
#define EXAMPLECLASS_H

#include <QObject>

class ExampleClass : public QObject
{
    Q_OBJECT
public:
    explicit ExampleClass(QObject *parent = 0);

signals:
    void notify(QString message);

public slots:
    int add(int a, int b);
    void sleepAndNotify(QString message, int msec);
};

#endif // EXAMPLECLASS_H
