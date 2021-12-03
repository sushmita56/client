#include "progress.h"
#include "ui_progress.h"

using namespace SetupWizard;

Progress::Progress(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Progress)
{
    ui->setupUi(this);
}

Progress::~Progress()
{
    delete ui;
}
