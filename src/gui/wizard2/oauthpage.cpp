#include "oauthpage.h"
#include "ui_oauthpage.h"

using namespace SetupWizard;

OauthPage::OauthPage(QWidget *parent)
    : WizardPage(parent)
    , ui(new Ui::OauthPage)
{
    // TODO: add web icon to button
    ui->setupUi(this);
}

OauthPage::~OauthPage()
{
    delete ui;
}

bool OauthPage::isComplete() const
{
    return false;
}

KnownPages OauthPage::type() const
{
    return KnownPages::Oauth;
}
