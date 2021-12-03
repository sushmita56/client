#pragma once

#include "wizardpage.h"

#include <QWidget>

namespace Ui {
class ServerPage;
}


namespace SetupWizard {
class ServerPage : public WizardPage
{
    Q_OBJECT

public:
    explicit ServerPage(QWidget *parent = nullptr);
    ~ServerPage();

    bool isComplete() const override;

    KnownPages type() const override;

    QUrl url() const;

private:
    Ui::ServerPage *ui;
};
}
