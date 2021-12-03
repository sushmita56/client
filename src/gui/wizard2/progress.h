#pragma once

#include <QWidget>

namespace Ui {
class Progress;
}

namespace SetupWizard {
class Progress : public QWidget
{
    Q_OBJECT

public:
    explicit Progress(QWidget *parent = nullptr);
    ~Progress();

private:
    Ui::Progress *ui;
};

}
