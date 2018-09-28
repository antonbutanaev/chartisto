#include <robotrade/EntryAnalyzer.h>

namespace robotrade {

std::optional<EntryAnalyzer::Result> EntryAnalyzer::analyze(
	Direction /*direction*/,
    chart::Price /*limitOrderPrice*/
) {
	return {};
}

}
