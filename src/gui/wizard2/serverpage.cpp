#include "serverpage.h"
#include "ui_serverpage.h"

#include "libsync/theme.h"

#include <QDebug>
#include <QTimer>
#include <QUrl>

using namespace OCC;
using namespace SetupWizard;

ServerPage::ServerPage(QWidget *parent)
    : WizardPage(parent)
    , ui(new Ui::ServerPage)
{
    ui->setupUi(this);

    connect(ui->lineEdit, &QLineEdit::editingFinished, this, [&] {
        // assume we have a perfect url fromUserInput would default to http://
        const auto testUrl = QUrl::fromEncoded(ui->lineEdit->text().simplified().toUtf8());
        if (!testUrl.isEmpty() && testUrl.scheme().isEmpty()) {
            // no scheme defined, default to https
            auto currentUrl = url();
            currentUrl.setScheme(QStringLiteral("https"));
            ui->lineEdit->setText(currentUrl.url());
        }
        Q_EMIT compleChanged();
    });

    const auto serverUrl = Theme::instance()->overrideServerUrlV2();
    if (!serverUrl.isEmpty()) {
        ui->lineEdit->setText(serverUrl);
        //        ui->lineEdit->setReadOnly(true);
        QTimer::singleShot(0, this, &ServerPage::compleChanged);
    }
}

ServerPage::~ServerPage()
{
    delete ui;
}

bool ServerPage::isComplete() const
{
    return url().isValid();
}

KnownPages ServerPage::type() const
{
    return KnownPages::Serever;
}

QUrl ServerPage::url() const
{
    return QUrl::fromUserInput(ui->lineEdit->text().simplified());
}
