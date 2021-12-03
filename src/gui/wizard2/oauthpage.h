#pragma once

#include "wizardpage.h"

#include <QWidget>

namespace Ui {
class OauthPage;
}

namespace SetupWizard {

class OauthPage : public WizardPage
{
    Q_OBJECT

public:
    explicit OauthPage(QWidget *parent = nullptr);
    ~OauthPage();

private:
    Ui::OauthPage *ui;

    // WizardPage interface
public:
    virtual bool isComplete() const override;
    virtual KnownPages type() const override;
};

}
