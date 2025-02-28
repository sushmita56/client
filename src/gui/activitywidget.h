/*
 * Copyright (C) by Klaas Freitag <freitag@owncloud.com>
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

#ifndef ACTIVITYWIDGET_H
#define ACTIVITYWIDGET_H

#include <QDialog>
#include <QDateTime>
#include <QLocale>
#include <QAbstractListModel>
#include <chrono>

#include "progressdispatcher.h"
#include "owncloudgui.h"
#include "account.h"
#include "activitydata.h"

#include "ui_activitywidget.h"

class QPushButton;
class QProgressIndicator;

namespace OCC {

class Account;
class AccountStatusPtr;
class ProtocolWidget;
class IssuesWidget;
class JsonApiJob;
class NotificationWidget;
class ActivityListModel;

namespace Ui {
    class ActivityWidget;
}
class Application;

/**
 * @brief The ActivityWidget class
 * @ingroup gui
 *
 * The list widget to display the activities, contained in the
 * subsequent ActivitySettings widget.
 */

class ActivityWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ActivityWidget(QWidget *parent = nullptr);
    ~ActivityWidget() override;
    void storeActivityList(QTextStream &ts);

    /**
     * Adjusts the activity tab's and some widgets' visibility
     *
     * Based on whether activities are enabled and whether notifications are
     * available.
     */
    void checkActivityTabVisibility();

public slots:
    void slotRefreshActivities(AccountStatePtr ptr);
    void slotRefreshNotifications(AccountStatePtr ptr);
    void slotRemoveAccount(AccountStatePtr ptr);
    void slotAccountActivityStatus(AccountStatePtr ast, int statusCode);
    void slotRequestCleanupAndBlacklist(const Activity &blacklistActivity);

signals:
    void guiLog(const QString &, const QString &);
    void dataChanged();
    void hideActivityTab(bool);
    void newNotification();

private slots:
    void slotBuildNotificationDisplay(const ActivityList &list);
    void slotSendNotificationRequest(const QString &accountName, const QString &link, const QByteArray &verb);
    void slotNotifyNetworkError(QNetworkReply *);
    void slotNotifyServerFinished(const QString &reply, int replyCode);
    void endNotificationRequest(NotificationWidget *widget, int replyCode);
    void scheduleWidgetToRemove(NotificationWidget *widget, int milliseconds = 100);
    void slotCheckToCleanWidgets();

private:
    void slotItemContextMenu();
    void showLabels();
    QString timeString(QDateTime dt, QLocale::FormatType format) const;
    Ui::ActivityWidget *_ui;
    QPushButton *_copyBtn;

    QSet<QString> _accountsWithoutActivities;
    QMap<Activity::Identifier, NotificationWidget *> _widgetForNotifId;
    QElapsedTimer _guiLogTimer;
    QSet<Activity::Identifier> _guiLoggedNotifications;
    ActivityList _blacklistedNotifications;

    QHash<NotificationWidget *, QDateTime> _widgetsToRemove;
    QTimer _removeTimer;

    // number of currently running notification requests. If non zero,
    // no query for notifications is started.
    int _notificationRequestsRunning;

    ActivityListModel *_model;
    QSortFilterProxyModel *_sortModel;
    QVBoxLayout *_notificationsLayout;
};


/**
 * @brief The ActivitySettings class
 * @ingroup gui
 *
 * Implements a tab for the settings dialog, displaying the three activity
 * lists.
 */
class ActivitySettings : public QWidget
{
    Q_OBJECT
public:
    explicit ActivitySettings(QWidget *parent = nullptr);
    ~ActivitySettings() override;

public slots:
    void slotRefresh(AccountStatePtr ptr);
    void slotRemoveAccount(AccountStatePtr ptr);

    void setNotificationRefreshInterval(std::chrono::milliseconds interval);

    void slotShowIssuesTab();

private slots:
    void setActivityTabHidden(bool hidden);
    void slotRegularNotificationCheck();
    void slotShowIssueItemCount(int cnt);
    void slotShowActivityTab();

signals:
    void guiLog(const QString &, const QString &);

private:
    bool event(QEvent *e) override;

    QTabWidget *_tab;
    int _activityTabId;
    int _protocolTabId;
    int _syncIssueTabId;

    ActivityWidget *_activityWidget;
    ProtocolWidget *_protocolWidget;
    IssuesWidget *_issuesWidget;
    QProgressIndicator *_progressIndicator;
    QTimer _notificationCheckTimer;
    QHash<AccountStatePtr, QElapsedTimer> _timeSinceLastCheck;
};
}
#endif // ActivityWIDGET_H
