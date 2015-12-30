#ifndef REDISOPT_H
#define REDISOPT_H

#include "redisclient.h"

#include <iostream>
#include <boost/date_time.hpp>

//
using namespace std;


namespace com { namespace autotrade { namespace util {
	const int REDIS_OPT_ERROR_NUMBER_BASE = 100;
	struct REDIS_OPT_ERROR_CODE {
		enum type {
			REDIS_OPT_OK = 0,
			REDIS_OPT_INIT_FAIL = REDIS_OPT_ERROR_NUMBER_BASE + 1,
			REDIS_OPT_SET_DAY_DATA_ERROR = REDIS_OPT_ERROR_NUMBER_BASE + 2
		};
	};

class RedisOpt{
public:
	
	RedisOpt(const std::string & host = "localhost", uint16_t port = 6379, int dbindex = 0)
	: m_strHost(host), m_nPort(port), m_nDbindex(dbindex){}
	
	int init(const std::string & host = "localhost", uint16_t port = 6379, int dbindex = 0);
	int set_day_data(std::string& day,std::string& time,std::string& code,std::string& price);
	int set_day_raw_data(int day,int time,const char* code,int price);

public:
	std::string m_strHost;
    boost::uint16_t m_nPort;
    int m_nDbindex;
private:
	boost::shared_ptr<redis::client> m_sptrRedis;
};


}}} // namespace

#endif






