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
	EMACross();
	void process(const std::vector<std::string> &quoteFiles, unsigned seed);
private:
	struct Parsed {
		chart::data::PBars bars;
		util::umap<size_t, chart::data::PPoints> emas;
		util::umap<size_t, chart::data::PPoints> atrs;
	};

	struct TaskParams {
		size_t barNum;
		const Parsed &parsed;
	};
	struct TaskResult {
		std::string title;
		std::vector<EntryAnalyzer::Result> results;
		std::string log;
	};

	TaskResult runTask(const TaskParams&, unsigned seed);

	util::Async async_;
};

}
