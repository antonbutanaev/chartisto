#include <QString>
#include <QtTest>

class Test : public QObject
{
    Q_OBJECT

public:
    Test();

private Q_SLOTS:
    void testPainter();
};

Test::Test()
{
}

void Test::testPainter()
{
    QVERIFY2(true, "Failure");
}

QTEST_APPLESS_MAIN(Test)

#include "testpainter.moc"
