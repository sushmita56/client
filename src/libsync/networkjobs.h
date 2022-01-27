/*
 * Copyright (C) by Klaas Freitag <freitag@owncloud.com>
 * Copyright (C) by Daniel Molkentin <danimo@owncloud.com>
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

#ifndef NETWORKJOBS_H
#define NETWORKJOBS_H

#include "abstractnetworkjob.h"
#include "common/result.h"
#include <QJsonObject>
#include <QUrlQuery>
#include <functional>

class QUrl;

namespace OCC {

/** Strips quotes and gzip annotations */
OWNCLOUDSYNC_EXPORT QByteArray parseEtag(const QByteArray &header);

struct HttpError
{
    int code; // HTTP error code
    QString message;
};

template <typename T>
using HttpResult = Result<T, HttpError>;

/**
 * @brief The EntityExistsJob class
 * @ingroup libsync
 */
class OWNCLOUDSYNC_EXPORT EntityExistsJob : public AbstractNetworkJob
{
    Q_OBJECT
public:
    explicit EntityExistsJob(AccountPtr account, const QString &path, QObject *parent = nullptr);
    void start() override;

signals:
    void exists(QNetworkReply *);

private slots:
    bool finished() override;
};

/**
 * @brief The LsColJob class
 * @ingroup libsync
 */
class OWNCLOUDSYNC_EXPORT LsColXMLParser : public QObject
{
    Q_OBJECT
public:
    explicit LsColXMLParser();

    bool parse(const QByteArray &xml, QHash<QString, qint64> *sizes, const QString &expectedPath);

signals:
    void directoryListingSubfolders(const QStringList &items);
    void directoryListingIterated(const QString &name, const QMap<QString, QString> &properties);
    void finishedWithError(QNetworkReply *reply);
    void finishedWithoutError();
};

class OWNCLOUDSYNC_EXPORT LsColJob : public AbstractNetworkJob
{
    Q_OBJECT
public:
    explicit LsColJob(AccountPtr account, const QString &path, QObject *parent = nullptr);
    explicit LsColJob(AccountPtr account, const QUrl &url, QObject *parent = nullptr);
    void start() override;

    /**
     * Used to specify which properties shall be retrieved.
     *
     * The properties can
     *  - contain no colon: they refer to a property in the DAV: namespace
     *  - contain a colon: and thus specify an explicit namespace,
     *    e.g. "ns:with:colons:bar", which is "bar" in the "ns:with:colons" namespace
     */
    void setProperties(const QList<QByteArray> &properties);
    QList<QByteArray> properties() const;

    const QHash<QString, qint64> &sizes() const;

signals:
    void directoryListingSubfolders(const QStringList &items);
    void directoryListingIterated(const QString &name, const QMap<QString, QString> &properties);
    void finishedWithError(QNetworkReply *reply);
    void finishedWithoutError();

private slots:
    bool finished() override;

protected:
    void startImpl(const QNetworkRequest &req);

private:
    QList<QByteArray> _properties;
    QUrl _url; // Used instead of path() if the url is specified in the constructor
    QHash<QString, qint64> _sizes;
};

/**
 * @brief The PropfindJob class
 *
 * Setting the desired properties with setProperties() is mandatory.
 *
 * Note that this job is only for querying one item.
 * There is also the LsColJob which can be used to list collections
 *
 * @ingroup libsync
 */
class OWNCLOUDSYNC_EXPORT PropfindJob : public LsColJob
{
    Q_OBJECT
public:
    using LsColJob::LsColJob;
    void start() override;

signals:
    void result(const QMap<QString, QString> &values);

private:
    bool _done = false;
};

#ifndef TOKEN_AUTH_ONLY
/**
 * @brief Retrieves the account users avatar from the server using a GET request.
 *
 * If the server does not have the avatar, the result Pixmap is empty.
 *
 * @ingroup libsync
 */
class OWNCLOUDSYNC_EXPORT AvatarJob : public AbstractNetworkJob
{
    Q_OBJECT
public:
    /**
     * @param userId The user for which to obtain the avatar
     * @param size The size of the avatar (square so size*size)
     */
    explicit AvatarJob(AccountPtr account, const QString &userId, int size, QObject *parent = nullptr);

    void start() override;

    /** The retrieved avatar images don't have the circle shape by default */
    static QPixmap makeCircularAvatar(const QPixmap &baseAvatar);

signals:
    /**
     * @brief avatarPixmap - returns either a valid pixmap or not.
     */

    void avatarPixmap(const QPixmap &);

private slots:
    bool finished() override;

private:
    QUrl _avatarUrl;
};
#endif

/**
 * @brief Send a Proppatch request
 *
 * Setting the desired properties with setProperties() is mandatory.
 *
 * WARNING: Untested!
 *
 * @ingroup libsync
 */
class OWNCLOUDSYNC_EXPORT ProppatchJob : public AbstractNetworkJob
{
    Q_OBJECT
public:
    explicit ProppatchJob(AccountPtr account, const QString &path, QObject *parent = nullptr);
    void start() override;

    /**
     * Used to specify which properties shall be set.
     *
     * The property keys can
     *  - contain no colon: they refer to a property in the DAV: namespace
     *  - contain a colon: and thus specify an explicit namespace,
     *    e.g. "ns:with:colons:bar", which is "bar" in the "ns:with:colons" namespace
     */
    void setProperties(QMap<QByteArray, QByteArray> properties);
    QMap<QByteArray, QByteArray> properties() const;

signals:
    void success();
    void finishedWithError();

private slots:
    bool finished() override;

private:
    QMap<QByteArray, QByteArray> _properties;
};

