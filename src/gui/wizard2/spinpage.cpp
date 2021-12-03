#include "spinpage.h"
#include "ui_spinpage.h"

using namespace SetupWizard;

SpinPage::SpinPage(const std::function<void(SpinPage *)> &&func, QWidget *parent)
    : WizardPage(parent)
    , ui(new Ui::SpinPage)
    , _func(std::move(func))
{
    ui->setupUi(this);
    connect(this, &WizardPage::entered, this, &SpinPage::start);
}

SpinPage::~SpinPage()
{
    delete ui;
}

void SpinPage::addWidget(QWidget *w)
{
    ui->layout->addWidget(w);
}

bool SpinPage::isComplete() const
{
    return _complete;
}

KnownPages SpinPage::type() const
{
    return KnownPages::Spinner;
}

void SpinPage::start()
{
    Q_ASSERT(!_complete);
    ui->spinenr->startAnimation();
    _func(this);
}

void SpinPage::stop()
{
    ui->spinenr->stopAnimation();
    _complete = true;
    Q_EMIT compleChanged();
}

void SpinPage::setError(const QString &error)
{
    ui->spinenr->stopAnimation();
    ui->labelMessage->setText(error);
}
