#ifndef HqTdfApi3_H
#define HqTdfApi3_H

#include <winsock2.h>
#include <windows.h>
#include <assert.h>
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <fstream>
#include <map>
#include <queue>
#include <cctype>
#include <assert.h>
#include <list>
#include <assert.h>
#include <time.h>
//
#include "../redis-cplusplus-client/redisOpt.h"
#include "common.h"
//
using namespace std;
using namespace com::autotrade::common;
using namespace com::autotrade::util;
//
class ConfigSettings;



namespace com { namespace autotrade { namespace hqtdfapi3 {

	struct HQ_TDFAPI_ERROR_CODE {
		enum type {
			TDFAPI_OK = 0,
			TDFAPI_LOAD_SETTING_CONFIG_FILE_ERROR = 1,
			TDFAPI_OPEN_RUN_LOGFILE_ERROR = 2,
			TDFAPI_OPEN_MARKET_LOGFILE_ERROR = 3,
			TDFAPI_INIT_CONN_SERVER_ERROR = 4,
			REDIS_SERVER_INIT_ERROR= 5,
		};
	};

	void g_RecvData(THANDLE hTdf, TDF_MSG* pMsgHead);
	void g_RecvSys(THANDLE hTdf, TDF_MSG* pSysMsg);
	BOOL WINAPI g_ConsoleHandler(DWORD msgType);
	DWORD __stdcall g_SubWriteThread(void* lParam);

class HqTdfApi3{
public:
	HqTdfApi3(){
		m_hTDF = NULL;
		m_hSubWriteThread = NULL;
		m_pMarketBlock = NULL;
		m_hEventDataArrived = NULL;
		m_nMaxBuf = 0;
		m_nMaxWriteTimeGap = 0;
		m_nQuitSubThread = 0;
		m_nOutputDevice = 0;
		m_nOutputCodeTable = 0;
		m_nCommonLogGap = 0;
		m_isInitOK = false;
		m_nPeriod = 10;
	}
	bool isInit(){
		return m_isInitOK;
	}
	int init();
	void RecvData(THANDLE hTdf, TDF_MSG* pMsgHead);
	void RecvSys(THANDLE hTdf, TDF_MSG* pSysMsg);
	void PushToListAndNotify(char* pBuf, std::list<char*>& listQueue, AutoLock& guard, HANDLE hEvent);
	bool OpenStream(std::ofstream& of, const std::string& strPath, const std::string& strLog);
	bool InitDataFiles(const std::string& strDataDir);
	void DumpScreenMarket(TDF_MARKET_DATA* pMarket, int nItems);
	void DumpMarketInfo(DataBlockHead* pHead);
	void DumpMarketInfoForPandas(DataBlockHead* pHead);
	const char* GetErrStr(TDF_ERR nErr);
	DWORD __stdcall SubWriteThread(void* lParam);
	BOOL WINAPI ConsoleHandler(DWORD msgType);
	//
	template<class InfoType, class DataType, TDF_MSG_ID nMsgId>
	void DoRecv(THANDLE hTdf, unsigned int nItemCount, unsigned int nItemSize, __time64_t* nLastUpdateTime, char** ppDataBlock, void* pDataSource, unsigned int nServerTime);
	//
	bool checkFileAndCloseOldFile();
	//
	THANDLE m_hTDF;
	THANDLE m_hSubWriteThread;
	HANDLE m_hEventDataArrived;
	std::string m_strDataPath;
	std::string m_strCurMarketFileName;
	//std::set<string&> m_setWindStockCode;
	RedisOpt m_objRedisOpt;
private:
	std::ofstream m_fsMarket;
	std::ofstream m_fsRedisErrLog;
	AutoLock m_lockDataBlockList;
	AutoLock m_lockForLogFile;
	std::list<char*> m_listDataBlock;
	char* m_pMarketBlock;
	unsigned int m_nMaxBuf;
	unsigned int m_nMaxWriteTimeGap;
	int m_nQuitSubThread;
	int m_nOutputDevice;
	int m_nOutputCodeTable;
	int m_nCommonLogGap;
	bool m_isInitOK;
	int m_nPeriod;
};


}}} // namespace

#endif






