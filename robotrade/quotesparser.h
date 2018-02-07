#ifndef QUOTESPARSER_H
#define QUOTESPARSER_H

#include <iosfwd>
#include <memory>
#include <chart/chart.h>

namespace robotrade {

class Bars;
std::unique_ptr<Bars> parse(std::istream&);

class Bars : public chart::data::Bars {
public:
	Bars();
	~Bars();
    std::string title() const override;
    chart::Time time(size_t) const override;
    chart::Price open(size_t) const override;
    chart::Price close(size_t) const override;
    chart::Price high(size_t) const override;
    chart::Price low(size_t) const override;
    chart::Volume volume(size_t) const override;

    size_t numBars() const override;

    friend std::unique_ptr<Bars> parse(std::istream&);
private:
    struct Impl;
    std::unique_ptr<Impl> i_;
};

}

#endif
