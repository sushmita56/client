#pragma once

#include <QNetworkAccessManager>

#include "owncloudlib.h"

namespace OCC {

class OWNCLOUDSYNC_EXPORT AbstractCoreJob : public QObject
{
    Q_OBJECT
public:
    AbstractCoreJob(QNetworkAccessManager *nam, QObject *parent = nullptr);
    virtual ~AbstractCoreJob();

    virtual void start(const QUrl &url) = 0;

    const QVariant &result() const;

    const QString &error() const;

signals:
    void finished();

protected:
    void setResult(const QVariant &newResult);
    void setError(const QString &newError);
    QNetworkAccessManager *nam() const;

private:
    QNetworkAccessManager *_nam;
    QVariant _result;
    QString _error;
};

}