/**
 * @brief The MkColJob class
 * @ingroup libsync
 */
class OWNCLOUDSYNC_EXPORT MkColJob : public AbstractNetworkJob
{
    Q_OBJECT
    QUrl _url; // Only used if the constructor taking a url is taken.
    QMap<QByteArray, QByteArray> _extraHeaders;

public:
    explicit MkColJob(AccountPtr account, const QString &path, QObject *parent = nullptr);
    explicit MkColJob(AccountPtr account, const QUrl &url,
        const QMap<QByteArray, QByteArray> &extraHeaders, QObject *parent = nullptr);
    void start() override;

signals:
    void finishedWithError(QNetworkReply *reply);
    void finishedWithoutError();

private:
    bool finished() override;
};

/**
 * @brief The CheckServerJob class
 * @ingroup libsync
 */
class OWNCLOUDSYNC_EXPORT CheckServerJob : public AbstractNetworkJob
{
    Q_OBJECT
public:
    explicit CheckServerJob(AccountPtr account, QObject *parent = nullptr);
    void start() override;

    static QString version(const QJsonObject &info);
    static QString versionString(const QJsonObject &info);
    static bool installed(const QJsonObject &info);

    int maxRedirectsAllowed() const;
    void setMaxRedirectsAllowed(int maxRedirectsAllowed);

signals:
    /** Emitted when a status.php was successfully read.
     *
     * \a url see _serverStatusUrl (does not include "/status.php")
     * \a info The status.php reply information
     */
    void instanceFound(const QUrl &url, const QJsonObject &info);

    /** Emitted on invalid status.php reply.
     *
     * \a reply is never null
     */
    void instanceNotFound(QNetworkReply *reply);

    /** A timeout occurred.
     *
     * \a url The specific url where the timeout happened.
     */
    void timeout(const QUrl &url);

private:
    bool finished() override;
    void onTimedOut() override;
private slots:
    virtual void metaDataChangedSlot();
    virtual void encryptedSlot();

private:
    bool _subdirFallback;

    /** The permanent-redirect adjusted account url.
     *
     * Note that temporary redirects or a permanent redirect behind a temporary
     * one do not affect this url.
     */
    QUrl _serverUrl;

    int _maxRedirectsAllowed = 5;

    /** we only got permanent redirects */
    bool _redirectDistinct = true;
    /** only retry once, the first try might give us needed cookies
        the second is supposed to succeed
    */
    bool _firstTry = true;
};


/**
 * @brief The RequestEtagJob class
 */
class OWNCLOUDSYNC_EXPORT RequestEtagJob : public AbstractNetworkJob
{
    Q_OBJECT
public:
    explicit RequestEtagJob(AccountPtr account, const QString &path, QObject *parent = nullptr);
    void start() override;

signals:
    void etagRetreived(const QByteArray &etag, const QDateTime &time);
    void finishedWithResult(const HttpResult<QByteArray> &etag);

private slots:
    bool finished() override;
};

/**
 * @brief Checks with auth type to use for a server
 * @ingroup libsync
 */
class OWNCLOUDSYNC_EXPORT DetermineAuthTypeJob : public AbstractNetworkJob
{
    Q_OBJECT
public:
    enum class AuthType {
        Basic, // also the catch-all fallback for backwards compatibility reasons
        OAuth,
        Unknown
    };
    Q_ENUM(AuthType)

    explicit DetermineAuthTypeJob(AccountPtr account, QObject *parent = nullptr);
    void start() override;
signals:
    void authType(AuthType);

protected Q_SLOTS:
    bool finished() override;
};

/**
 * @brief A basic job around a network request without extra funtionality
 * @ingroup libsync
 *
 * Primarily adds timeout and redirection handling.
 */
class OWNCLOUDSYNC_EXPORT SimpleNetworkJob : public AbstractNetworkJob
{
    Q_OBJECT
public:
    explicit SimpleNetworkJob(AccountPtr account, const QString &path, QObject *parent = nullptr);
    virtual ~SimpleNetworkJob();

    virtual void prepareQueryRequest(const QByteArray &verb, const QUrlQuery &arguments, const QNetworkRequest &req = QNetworkRequest());

    virtual void prepareJsonRequest(const QByteArray &verb, const QJsonObject &arguments, const QNetworkRequest &req = QNetworkRequest());

    virtual void prepareRequest(const QByteArray &verb, QIODevice *requestBody, const QNetworkRequest &req = QNetworkRequest());

    void start() override;


signals:
    void finishedSignal(QNetworkReply *reply);

protected:
    bool isPrepared() const;
    bool finished() override;

private:
    QUrl simpleUrl() const;
    QByteArray _simpleVerb;
    QIODevice *_simpleBody;
    QNetworkRequest _simpleRequest;
};

/**
 * @brief Runs a PROPFIND to figure out the private link url
 *
 * The numericFileId is used only to build the deprecatedPrivateLinkUrl
 * locally as a fallback. If it's empty and the PROPFIND fails, targetFun
 * will be called with an empty string.
 *
 * The job and signal connections are parented to the target QObject.
 *
 * Note: targetFun is guaranteed to be called only through the event
 * loop and never directly.
 */
void OWNCLOUDSYNC_EXPORT fetchPrivateLinkUrl(AccountPtr account, const QString &remotePath, QObject *target,
    std::function<void(const QString &url)> targetFun);

} // namespace OCC


#endif // NETWORKJOBS_H
