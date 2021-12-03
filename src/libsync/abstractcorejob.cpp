#include "abstractcorejob.h"

using namespace OCC;

AbstractCoreJob::AbstractCoreJob(QNetworkAccessManager *nam, QObject *parent)
    : QObject(parent)
    , _nam(nam)
{
}

AbstractCoreJob::~AbstractCoreJob()
{
}

const QVariant &AbstractCoreJob::result() const
{
    return _result;
}

const QString &AbstractCoreJob::error() const
{
    return _error;
}

QNetworkAccessManager *AbstractCoreJob::nam() const
{
    return _nam;
}

void AbstractCoreJob::setError(const QString &newError)
{
    _error = newError;
}

void AbstractCoreJob::setResult(const QVariant &newResult)
{
    _result = newResult;
}
