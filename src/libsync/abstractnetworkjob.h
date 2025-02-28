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

#pragma once

#include "accountfwd.h"
#include "jobqueue.h"

#include "common/asserts.h"

#include "owncloudlib.h"

#include <QObject>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPointer>
#include <QElapsedTimer>
#include <QDateTime>
#include <QTimer>

#include <chrono>

class QUrl;


OWNCLOUDSYNC_EXPORT QDebug operator<<(QDebug debug, const OCC::AbstractNetworkJob *job);

namespace OCC {

class AbstractSslErrorHandler;

/**
 * @brief The AbstractNetworkJob class
 * @ingroup libsync
 */
class OWNCLOUDSYNC_EXPORT AbstractNetworkJob : public QObject
{
    Q_OBJECT
public:
    explicit AbstractNetworkJob(AccountPtr account, const QString &path, QObject *parent = nullptr);
    ~AbstractNetworkJob() override;

    virtual void start();

    AccountPtr account() const { return _account; }

    void setPath(const QString &path);
    QString path() const { return _path; }

    QUrl url() const { return _request.url(); }

    QNetworkReply *reply() const { return _reply; }

    void setIgnoreCredentialFailure(bool ignore);
    bool ignoreCredentialFailure() const { return _ignoreCredentialFailure; }

    QByteArray responseTimestamp();
    /* Content of the X-Request-ID header. (Only set after the request is sent) */
    QByteArray requestId();

    auto timeoutSec() const { return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::milliseconds(_timer.interval())); }
    bool timedOut() const { return _timedout; }

    /** Returns an error message, if any. */
    QString errorString() const;

    /** Like errorString, but also checking the reply body for information.
     *
     * Specifically, sometimes xml bodies have extra error information.
     * This function reads the body of the reply and parses out the
     * error information, if possible.
     *
     * \a body is optinally filled with the reply body.
     *
     * Warning: Needs to call reply()->readAll().
     */
    QString errorStringParsingBody(QByteArray *body = nullptr);

    /** Make a new request */
    void retry();

    /** Abort the job due to an error */
    void abort();

    /** static variable the HTTP timeout. If set to 0, the default will be used
     */
    static std::chrono::seconds httpTimeout;

    /** whether or noth this job should be restarted after authentication */
    bool  isAuthenticationJob() const;
    void  setAuthenticationJob(bool b);

    /** How many times was that job retried */
    int retryCount() const { return _retryCount; }


    virtual bool needsRetry() const;

public slots:
    void setTimeout(const std::chrono::seconds sec);
    void resetTimeout();
signals:
    /** Emitted on network error.
     *
     * \a reply is never null
     */
    void networkError(QNetworkReply *reply);
    void networkActivity();

protected:
    /** Initiate a network request, returning a QNetworkReply.
     *
     * Calls setReply() and setupConnections() on it.
     *
     * Takes ownership of the requestBody (to allow redirects).
     */
    void sendRequest(const QByteArray &verb, const QUrl &url,
        const QNetworkRequest &req = QNetworkRequest(),
        QIODevice *requestBody = nullptr);

    void setReply(QNetworkReply *reply);

    /** Makes this job drive a pre-made QNetworkReply
     *
     * This reply cannot have a QIODevice request body because we can't get
     * at it and thus not resend it in case of redirects.
     */
    void adoptRequest(QNetworkReply *reply);

    void setupConnections(QNetworkReply *reply);

    /** Can be used by derived classes to set up the network reply.
     *
     * Particularly useful when the request is redirected and reply()
     * changes. For things like setting up additional signal connections
     * on the new reply.
     */
    virtual void newReplyHook(QNetworkReply *) {}

    /// Creates a url for the account from a relative path
    QUrl makeAccountUrl(const QString &relativePath) const;

    /// Like makeAccountUrl() but uses the account's dav base path
    QUrl makeDavUrl(const QString &relativePath) const;

    /** Called at the end of QNetworkReply::finished processing.
     *
     * Returning true triggers a deleteLater() of this job.
     */
    virtual bool finished() = 0;

    /** Called on timeout.
     *
     * The default implementation aborts the reply.
     */
    virtual void onTimedOut();

    QByteArray _responseTimestamp;
    bool _timedout; // set to true when the timeout slot is received

    QString replyStatusString();

private slots:
    void slotFinished();
    void slotTimeout();

protected:
    AccountPtr _account;

private:
    QNetworkReply *addTimer(QNetworkReply *reply);
    bool _ignoreCredentialFailure;
    QNetworkRequest _request;
    QByteArray _verb;
    QPointer<QNetworkReply> _reply; // (QPointer because the NetworkManager may be destroyed before the jobs at exit)
    QString _path;
    QTimer _timer;
    int _http2ResendCount = 0;

    // Set by the xyzRequest() functions and needed to be able to redirect
    // requests, should it be required.
    //
    // Reparented to the currently running QNetworkReply.
    QPointer<QIODevice> _requestBody;

    bool _isAuthenticationJob = false;
    int _retryCount = 0;

    friend QDebug(::operator<<)(QDebug debug, const AbstractNetworkJob *job);
};

/**
 * @brief Internal Helper class
 */
class NetworkJobTimeoutPauser
{
public:
    NetworkJobTimeoutPauser(QNetworkReply *reply);
    ~NetworkJobTimeoutPauser();

private:
    QPointer<QTimer> _timer;
};


/** Gets the SabreDAV-style error message from an error response.
 *
 * This assumes the response is XML with a 'error' tag that has a
 * 'message' tag that contains the data to extract.
 *
 * Returns a null string if no message was found.
 */
QString OWNCLOUDSYNC_EXPORT extractErrorMessage(const QByteArray &errorResponse);

/** Builds a error message based on the error and the reply body. */
QString OWNCLOUDSYNC_EXPORT errorMessage(const QString &baseError, const QByteArray &body);

/** Nicer errorString() for QNetworkReply
 *
 * By default QNetworkReply::errorString() often produces messages like
 *   "Error downloading <url> - server replied: <reason>"
 * but the "downloading" part invariably confuses people since the
 * error might very well have been produced by a PUT request.
 *
 * This function produces clearer error messages for HTTP errors.
 */
QString OWNCLOUDSYNC_EXPORT networkReplyErrorString(const QNetworkReply &reply);

} // namespace OCC

