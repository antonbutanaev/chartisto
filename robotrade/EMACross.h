#pragma once

#include <vector>
#include <util/async.h>
#include <util/funcIterator.h>
#include <chart/data.h>
#include <robotrade/entryAnalyzer.h>
#include <util/hasher.h>

namespace robotrade {

class EMACross {
public:
	struct Config {
		double windowSizeK = 2.;
		double profitPerStopK = 3.;
		double maxMovePerAtrK = 2.;
	};

	EMACross(unsigned verbose);
	void process(
		bool printSummary, bool printOrders,
		unsigned daysToAnalyze, const std::vector<std::string> &quoteFiles, unsigned seed
	);
private:
	struct PriceInfo {
		chart::data::PBars bars;
		util::umap<size_t, chart::data::PPoints> emas;
		util::umap<size_t, chart::data::PPoints> atrs;
	};

	struct TaskParam {
		size_t barNum;
		const PriceInfo &priceInfo;
	};
	struct TaskResult {
		std::string title;
		std::vector<EntryAnalyzer::Result> results;
		std::string log;
	};

	TaskResult runTask(const TaskParam&, unsigned seed, const Config&);

	util::Async async_;
	enum Verbose{Log = 1};
	unsigned verbose_;
};

}
