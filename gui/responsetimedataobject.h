#ifndef RESPONSETIMEDATAOBJECT_H
#define RESPONSETIMEDATAOBJECT_H

#include <QMetaType>
#include <QColor>
#include "qcustomplot.h"

class ResponseTimeDataObject
{
public:

    ResponseTimeDataObject() = default;
    ResponseTimeDataObject(const ResponseTimeDataObject &) = default;
    ResponseTimeDataObject &operator=(const ResponseTimeDataObject &) = default;

    void clear();
    void append(double v);
    QString name() const;
    void setName(const QString &name);
    QColor color() const;
    void setColor(const QColor &color);
    QVector<double> data() const;
    double getMin() const;
    void setMin(double value);
    double getMax() const;
    void setMax(double value);

private:
    double min = 4000000;
    double max = -4000000;

    QColor  mColor; /* Color to use when plotting */
    QString mName; /* data series name */
    QVector<double> mData; /* data series data */
};

#endif // RESPONSETIMEDATAOBJECT_H
