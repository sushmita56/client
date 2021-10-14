#include "drives.h"

#include <QJsonArray>

using namespace OCC;
using namespace GraphApi;

Drives::Drives(const AccountPtr &account, QObject *parent)
    : JsonJob(account, QStringLiteral("/graph/v1.0/me/drives"), parent)
{
}

Drives::~Drives()
{
}

QVector<OpenGraph::DrivesData> Drives::drives() const
{
    const auto array = data().value(QLatin1String("value")).toArray();
    QVector<OpenGraph::DrivesData> out;
    out.reserve(array.size());
    for (const auto &d : array) {
        out << OpenGraph::DrivesData::fromJson(d.toObject());
    }
    return out;
}
