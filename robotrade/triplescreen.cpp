#include <robotrade/triplescreen.h>

using namespace chart;
using namespace std;

namespace robotrade {

struct TripleScreen::Impl {
	Impl(data::PBars weekly, data::PBars daily, Criteria criteria) {

	}

	StategyResult run() {
		return {};
	}

};

TripleScreen::TripleScreen(data::PBars weekly, data::PBars daily, Criteria criteria) :
i_(new Impl(weekly, daily, criteria))
{
}

TripleScreen::~TripleScreen() {}

StategyResult TripleScreen::run() {
	return i_->run();
}

}
