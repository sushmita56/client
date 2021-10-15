/*
   This software is in the public domain, furnished "as is", without technical
   support, and with no warranty, express or implied, as to its usefulness for
   any purpose.
*/

#include <QtTest>

#include "updater/updater.h"
#include "updater/ocupdater.h"

using namespace OCC;

class TestUpdater : public QObject
{
    Q_OBJECT

private slots:
    void testVersionToInt()
    {
        auto lowVersion = Updater::Helper::versionToInt(1, 2, 80, 3000);
        QCOMPARE(Updater::Helper::stringVersionToInt("1.2.80.3000"), lowVersion);

        auto highVersion = Updater::Helper::versionToInt(999, 2, 80, 3000);
        auto currVersion = Updater::Helper::currentVersionToInt();
        QVERIFY(currVersion > 0);
        QVERIFY(currVersion > lowVersion);
        QVERIFY(currVersion < highVersion);
    }

};

QTEST_APPLESS_MAIN(TestUpdater)
#include "testupdater.moc"
