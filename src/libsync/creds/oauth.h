/*
 * Copyright (C) by Olivier Goffart <ogoffart@woboq.com>
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

#pragma once
#include "accountfwd.h"
#include "owncloudlib.h"

#include <QNetworkReply>
#include <QPointer>
#include <QTcpServer>
#include <QUrl>


namespace OCC {
class SimpleNetworkJob;

/**
 * Job that do the authorization grant and fetch the access token
 *
 * Normal workflow:
 *
 *   --> start()
 *       |
 *       +----> fetchWellKnown() query the ".well-known/openid-configuration" endpoint
 *       |
 *       +----> openBrowser() open the browser after fetchWellKnown finished to the specified page
 *       |                    (or the default 'oauth2/authorize' if fetchWellKnown does not exist)
 *       |                    Then the browser will redirect to http://localhost:xxx
 *       |
 *       +----> _server starts listening on a TCP port waiting for an HTTP request with a 'code'
 *                |
 *                v
 *             request the access_token and the refresh_token via 'apps/oauth2/api/v1/token'
 *                |
 *                +-> Request the user_id is not present
 *                |     |
 *                v     v
 *              finalize(...): emit result(...)
 *
 */
class OWNCLOUDSYNC_EXPORT OAuth : public QObject
{
    Q_OBJECT
public:
    enum Result { NotSupported,
        LoggedIn,
        Error };
    Q_ENUM(Result);

    OAuth(Account *account, QObject *parent);
    ~OAuth() override;
    void startAuthentication();
    void refreshAuthentication(const QString &refreshToken);
    void openBrowser();
    QUrl authorisationLink() const;
    /**
     * Call the callback when the call to the well-known endpoint finishes.
     * (or immediatly if it is ready)
     * The callback will not be called if this object gets destroyed
     */
    void authorisationLinkAsync(std::function<void(const QUrl&)> callback) const;

signals:
    /**
     * The state has changed.
     * when logged in, token has the value of the token.
     */
    void result(OAuth::Result result, const QString &user = QString(), const QString &token = QString(), const QString &refreshToken = QString());

    /**
     * emitted when the call to the well-known endpoint is finished
     */
    void authorisationLinkChanged(const QUrl &);

    void refreshError(QNetworkReply::NetworkError error, const QString &errorString);
    void refreshFinished(const QString &accessToken, const QString &refreshToken);

    void fetchWellKnownFinished();

private:
    void fetchWellKnown();
    void finalize(const QPointer<QTcpSocket> &socket, const QString &accessToken,
        const QString &refreshToken, const QString &userId, const QUrl &messageUrl);

    QNetworkReply *postTokenRequest(const QList<QPair<QString, QString>> &queryItems);

    QByteArray generateRandomString(size_t size) const;


    Account* _account;
    QTcpServer _server;
    bool _wellKnownFinished = false;

    QString _clientId;
    QString _clientSecret;

    QUrl _authEndpoint;
    QUrl _tokenEndpoint;
    QUrl _registrationEndpoint;
    QString _redirectUrl;
    QByteArray _pkceCodeVerifier;
    QByteArray _state;

    bool _isRefreshingToken = false;
};

} // namespace OCC
