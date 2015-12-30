// redisOpt.cpp : 
//
#include "redisOpt.h"
#include <boost/lexical_cast.hpp>
#include <string>
//
using namespace std;
using namespace boost;

namespace com { namespace autotrade { namespace util {

int RedisOpt::init(const std::string & host , uint16_t port, int dbindex ){
	try{
		m_strHost = host;
		m_nPort = port;
		m_nDbindex = dbindex;
		boost::shared_ptr<redis::client> shared_c ( new redis::client(m_strHost,m_nPort,m_nDbindex));
		m_sptrRedis = shared_c;
	}catch (redis::redis_error & e) {
		cerr << "got exception: " << e.what() << endl << "FAIL" << endl;
		return REDIS_OPT_ERROR_CODE::REDIS_OPT_INIT_FAIL;
	}
	return REDIS_OPT_ERROR_CODE::REDIS_OPT_OK;
}

int RedisOpt::set_day_raw_data(int day,int time,const char* code,int price){
	std::string strPrice;
	//cout << day << endl;
	//cout << time << endl;
	//cout << code << endl;
	//cout << price << endl;
	string str_day = boost::lexical_cast<string>(day);
	string str_time = boost::lexical_cast<string>(time);
	string str_code = boost::lexical_cast<string>(code);
	string str_price = boost::lexical_cast<string>(price);
	return set_day_data(str_day,str_time,str_code,str_price);
}

int RedisOpt::set_day_data(std::string& day,std::string& time,std::string& code,std::string& price){
	try{
		if(code.length() == 6 && (code.c_str()[0] == '6' || code.c_str()[0] == '0' || code.c_str()[0] == '3' )){
			static int num = 0;
			num++;
			m_sptrRedis->hset("day:" + day,code,price);
			//m_sptrRedis->hset("stock:" + code,day,price);
			//std::string code_cur = code + "-hour";
			//m_sptrRedis->hset(code_cur,time,price);
			//
			string timestamp = day.substr(0,4) + "-" +  day.substr(4,2) + "-" + day.substr(6,2) + " " + time.substr(0,2) + ":" + time.substr(2,2) + ":" + time.substr(4,2);
			//
			//std::ofstream dataLog;
			//dataLog.open("e:\\" + day + ".csv", std::ios_base::app);
			//dataLog << timestamp << "," << code <<"," << price <<endl;
			if(num % 1000 == 0){
				cout << timestamp << "," << code <<"," << price << "," << num << endl;
			}
		}
	}catch (redis::redis_error & e) {
		cerr << "got exception: " << e.what() << endl << "FAIL" << endl;
		return REDIS_OPT_ERROR_CODE::REDIS_OPT_SET_DAY_DATA_ERROR;
	}
	return REDIS_OPT_ERROR_CODE::REDIS_OPT_OK;
}


}}} // namespace












