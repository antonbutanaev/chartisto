#include <gtest/gtest.h>
#include <chart/chart.h>

using namespace chart;

class EmptyCandles: public Candles {
    int size() const override {return 0;}
    time_t time(int) const override {return NoTime;}
    Price open(int) const override {return NoPrice;}
    Price close(int) const override {return NoPrice;}
    Price high(int) const override {return NoPrice;}
    Price low(int) const override {return NoPrice;}
    Volume volume(int) const override {return 0;}
};

class EmptyPoints: public Points {
public:
    int size(int) const override {return 0;}
    time_t time(int) const override {return 0;}
    Price close(int) const override {return NoPrice;}
};

class EmptyChart: public Chart {
    int numCandlesData() const override {return 0;}
    int numPointData() const override {return 0;}

    Candles &candles(int) const override {
        static EmptyCandles candles;
        return candles;
    }

    Points &points(int) const override {
        static EmptyPoints points;
        return points;
    }
};

TEST(TestChart, AddEmptyChart) {
	Canvas canvas;
	EXPECT_EQ(canvas.numCharts(), 0);
	canvas.addChart(new EmptyChart);
	EXPECT_EQ(canvas.numCharts(), 1);
}
