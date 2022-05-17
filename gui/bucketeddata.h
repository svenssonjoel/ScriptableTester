#ifndef BUCKETEDDATA_H
#define BUCKETEDDATA_H

#include <QObject>
#include <QMetaType>
#include <QColor>

class BucketedData
{
public:
    BucketedData();

    QVector<double> getBucket() const;
    void setBucket(const QVector<double> &value);

    QColor getColor() const;
    void setColor(const QColor &value);

private:
    QVector<double> bucket;

    QColor color;

};

#endif // BUCKETEDDATA_H
