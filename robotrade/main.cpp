#include <date.h>
#include <iostream>
#include <fstream>
#include <json/json.h>
#include <boost/program_options.hpp>

using namespace date;

date::year_month_day beginning_of_week(date::year_month_day ymd) {
    sys_days sd = ymd;
    return sd - (weekday{sd} - mon);
}

int main(int ac, char *av[]) {
	namespace po = boost::program_options;
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("compression", po::value<int>(), "set compression level")
	;

	po::variables_map vm;
	po::store(po::parse_command_line(ac, av, desc), vm);
	po::notify(vm);   

	if (vm.count("help")) {
		std::cout << desc << "\n";
		return 1;
	}

	const auto d1 = jul/4/2001;
	const auto wd = weekday(d1);
	std::cout << d1 << ":" << weekday(d1) << '\n';
	
	const auto d2 = beginning_of_week(d1);
	const auto wd2 = weekday(d2);
	std::cout << d2 << ":" << weekday(wd2) << '\n';

	if (0) {
		Json::Value config;
		std::fstream("file") >> config;
	}
}
