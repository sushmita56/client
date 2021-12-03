#pragma once

#include "abstractcorejob.h"

#include "owncloudlib.h"

namespace OCC {


class OWNCLOUDSYNC_EXPORT DetermineAuthTypeJob2 : public AbstractCoreJob
{
    Q_OBJECT
public:
    enum class AuthType {
        Basic, // also the catch-all fallback for backwards compatibility reasons
        OAuth,
        Unknown
    };
    Q_ENUM(AuthType)

    DetermineAuthTypeJob2(QNetworkAccessManager *nam, QObject *parent = nullptr);
    ~DetermineAuthTypeJob2() override;

    void start(const QUrl &url) override;
};

}
