#pragma once

#include <util/async.h>
#include <util/funcIterator.h>
#include <chart/data.h>

namespace robotrade {

class EMACross {
public:
	EMACross();
	void process(util::FuncItreator<const std::string> quoteFiles);
private:
	struct Result {
		std::string title;
		size_t numQuotes;
	};
	Result process(chart::data::PBars bars);

	util::Async async_;

};

}
