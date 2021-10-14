#pragma once

#include "networkjobs/jsonjob.h"

#include "owncloudlib.h"

namespace OCC {
namespace GraphApi {
    /*
    {
        "value": [
            {
                "driveType": "personal",
                "id": "1284d238-aa92-42ce-bdc4-0b0000009157!62aaece8-aa17-4064-ab55-2446bccab4c3",
                "lastModifiedDateTime": "2021-10-15T13:20:35.425697673Z",
                "name": "Albert Einstein",
                "owner": {
                    "user": {
                        "id": "4c510ada-c86b-4815-8820-42cdf82c3d51"
                    }
                },
                "quota": {
                    "remaining": 35012505600,
                    "state": "normal",
                    "total": 35012505600,
                    "used": 0
                },
                "root": {
                    "id": "1284d238-aa92-42ce-bdc4-0b0000009157!62aaece8-aa17-4064-ab55-2446bccab4c3",
                    "webDavUrl": "https://localhost:9200/dav/spaces/1284d238-aa92-42ce-bdc4-0b0000009157!62aaece8-aa17-4064-ab55-2446bccab4c3"
                }
            }
        ]
    }
     */

    namespace OpenGraph {

        struct Quota
        {
            uint64_t remaining;
            QString state;
            uint64_t total;
            uint64_t used;

            static Quota fromJson(const QJsonObject &obj)
            {
                return {
                    obj[QLatin1String("remaning")].toVariant().value<uint64_t>(),
                    obj[QLatin1String("state")].toString(),
                    obj[QLatin1String("total")].toVariant().value<uint64_t>(),
                    obj[QLatin1String("used")].toVariant().value<uint64_t>()
                };
            }
        };

        struct Root
        {
            QString id;
            QUrl webDavUrl;

            static Root fromJson(const QJsonObject &obj)
            {
                return {
                    obj[QLatin1String("id")].toString(),
                    QUrl::fromEncoded(obj[QLatin1String("webDavUrl")].toString().toUtf8())
                };
            }
        };

        struct DrivesData
        {
            QString driveType;
            QString id;
            QDateTime lastModifiedDateTime;
            QString name;
            /**
                "owner": {
                    "user": {
                        "id": "4c510ada-c86b-4815-8820-42cdf82c3d51"
                    }
                },
                **/
            Quota quota;
            Root root;

            static DrivesData fromJson(const QJsonObject &obj)
            {
                return {
                    obj[QLatin1String("driveType")].toString(),
                    obj[QLatin1String("id")].toString(),
                    // 2021-11-08T06:30:22.772870649Z
                    QDateTime::fromString(obj[QLatin1String("lastModifiedDateTime")].toString(), Qt::ISODate),
                    obj[QLatin1String("name")].toString(),
                    // owner,
                    Quota::fromJson(obj[QLatin1String("quota")].toObject()),
                    Root::fromJson(obj[QLatin1String("root")].toObject())
                };
            }
        };

    }

    class OWNCLOUDSYNC_EXPORT Drives : public JsonJob
    {
        Q_OBJECT
    public:
        Drives(const AccountPtr &account, QObject *parent = nullptr);
        ~Drives();

        QVector<OpenGraph::DrivesData> drives() const;
    };
}
}
