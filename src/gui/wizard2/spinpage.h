#pragma once

#include "wizardpage.h"

#include <QWidget>

#include <functional>

namespace Ui {
class SpinPage;
}

namespace SetupWizard {

class SpinPage : public WizardPage
{
    Q_OBJECT

public:
    explicit SpinPage(const std::function<void(SpinPage *)> &&func, QWidget *parent = nullptr);
    ~SpinPage();
    void addWidget(QWidget *w);
    bool isComplete() const override;
    KnownPages type() const override;

    void setError(const QString &error);
    void stop();

private:
    void start();
    Ui::SpinPage *ui;
    bool _complete = false;
    QString _error;
    std::function<void(SpinPage *)> _func;
};
}
