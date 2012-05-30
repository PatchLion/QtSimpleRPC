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

void ExampleClass::sleepAndNotify(QString message, int msec)
{
    QEventLoop sleepLoop;
    QTimer::singleShot(msec, &sleepLoop, SLOT(quit()));
    sleepLoop.exec();

    emit notify(message);
}
