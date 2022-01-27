#include "jsonjob.h"

#include "account.h"
#include "common/utility.h"

#include <QJsonDocument>
#include <QLoggingCategory>
#include <QNetworkReply>

Q_LOGGING_CATEGORY(lcJsonApiJob, "sync.networkjob.jsonapi", QtInfoMsg)

using namespace OCC;

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

const QJsonObject &JsonJob::data() const
{
    return _data;
}


JsonApiJob::JsonApiJob(AccountPtr account, const QString &path, const UrlQuery &arguments, const QNetworkRequest &req, QObject *parent)
    : JsonJob(account, path, "GET", UrlQuery { { QStringLiteral("format"), QStringLiteral("json") } } + arguments, req, parent)
{
    _request.setRawHeader(QByteArrayLiteral("OCS-APIREQUEST"), QByteArrayLiteral("true"));
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
