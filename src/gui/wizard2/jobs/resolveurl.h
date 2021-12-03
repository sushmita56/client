#pragma once

#include "libsync/abstractcorejob.h"

#include <QObject>

namespace SetupWizard {

class ResolveUrl : public OCC::AbstractCoreJob
{
    Q_OBJECT
public:
    explicit ResolveUrl(QNetworkAccessManager *nam, QObject *parent = nullptr);

    void start(const QUrl &url) override;
};
}
