#include "hqtdfapi3.h"
//#include "ConfigParser.h"
//#include "ConfigSettings.h"
#include "PathHelper.h"

#include <iostream>
#include <boost/date_time.hpp>

#include <boost/date_time/gregorian/gregorian.hpp>   
#define BOOST_DATE_TIME_SOURCE  

using namespace std;
using namespace com::autotrade::common;
using namespace com::autotrade::hqtdfapi3;

extern com::autotrade::hqtdfapi3::HqTdfApi3* g_pHqTdfApi3Sdk;





int main(char*argv[], int argc){
	HqTdfApi3 tdf3;
	int ret = tdf3.init();
	if(HQ_TDFAPI_ERROR_CODE::TDFAPI_OK != ret){
		Print("HqTdfApi3 init() failed ! 请输入Enter 结束程序!\n");
		return ret;
	}
	//
	g_pHqTdfApi3Sdk = &tdf3;
	//
	SetConsoleCtrlHandler(g_ConsoleHandler, TRUE);
	g_pHqTdfApi3Sdk->m_hEventDataArrived = CreateEvent(NULL, FALSE, FALSE, NULL);
	g_pHqTdfApi3Sdk->m_hSubWriteThread = CreateThread(NULL, 0, g_SubWriteThread, NULL, 0, 0);
	//
	//1.fisrt get all market code.

#if 1
	Print("请输入q结束程序!\n");
	Print("\td 删除一个订阅\n");
	Print("\ts 设置订阅\n");
	Print("\tc 清除订阅\n");
	Print("\ta 添加订阅\n");

	char szCmd[128];
	std::cin >> szCmd;

	auto split = [](char* str, const char* sc)->std::vector<std::string>{
		std::vector<std::string> ret;
		char* nextToken = nullptr;
		auto s = strtok_s(str, sc, &nextToken);
		while(s != NULL){
			ret.push_back(s);
			s = strtok_s(nullptr, sc, &nextToken);
		}
		return ret;
	};

	auto tostr = [](std::vector<std::string> vec)->std::string{
		std::string ret;
		for (int i = 1; i < vec.size(); ++i){
			ret += vec[i];
			ret += ";";
		}
		return ret;
	};


	auto outCodeTable = [&](const char* szMarket){
		std::fstream f;
		char szFile[255];
		sprintf(szFile, "data\\outCD_%s.csv", szMarket);
		f.open(szFile, std::ios_base::out);
		f << "szMarket,szWindCode,szCode,szENName,szCNName,nType" << std::endl;
		TDF_CODE* pCodeTable; 
		unsigned int nItems;
		TDF_GetCodeTable(g_pHqTdfApi3Sdk->m_hTDF, szMarket, &pCodeTable, &nItems);
		std::for_each(pCodeTable, pCodeTable+nItems, [&](TDF_CODE& code){
			f << code.szMarket << ",";
			f << code.szWindCode << ",";
			f << code.szCode << ",";
			f << code.szENName << ",";
			f << code.szCNName << ",";
			f << code.nType << std::endl;
		});
		f.close();
		TDF_FreeArr(pCodeTable);
	}; 
	
	auto cmds = split(szCmd, ",");
	while(cmds.size() > 0 && cmds[0] != "q")
	{
		if (cmds.size() > 1)
		{
			if (cmds[0] == "d")
			{
				for (int i = 1; i < cmds.size(); ++i)
				{
					TDF_SetSubscription(g_pHqTdfApi3Sdk->m_hTDF, cmds[i].c_str(), SUBSCRIPTION_DEL);
				}
			}
			else if (cmds[0] == "a")
			{
				for (int i = 1; i < cmds.size(); ++i)
				{
					TDF_SetSubscription(g_pHqTdfApi3Sdk->m_hTDF, cmds[i].c_str(), SUBSCRIPTION_ADD);
				}
			}
			else if (cmds[0] == "s")
			{
				TDF_SetSubscription(g_pHqTdfApi3Sdk->m_hTDF, tostr(cmds).c_str(), SUBSCRIPTION_SET);
			}
			else if (cmds[0] == "c")
			{
				TDF_SetSubscription(g_pHqTdfApi3Sdk->m_hTDF, "", SUBSCRIPTION_FULL);
			}
			else if (cmds[0] == "o")
			{
				for (int i = 1; i < cmds.size(); ++i)
					outCodeTable(cmds[i].c_str());
			}
			else if (cmds[0] == "g")
			{
				//根据万得代码获取详细的期权代码信息
				//getCodeInfo(cmds[1].c_str());
			}
		}
		else if(cmds.size() > 0 && cmds[0] == "c")
			TDF_SetSubscription(g_pHqTdfApi3Sdk->m_hTDF, "", SUBSCRIPTION_FULL);

		Print("请输入q结束程序!\n");
		std::cin >> szCmd;
		cmds = split(szCmd, ",");
	}
#endif

	
	//if (m_hTDF)
	//{
	//	m_nQuitSubThread = 1;
	//	SetEvent(m_hEventDataArrived);
	//	WaitForSingleObject(m_hSubWriteThread, INFINITE);
	//	CloseHandle(m_hSubWriteThread);
	//	m_hSubWriteThread = NULL;
	//	TDF_Close(m_hTDF);

	//	m_hTDF = NULL;
	//	CloseHandle(m_hEventDataArrived);
	//	m_hEventDataArrived = NULL;
	//}
	//if (m_pMarketBlock)
	//{
	//	//DumpMarketInfo((DataBlockHead*)m_pMarketBlock);
	//	delete[] (char*)m_pMarketBlock;
	//	m_pMarketBlock = NULL;
	//}
	//m_fsMarket.close();
	//Print("Close complete!\n");
	//g_fsLog.close();
	
	return 0;
}