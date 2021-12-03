#include "resolveurl.h"

#include "common/utility.h"

#include <QLoggingCategory>
#include <QNetworkAccessManager>
#include <QNetworkReply>

Q_LOGGING_CATEGORY(lcResolveUrl, "wizard.resolveurl")

using namespace SetupWizard;

ResolveUrl::ResolveUrl(QNetworkAccessManager *nam, QObject *parent)
    : OCC::AbstractCoreJob(nam, parent)
{
}

void ResolveUrl::start(const QUrl &url)
{
    QNetworkRequest req(OCC::Utility::concatUrlPath(url, QStringLiteral("status.php")));
    req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    auto reply = nam()->get(req);

    connect(reply, &QNetworkReply::finished, this, [oldUrl = url, reply, this] {
        reply->deleteLater();
        const auto status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if (reply->error() != QNetworkReply::NoError) {
            setError(tr("Failed to resolve the url: %1 error:%2").arg(oldUrl.toDisplayString(), reply->errorString()));
        } else if (status == 404) {
            setError(tr("Failed to resolve %1").arg(oldUrl.toDisplayString()));
            qCWarning(lcResolveUrl) << error();
        } else {
            const auto url = reply->url().adjusted(QUrl::RemoveFilename);
            if (url != oldUrl) {
                qCInfo(lcResolveUrl) << oldUrl << "was redirected to" << url;
                if (url.scheme() == QLatin1String("https") && oldUrl.host() == url.host()) {
                    setResult(url);
                } else {
                    // TODO:
                    setError(tr("Unsupported redirect to :%1").arg(url.toDisplayString()));
                }
            } else {
                setResult(oldUrl);
            }
        }
        Q_EMIT finished();
    });
}
