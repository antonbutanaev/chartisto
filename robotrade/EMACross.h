#pragma once

#include <util/async.h>
#include <util/funcIterator.h>
#include <chart/data.h>

namespace robotrade {

class EMACross {
public:
	EMACross();
	void process(const std::vector<std::string> &quoteFiles);
private:
	struct TaskParams {
		size_t emaPeriod;
		chart::data::PPoints ema;
		chart::data::PBars bars;
	};
	struct TaskResult {
		std::string log;
	};

	TaskResult runTask(const TaskParams&);

	util::Async async_;
};

}
