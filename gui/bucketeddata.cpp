#include "bucketeddata.h"

BucketedData::BucketedData()
{

}

QVector<double> BucketedData::getBucket() const
{
    return bucket;
}

void BucketedData::setBucket(const QVector<double> &value)
{
    bucket = value;
}

QColor BucketedData::getColor() const
{
    return color;
}

void BucketedData::setColor(const QColor &value)
{
    color = value;
}
