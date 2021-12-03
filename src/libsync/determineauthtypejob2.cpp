#include "determineauthtypejob2.h"

#include "common/utility.h"
#include "creds/httpcredentials.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcDetermineAuthTypeJob2, "sync.networkjob.determineauthtype2", QtInfoMsg);

using namespace OCC;

DetermineAuthTypeJob2::DetermineAuthTypeJob2(QNetworkAccessManager *nam, QObject *parent)
    : AbstractCoreJob(nam, parent)
{
}

DetermineAuthTypeJob2::~DetermineAuthTypeJob2()
{
}

void DetermineAuthTypeJob2::start(const QUrl &url)
{
    QNetworkRequest req(Utility::concatUrlPath(url, QStringLiteral("remote.php/dav/files/")));
    req.setAttribute(HttpCredentials::DontAddCredentialsAttribute, true);
    req.setAttribute(QNetworkRequest::AuthenticationReuseAttribute, QNetworkRequest::Manual);
    auto *reply = nam()->sendCustomRequest(req, "PROPFIND");
    connect(reply, &QNetworkReply::finished, this, [reply, this] {
        reply->deleteLater();
        const auto authChallenge = reply->rawHeader(QByteArrayLiteral("WWW-Authenticate")).toLower();
        // we fall back to basic in any case
        if (authChallenge.contains(QByteArrayLiteral("bearer "))) {
            setResult(qVariantFromValue(AuthType::OAuth));
        } else {
            setResult(qVariantFromValue(AuthType::Basic));
            if (authChallenge.isEmpty()) {
                qCWarning(lcDetermineAuthTypeJob2) << "Did not receive WWW-Authenticate reply to auth-test PROPFIND";
            }
        }
        qCInfo(lcDetermineAuthTypeJob2) << "Auth type for" << reply->url() << "is" << result();

        Q_EMIT finished();
    });
}
