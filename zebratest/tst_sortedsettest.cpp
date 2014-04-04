#include <QString>
#include <QtTest>
#include "../sortedset.h"

class SortedSetTest : public QObject
{
    Q_OBJECT

public:
    SortedSetTest();

private Q_SLOTS:
    void testCase1();
};

SortedSetTest::SortedSetTest()
{
}

void SortedSetTest::testCase1()
{
    QVERIFY2(false, "Failure");
}

QTEST_APPLESS_MAIN(SortedSetTest)

#include "tst_sortedsettest.moc"
