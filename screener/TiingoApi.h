#ifndef SCREENER_TIINGOAPI_H_
#define SCREENER_TIINGOAPI_H_

#include <memory>
#include <json/json.h>
#include "ProcessTickers.h"

namespace tiingo {

class TiingoApi {
public:
	TiingoApi(const std::string &authToken);
	~TiingoApi();

	Json::Value getData(const std::string &ticker, Date from, Date to);

private:
	struct Impl;
	std::unique_ptr<Impl> i_;
};

}

#endif
