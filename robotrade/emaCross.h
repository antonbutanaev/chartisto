#pragma once

#include <json/value.h>
#include <optional>
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
		size_t emaFrom = 10;
		size_t emaTo = 60;
		Json::Value risk;
		Json::Value titleToTicker;

		struct Risk {
			chart::Price maxLoss = 2000;
			chart::Price maxPosition = 200000;
		};

		Config(const std::string &jsonConfig);
		Risk getRisk(const std::string &title) const;
	};

	EMACross(const std::string &jsonConfig, unsigned verbose);
	void process(
		bool printSummary, bool printOrders,
		unsigned daysToAnalyze,
		const std::vector<std::string> &quoteFiles,
		const std::string &todayQuotesDir,
		unsigned seed,
		const std::string &exportStops
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
		struct Order {
			size_t period;
			EntryAnalyzer::Result result;
		};
		std::vector<Order> orders;
		std::string log;
	};

	TaskResult runTask(const TaskParam&, unsigned seed);

	util::Async async_;
	enum Verbose{Log = 1};
	unsigned verbose_;
	const Config config_;
};

}
