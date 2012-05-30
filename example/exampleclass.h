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
    QString concat(QString a, QString b);
    int sum(QList<int> list);
    qreal sum(QList<qreal> list);
    void sleepAndNotify(int msec, QString message);
};

#endif // EXAMPLECLASS_H
