#pragma once

#include "wizard.h"

namespace SetupWizard {

// inspired by QWizardPage

class WizardPage : public QWidget
{
    Q_OBJECT
public:
    explicit WizardPage(QWidget *parent = nullptr);
    virtual ~WizardPage();

    virtual bool isComplete() const = 0;
    virtual KnownPages type() const = 0;

signals:
    void compleChanged();
    void entered();
};
}
