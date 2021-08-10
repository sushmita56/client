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

#ifndef GRAPHAPI_H
#define GRAPHAPI_H

#include "accountfwd.h"
#include "graphapijob.h"

#include <QVector>
#include <QJsonDocument>
#include <QList>
#include <QPair>
#include <QUrl>

#define OCS_SUCCESS_STATUS_CODE 100
// Apparantly the v2.php URLs can return that
#define OCS_SUCCESS_STATUS_CODE_V2 200

namespace OCC {

/**
 * @brief The OcsShareJob class
 * @ingroup gui
 *
 * Base class for jobs that talk to the OCS endpoints on the server.
 * All the communication logic is handled in this class.
 *
 * All OCS jobs (e.g. sharing) should extend this class.
 */

class GraphMeDrivesJob : public GraphApiJob
{
    Q_OBJECT

public:
    GraphMeDrivesJob(AccountPtr ptr);
    void start() override;
};

}

#endif //
