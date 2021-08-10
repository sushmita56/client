/*
 * Copyright (C) by Roeland Jago Douma <roeland@famdouma.nl>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#include "graphapijob.h"
#include "graphapi.h"
#include "networkjobs.h"
#include "account.h"

#include <QBuffer>
#include <QJsonDocument>
#include <QJsonObject>

namespace OCC {

Q_LOGGING_CATEGORY(lcgraphapi, "libsync.graphApi", QtInfoMsg)

GraphMeDrivesJob::GraphMeDrivesJob(AccountPtr account)
    : GraphApiJob(account)
{
    setIgnoreCredentialFailure(true);
    setPath("graph/v1.0/me/drives");
}

void GraphMeDrivesJob::start()
{

}

}
