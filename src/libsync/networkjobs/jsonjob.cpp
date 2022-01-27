#include "jsonjob.h"

#include "account.h"
#include "common/utility.h"

#include <QJsonDocument>
#include <QLoggingCategory>
#include <QNetworkReply>

Q_LOGGING_CATEGORY(lcJsonApiJob, "sync.networkjob.jsonapi", QtInfoMsg)

using namespace OCC;

JsonJob::JsonJob(const AccountPtr &account, const QString &path, QObject *parent)
    : SimpleNetworkJob(account, path, parent)
{
    setPath(path);
}

JsonJob::~JsonJob()
{
}

bool JsonJob::finished()
{
    qCInfo(lcJsonApiJob) << "JsonJob of" << reply()->request().url() << "FINISHED WITH STATUS"
                         << replyStatusString();

    if (reply()->error() != QNetworkReply::NoError) {
        qCWarning(lcJsonApiJob) << "Network error: " << this << errorString();
    } else {
        parse(reply()->readAll());
    }
    return SimpleNetworkJob::finished();
}

void JsonJob::parse(const QByteArray &data)
{
    const auto doc = QJsonDocument::fromJson(data, &_parseError);
    // empty or invalid response
    if (_parseError.error != QJsonParseError::NoError || doc.isNull()) {
        qCWarning(lcJsonApiJob) << "invalid JSON!" << data << _parseError.errorString();
    } else {
        _data = doc.object();
    }
}

const QJsonParseError &JsonJob::parseError() const
{
    return _parseError;
}

void JsonJob::start()
{
    // TODO: is this assumption sane
    if (!isPrepared()) {
        prepareQueryRequest(QByteArrayLiteral("GET"), QUrlQuery(), QNetworkRequest());
    }
    SimpleNetworkJob::start();
}

const QJsonObject &JsonJob::data() const
{
    return _data;
}

void JsonApiJob::prepareQueryRequest(const QByteArray &verb, const QUrlQuery &arguments, const QNetworkRequest &req)
{
    QNetworkRequest request = req;
    request.setRawHeader(QByteArrayLiteral("OCS-APIREQUEST"), QByteArrayLiteral("true"));
    QUrlQuery query = arguments;
    query.addQueryItem(QStringLiteral("format"), QStringLiteral("json"));
    JsonJob::prepareQueryRequest(verb, query, request);
}

int JsonApiJob::ocsStatus() const
{
    return _ocsStatus;
}

void JsonApiJob::parse(const QByteArray &rawData)
{
    static const QRegularExpression rex(QStringLiteral("<statuscode>(\\d+)</statuscode>"));
    const auto match = rex.match(QString::fromUtf8(rawData));
    if (match.hasMatch()) {
        // this is a error message coming back from ocs.
        _ocsStatus = match.captured(1).toInt();
    } else {
        JsonJob::parse(rawData);
        // example: "{"ocs":{"meta":{"status":"ok","statuscode":100,"message":null},"data":{"version":{"major":8,"minor":"... (504)
        if (data().contains(QLatin1String("ocs"))) {
            _ocsStatus = data().value(QLatin1String("ocs")).toObject().value(QLatin1String("meta")).toObject().value(QLatin1String("statuscode")).toInt();
        }
    }
}
