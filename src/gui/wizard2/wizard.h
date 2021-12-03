#pragma once

#include "wizard_exports.h"


#include <QDialog>

class QNetworkAccessManager;

namespace Ui {
class Wizard;
}

namespace SetupWizard {

class WizardPage;
class SpinPage;

enum class KnownPages {
    Serever,
    Oauth,
    Spinner

};

class WIZARD_EXPORT Wizard : public QDialog
{
    Q_OBJECT
public:
    explicit Wizard(QWidget *parent = nullptr);
    ~Wizard();

    void next();
    WizardPage *currentPage();
    WizardPage *addPage(WizardPage *page);

private:
    void checkServer(SpinPage *spin, const QUrl &url);
    void checkOauth(SpinPage *spin, const QUrl &url);


    ::Ui::Wizard *ui;
    QNetworkAccessManager *_nam;
    QVector<WizardPage *> _pages;
};
}
