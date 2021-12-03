#include "wizard.h"
#include "ui_wizard.h"

#include "oauthpage.h"
#include "serverpage.h"
#include "spinpage.h"
#include "wizardpage.h"

#include "jobs/resolveurl.h"

#include "libsync/accessmanager.h"
#include "libsync/determineauthtypejob2.h"
#include "libsync/networkjobs.h"

#include <QPushButton>
#include <QUrl>

using namespace SetupWizard;

Wizard::Wizard(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Wizard)
    , _nam(new OCC::AccessManager(this))
{
    // I think its
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    auto serverPage = new ServerPage();
    addPage(serverPage);
    addPage(new SpinPage([serverPage, this](SpinPage *spin) {
        checkServer(spin, serverPage->url());
    }));
    auto oauthPage = new SpinPage([serverPage, this](SpinPage *spin) {
        //        checkServer(spin, serverPage->url());
    });
    auto restartOauthButton = new QPushButton(tr("Reopen Browser"));
    connect(restartOauthButton, &QPushButton::clicked, [] {

    });
    oauthPage->addWidget(restartOauthButton);
    addPage(oauthPage);

    connect(ui->toolButton_next, &QToolButton::clicked, this, &Wizard::next);
}

Wizard::~Wizard()
{
    delete ui;
}

void Wizard::next()
{
    if (currentPage()->isComplete()) {
        ui->toolButton_next->setEnabled(false);
        const auto nextIndex = ui->stackedWidget->currentIndex() + 1;
        if (nextIndex >= _pages.size()) {
            // finish
        } else if (nextIndex > 0) {
            ui->stackedWidget->setCurrentIndex(nextIndex);
            ui->toolButton_back->setEnabled(true);
            auto page = qobject_cast<WizardPage *>(ui->stackedWidget->currentWidget());
            Q_EMIT page->entered();
        }
    }
}

WizardPage *Wizard::currentPage()
{
    return qobject_cast<WizardPage *>(ui->stackedWidget->currentWidget());
}

WizardPage *Wizard::addPage(WizardPage *page)
{
    _pages.append(page);
    ui->stackedWidget->addWidget(page);
    connect(page, &WizardPage::compleChanged, this, [page, this] {
        if (currentPage() == page) {
            if (page->isComplete()) {
                ui->toolButton_next->setEnabled(true);
                if (page->type() == KnownPages::Spinner) {
                    // continue
                    next();
                }
            }
        }
    });
    return page;
}

void Wizard::checkServer(SpinPage *spin, const QUrl &url)
{
    // TODO: webfinger

    auto resolveUrl = new ResolveUrl(_nam, this);
    connect(resolveUrl, &ResolveUrl::finished, this, [resolveUrl, spin, this] {
        resolveUrl->deleteLater();
        if (resolveUrl->error().isEmpty()) {
            auto getAuth = new OCC::DetermineAuthTypeJob2(_nam, this);
            connect(getAuth, &OCC::DetermineAuthTypeJob2::finished, this, [spin, getAuth, this] {
                getAuth->deleteLater();
                if (getAuth->error().isEmpty()) {
                    qDebug() << getAuth->result();
                    spin->stop();
                } else {
                    spin->setError(getAuth->error());
                }
            });
            getAuth->start(resolveUrl->result().toUrl());
        } else {
            spin->setError(resolveUrl->error());
        }
    });
    resolveUrl->start(url);
}
