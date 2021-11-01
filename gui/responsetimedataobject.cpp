#include "responsetimedataobject.h"

void ResponseTimeDataObject::clear()
{
    mData.clear();
}

void ResponseTimeDataObject::append(double v)
{
    if (v > getMax()) setMax(v);
    if (v < getMin()) setMin(v);

    mData.append(v);
}

QString ResponseTimeDataObject::name() const
{
    return mName;
}

void ResponseTimeDataObject::setName(const QString &name)
{
    mName = name;
}

QColor ResponseTimeDataObject::color() const
{
    return mColor;
}

void ResponseTimeDataObject::setColor(const QColor &color)
{
    mColor = color;
}

QVector<double> ResponseTimeDataObject::data() const
{
    return mData;
}

double ResponseTimeDataObject::getMin() const
{
    return min;
}

void ResponseTimeDataObject::setMin(double value)
{
    min = value;
}

double ResponseTimeDataObject::getMax() const
{
    return max;
}

void ResponseTimeDataObject::setMax(double value)
{
    max = value;
}
