#include "exampleclass.h"
#include <QTimer>
#include <QEventLoop>

ExampleClass::ExampleClass(QObject *parent) :
    QObject(parent)
{
}

int ExampleClass::add(int a, int b)
{
    return a + b;
}

QString ExampleClass::concat(QString a, QString b)
{
    return a + b;
}

int ExampleClass::sum(QList<int> list)
{
    int sum = 0;
    foreach(int item, list)
        sum += item;
    return sum;
}

qreal ExampleClass::sum(QList<qreal> list)
{
    double sum = 0.0;
    foreach(double item, list)
        sum += item;
    return sum;
}

void ExampleClass::sleepAndNotify(int msec, QString message)
{
    QEventLoop sleepLoop;
    QTimer::singleShot(msec, &sleepLoop, SLOT(quit()));
    sleepLoop.exec();

    emit notify(message);
}
