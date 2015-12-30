// hqtdfapi3.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include "hqtdfapi3.h"
#include "ConfigParser.h"
#include "ConfigSettings.h"
#include "PathHelper.h"


//
using namespace std;

std::ofstream g_fsLog;
com::autotrade::hqtdfapi3::HqTdfApi3* g_pHqTdfApi3Sdk;

static int TEN_SPAN[] = {0,10,20,30,40,50,60};
static int FIFTEEN_SPAN[] = {0,15,30,45,60};
static int TWENTY_SAPN[] = {0,20,40,60};
static int THIRTY_SPAN[] = {0,30,60};
static int HOUR_SPAN[] = {0,60};

namespace com { namespace autotrade { namespace hqtdfapi3 {

void g_RecvData(THANDLE hTdf, TDF_MSG* pMsgHead){
	if(g_pHqTdfApi3Sdk && g_pHqTdfApi3Sdk->isInit()){
		g_pHqTdfApi3Sdk->RecvData(hTdf,pMsgHead);
	}else{
		Print("g_pHqTdfApi3Sdk not init ! RecvData() call fail.\n");
	}
	return;
}

void g_RecvSys(THANDLE hTdf, TDF_MSG* pSysMsg){
	if(g_pHqTdfApi3Sdk && g_pHqTdfApi3Sdk->isInit()){
		g_pHqTdfApi3Sdk->RecvSys(hTdf,pSysMsg);
	}else{
		Print("g_pHqTdfApi3Sdk not init ! g_RecvSys() call fail.\n");
	}
	return ;
}

BOOL WINAPI g_ConsoleHandler(DWORD msgType){
	if(g_pHqTdfApi3Sdk && g_pHqTdfApi3Sdk->isInit()){
		return g_pHqTdfApi3Sdk->ConsoleHandler(msgType);
	}else{
		Print("g_pHqTdfApi3Sdk not init ! g_ConsoleHandler() call fail.\n");
	}
	return TRUE;
}
DWORD __stdcall g_SubWriteThread(void* lParam){
	if(g_pHqTdfApi3Sdk && g_pHqTdfApi3Sdk->isInit()){
		return g_pHqTdfApi3Sdk->SubWriteThread(lParam);
	}
	return 0;
}

int HqTdfApi3::init(){
	//
	ConfigSettings cfgSettings;
	//cfg path
	std::string strConfigFile = GetConfigDir() + GetBasicFileName() + ".ini";
	bool bLoadSettings = cfgSettings.LoadSettings(strConfigFile);
	if(false == bLoadSettings){
		Print("��������ʧ�ܣ����������ļ�:%s���ڣ���ip,port,user,password�Ѿ����ã�\n��Enter��������\n", strConfigFile.c_str());
		return HQ_TDFAPI_ERROR_CODE::TDFAPI_LOAD_SETTING_CONFIG_FILE_ERROR;
	}
	//log path
	std::string strLogPath(cfgSettings.szDataDir, 0, sizeof(cfgSettings.szDataDir)-1);
	m_strDataPath = strLogPath;
	//log name
	strLogPath += "log.txt";
	g_fsLog.open(strLogPath.c_str(), std::ios_base::out);
	if (!g_fsLog.is_open())
	{
		Print("����־�ļ�: %s ʧ�ܣ�һ��������ͬʱ���ж�ݣ�������ļ���,�� DataDir ����ָ��ͬĿ¼��\n��Enter��������\n", strLogPath.c_str());
		return HQ_TDFAPI_ERROR_CODE::TDFAPI_OPEN_RUN_LOGFILE_ERROR;
	}
	TDF_SetEnv(TDF_ENVIRON_HEART_BEAT_INTERVAL, cfgSettings.nHeartBeatGap);
	TDF_SetEnv(TDF_ENVIRON_MISSED_BEART_COUNT, cfgSettings.nMissedHeartBeatCount);
	TDF_SetEnv(TDF_ENVIRON_OPEN_TIME_OUT, cfgSettings.nOpenTimeOut);
	//TDF_SetEnv(TDF_ENVIRON_USE_PACK_OVER,1);
	assert(g_fsLog.is_open());
	if (false == InitDataFiles(cfgSettings.szDataDir))
	{
		Print("��ʼ�� market �ļ�ʧ�� ! �� Enter ���˳�����!\n");
		return HQ_TDFAPI_ERROR_CODE::TDFAPI_OPEN_MARKET_LOGFILE_ERROR;
	}
	//
	TDF_OPEN_SETTING_EXT open_settings_ext = {0};
	int nValidServerCount = cfgSettings.ServerCount();
	open_settings_ext.nServerNum = nValidServerCount;
	int i=0;
	for (i = 0; i < sizeof(open_settings_ext.siServer)/sizeof(TDF_SERVER_INFO); ++i){
		strncpy(open_settings_ext.siServer[i].szIp, cfgSettings.si[i].szIP, sizeof(open_settings_ext.siServer[i].szIp)-1);
		_snprintf(open_settings_ext.siServer[i].szPort, sizeof(open_settings_ext.siServer[i].szPort)-1, "%d", cfgSettings.si[i].nPort);
		strncpy(open_settings_ext.siServer[i].szUser, cfgSettings.si[i].szUser, sizeof(open_settings_ext.siServer[i].szUser)-1);
		strncpy(open_settings_ext.siServer[i].szPwd, cfgSettings.si[i].szPwd, sizeof(open_settings_ext.siServer[i].szPwd)-1);
	}
	//
	open_settings_ext.pfnMsgHandler = g_RecvData;
	open_settings_ext.pfnSysMsgNotify = g_RecvSys;
	//
	open_settings_ext.szMarkets = DeepCopy(cfgSettings.szMarketList);
	open_settings_ext.szSubScriptions = DeepCopy(cfgSettings.szSubscriptions);
	//
	open_settings_ext.nTime = cfgSettings.nTime;
	open_settings_ext.nTypeFlags = cfgSettings.nDataType;
	//settings.nProtocol = 0;
	//settings.nDate = configObj.nDate;
	m_nMaxBuf = cfgSettings.nMaxMemBuf;
	m_nMaxWriteTimeGap = cfgSettings.nMaxWriteTimeGap;
	m_nOutputDevice = cfgSettings.nOutputDevice;
	m_nCommonLogGap = cfgSettings.nCommonLogGap;
	m_nOutputCodeTable = cfgSettings.bOutputCodeTable;
	//
	bool bConnNetOK = false;
	for (i = 0; i < 5; i++){
		TDF_ERR nErr = TDF_ERR_SUCCESS;
		m_hTDF = TDF_OpenExt(&open_settings_ext, &nErr);
		Print("TDF_Open returned:%s\n", GetErrStr(nErr));
		if(nErr == TDF_ERR_NETWORK_ERROR){
			Print("TDF_Open ����ʧ�ܣ�3��֮������\n");
			Sleep(3*1000);
		}else{
			bConnNetOK = true;
			break;
		}
	}
	if(false == bConnNetOK){
		Print("TDF_Open ���� 5��ʧ�ܣ��˳�\n");
		return HQ_TDFAPI_ERROR_CODE::TDFAPI_INIT_CONN_SERVER_ERROR;
	}
	//init redis database
	if(REDIS_OPT_ERROR_CODE::REDIS_OPT_OK != m_objRedisOpt.init()){
		Print("Redis init fail���˳�\n");
		return HQ_TDFAPI_ERROR_CODE::REDIS_SERVER_INIT_ERROR;
	}
	//
	m_isInitOK = true;
	return HQ_TDFAPI_ERROR_CODE::TDFAPI_OK;
}

BOOL WINAPI HqTdfApi3::ConsoleHandler(DWORD msgType){
    if (msgType == CTRL_C_EVENT || msgType == CTRL_CLOSE_EVENT){
        if (m_hTDF){
            m_nQuitSubThread = 1;
            SetEvent(m_hEventDataArrived);
            if (m_hSubWriteThread){
                WaitForSingleObject(m_hSubWriteThread, INFINITE);
                CloseHandle(m_hSubWriteThread);
                m_hSubWriteThread = NULL;
            }
            TDF_Close(m_hTDF);
            m_hTDF = NULL;
            if (m_hEventDataArrived){
                CloseHandle(m_hEventDataArrived);
                m_hEventDataArrived = NULL;
            }
        }
		if (m_pMarketBlock){
			//DumpMarketInfo((DataBlockHead*)m_pMarketBlock);
			delete[] (char*)m_pMarketBlock;
			m_pMarketBlock = NULL;
		}
        m_fsMarket.close();
        g_fsLog.close();
        Print("Close complete!\n");
        exit(0);
        return TRUE;
    }
    return TRUE;
}

DWORD __stdcall HqTdfApi3::SubWriteThread(void* lParam){
	while (!m_nQuitSubThread){
		WaitForSingleObject(m_hEventDataArrived, INFINITE);
		m_lockDataBlockList.Lock();
		std::list<char*> listTemp(m_listDataBlock.begin(), m_listDataBlock.end());
		if (m_listDataBlock.size()){
			m_listDataBlock.clear();
		}
		m_lockDataBlockList.UnLock();
		for (std::list<char*>::iterator it=listTemp.begin(); it!=listTemp.end(); it++){
			DataBlockHead* pHead = (DataBlockHead*)*it;
			assert(pHead);
			switch(pHead->nMsgId){
			case MSG_DATA_MARKET:
				DumpMarketInfoForPandas(pHead);
				break;
			default:
				assert(0);
				break;
			}
			delete[] (char*)pHead;
			//Print("-----delete data block:0x%x------\n", pHead);
		}
	}
	if (m_pMarketBlock){
		DumpMarketInfoForPandas((DataBlockHead*)m_pMarketBlock);
	}
	//
	SetEvent(m_hSubWriteThread);
	return 0;
}

void HqTdfApi3::RecvData(THANDLE hTdf, TDF_MSG* pMsgHead){
	if (!pMsgHead->pData){
		assert(0);
		return ;
	}
	unsigned int nItemCount = pMsgHead->pAppHead->nItemCount;
	unsigned int nItemSize = pMsgHead->pAppHead->nItemSize;
	if (!nItemCount){
		assert(0);
		return ;
	}
	static __time64_t nLastLogTick = 0;
	static int put_count=0;
	switch(pMsgHead->nDataType){
		case MSG_DATA_MARKET:
			{
				assert(nItemSize == sizeof(TDF_MARKET_DATA));
				static __time64_t nLastUpdateTime = 0;
				if (m_nOutputDevice & OUTPUTTYPE_FILE){
					//DoRecv<MarketInfo, TDF_MARKET_DATA, MSG_DATA_MARKET>(hTdf, nItemCount, nItemSize, &nLastUpdateTime, &m_pMarketBlock, pMsgHead->pData, pMsgHead->nServerTime);
					//put_count++;
				}
				if(m_nOutputDevice & OUTPUTTYPE_SCREN ){
					//DumpScreenMarket((TDF_MARKET_DATA*)pMsgHead->pData, nItemCount);
				}else{
					//Print("now %d in DoRecv() \n",put_count);
					//
					//Print("now nItemCount = %d in RecvData() \n",nItemCount);
					for(int i = 0;i<nItemCount;i++){
						TDF_MARKET_DATA* pLastMarket = GETRECORD<TDF_MARKET_DATA>(pMsgHead->pData, i);
						//DISPLAY_TIME_GAP(m_nCommonLogGap, nLastLogTick, "���յ������¼:���룺%s, ҵ������:%d, ʱ��:%d, ���¼�:%d���ɽ�����:%I64d \n", pLastMarket->szWindCode, pLastMarket->nActionDay, pLastMarket->nTime, pLastMarket->nMatch, pLastMarket->iVolume);
						//
						//set code price to redis.
						int redis_ret = m_objRedisOpt.set_day_raw_data(pLastMarket->nTradingDay,pLastMarket->nTime,pLastMarket->szCode,pLastMarket->nMatch);
						if(REDIS_OPT_ERROR_CODE::REDIS_OPT_OK != redis_ret){
							m_fsRedisErrLog << "Redis error: " << redis_ret << endl;
						}
					}
					//
				}
			}
			break;
		case MSG_DATA_FUTURE:
			//Print("RecvData MSG_DATA_FUTURE\n");			
			break;
		case MSG_DATA_INDEX:
			//Print("RecvData MSG_DATA_INDEX\n");
			break;
		case MSG_DATA_TRANSACTION:
			//Print("RecvData MSG_DATA_TRANSACTION\n");
			break;
		case MSG_DATA_ORDERQUEUE:
			//Print("RecvData MSG_DATA_ORDERQUEUE\n");
			break;
		case MSG_DATA_ORDER:
			//Print("RecvData MSG_DATA_ORDER\n");
			break;
		case MSG_DATA_BBQTRANSACTION:
			//Print("RecvData MSG_DATA_BBQTRANSACTION\n");
			break;
		case MSG_DATA_BBQBID:
			//Print("RecvData MSG_DATA_BBQBID\n");
			break;
		default:
			assert(0);
			break;
	}
}

void HqTdfApi3::RecvSys(THANDLE hTdf, TDF_MSG* pSysMsg){
	if (!pSysMsg ||! hTdf){
		return;
	}
	//
	switch (pSysMsg->nDataType){
		case MSG_SYS_PACK_OVER:
			{
				TDF_PACK_OVER* pPackOver = (TDF_PACK_OVER*)pSysMsg->pData;
				Print("TDF_PACK_OVER:num:%d,conID:%d\n",pPackOver->nDataNumber,pPackOver->nConID);
			}
			break;
		case MSG_SYS_HEART_BEAT:
			Print("�յ�������Ϣ\n");
			break;
		case MSG_SYS_DISCONNECT_NETWORK:
			Print("����Ͽ�\n");
			break;
		case MSG_SYS_QUOTATIONDATE_CHANGE:
			{
				TDF_QUOTATIONDATE_CHANGE* pChange = (TDF_QUOTATIONDATE_CHANGE*)pSysMsg->pData;
				if (pChange){
					Print("�յ��������ڱ��֪ͨ�������Զ���������������%s, ԭ����:%d, �����ڣ�%d\n", pChange->szMarket, pChange->nOldDate, pChange->nNewDate);
				}
			}
			break;
		case MSG_SYS_MARKET_CLOSE:
			{
				TDF_MARKET_CLOSE* pCloseInfo = (TDF_MARKET_CLOSE*)pSysMsg->pData;
				if (pCloseInfo){
					Print("������Ϣ:market:%s, time:%d, info:%s\n", pCloseInfo->szMarket, pCloseInfo->nTime, pCloseInfo->chInfo);
				}
			}
			break;
		case MSG_SYS_CONNECT_RESULT:
			{
				TDF_CONNECT_RESULT* pConnResult = (TDF_CONNECT_RESULT*)pSysMsg->pData;
				if (pConnResult && pConnResult->nConnResult){
					Print("���� %s:%s user:%s, password:%s �ɹ�!\n", pConnResult->szIp, pConnResult->szPort, pConnResult->szUser, pConnResult->szPwd);
				}else{
					Print("���� %s:%s user:%s, password:%s ʧ��!\n", pConnResult->szIp, pConnResult->szPort, pConnResult->szUser, pConnResult->szPwd);
				}
			}
			break;
		case MSG_SYS_LOGIN_RESULT:
			{
				TDF_LOGIN_RESULT* pLoginResult = (TDF_LOGIN_RESULT*)pSysMsg->pData;
				if (pLoginResult && pLoginResult->nLoginResult){
					Print("��½�ɹ���info:%s, nMarkets:%d\n", pLoginResult->szInfo, pLoginResult->nMarkets);
					for (int i=0; i<pLoginResult->nMarkets; i++){
						Print("market:%s, dyn_date:%d\n", pLoginResult->szMarket[i], pLoginResult->nDynDate[i]);
					}
				}else{
					Print("��½ʧ�ܣ�ԭ��:%s\n", pLoginResult->szInfo);
				}
			}
			break;
		case MSG_SYS_CODETABLE_RESULT:
			{
				TDF_CODE_RESULT* pCodeResult = (TDF_CODE_RESULT*)pSysMsg->pData;
				if (pCodeResult ){
					Print("���յ������info:%s, �г�����:%d\n", pCodeResult->szInfo, pCodeResult->nMarkets);
					for (int i=0; i<pCodeResult->nMarkets; i++){
						Print("�г�:%s, ���������:%d, ���������:%d\n", pCodeResult->szMarket[i], pCodeResult->nCodeCount[i], pCodeResult->nCodeDate[i]);
						if (m_nOutputCodeTable){
							//��ȡ����� 
							TDF_CODE* pCodeTable; 
							unsigned int nItems;
							TDF_GetCodeTable(hTdf, pCodeResult->szMarket[i], &pCodeTable, &nItems);
							for (int i=0; i<nItems; i++){
								TDF_CODE& code = pCodeTable[i];
								Print("windcode:%s, code:%s, market:%s, name:%s, nType:0x%x\n",code.szWindCode, code.szCode, code.szMarket, code.szCNName, code.nType);
							}
							TDF_FreeArr(pCodeTable);
						}
						if(true){
							auto outCodeTable = [&](const char* szMarket){
								std::fstream f;
								std::string codeFile = m_strDataPath + "outCode_" + szMarket + ".csv";

								f.open(codeFile, std::ios_base::out);
								f << "szMarket,szWindCode,szCode,szENName,szCNName,nType" << std::endl;
								TDF_CODE* pCodeTable; 
								unsigned int nItems;
								TDF_GetCodeTable(g_pHqTdfApi3Sdk->m_hTDF, szMarket, &pCodeTable, &nItems);
								std::for_each(pCodeTable, pCodeTable+nItems, [&](TDF_CODE& code){
									if(16 == code.nType){
										f << code.szMarket << ",";
										f << code.szWindCode << ",";
										f << code.szCode << ",";
										f << code.szENName << ",";
										f << code.szCNName << ",";
										f << code.nType << std::endl;
										//
										//g_pHqTdfApi3Sdk->m_setWindStockCode.insert(code.szWindCode);
										//int nErr = TDF_SetSubscription(g_pHqTdfApi3Sdk->m_hTDF, code.szWindCode, SUBSCRIPTION_ADD);
										//if(nErr != TDF_ERR_SUCCESS){
										//	Print("TDF_Open %s returned:%s\n", code.szWindCode,g_pHqTdfApi3Sdk->GetErrStr((TDF_ERR)nErr));
										//}else{
										//	Print("TDF_Open %s returned:%s\n", code.szWindCode,g_pHqTdfApi3Sdk->GetErrStr((TDF_ERR)nErr));
										//}
									}
								});
								f.close();
								TDF_FreeArr(pCodeTable);
							}; 
							outCodeTable( pCodeResult->szMarket[i]);
						}
					}
				}
			}
			break;
		default:
			assert(0);
			break;
		}
}

void HqTdfApi3::PushToListAndNotify(char* pBuf, std::list<char*>& listQueue, AutoLock& guard, HANDLE hEvent){
    guard.Lock();
    listQueue.push_back(pBuf);
    guard.UnLock();
    SetEvent(hEvent);
}

bool HqTdfApi3::OpenStream(std::ofstream& of, const std::string& strPath, const std::string& strLog){
    of.open(strPath.c_str(), std::ios_base::app);
    if (!of.is_open()){
        Print("%s: %s ʧ�ܣ�\n", strLog.c_str(), strPath.c_str());
        return false;
    }
    return true;
}

//��鵱ǰʱ���Ƿ񵽴���Ҫ�л���ʱ�����
//��� return true
//���� return false
bool HqTdfApi3::checkFileAndCloseOldFile()
{
	std::string cur_time = boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time());  
	// ��ʱ��strTime����ʱ��ĸ�ʽ��YYYYMMDDTHHMMSS�����ں�ʱ���ô�д��ĸT������  
	int pos = cur_time.find('T');  
	cur_time.replace(pos,1,std::string(""));  
	//
	int cur_min = atoi(cur_time.substr(10,2).c_str());
	int cur_sec = atoi(cur_time.substr(12,2).c_str());

	//���㵱ǰ��ʱ��Ƭ
	int *cur_span = NULL;
	int cur_span_size = 0;
	switch(m_nPeriod)
	{
	case 10:
		cur_span = TEN_SPAN;
		cur_span_size = sizeof(TEN_SPAN)/sizeof(TEN_SPAN[0]);
		break;
	case 15:
		cur_span = FIFTEEN_SPAN;
		cur_span_size = sizeof(FIFTEEN_SPAN)/sizeof(FIFTEEN_SPAN[0]);
		break;
	case 20:
		cur_span = TWENTY_SAPN;
		cur_span_size = sizeof(TWENTY_SAPN)/sizeof(TWENTY_SAPN[0]);
		break;
	case 30:
		cur_span = THIRTY_SPAN;
		cur_span_size = sizeof(THIRTY_SPAN)/sizeof(THIRTY_SPAN[0]);
		break;
	case 60:
		cur_span = HOUR_SPAN;
		cur_span_size = sizeof(HOUR_SPAN)/sizeof(HOUR_SPAN[0]);
		break;
	default:
		cur_span = NULL;
	}

	int cur_span_beg = 0;
	int cur_span_end = 0;

	pos = 0;
	for(pos=0;pos<cur_span_size;pos++)
	{
		if(cur_min >= cur_span[pos] && cur_min < cur_span[pos+1])
		{
			cur_span_beg = cur_span[pos];
			cur_span_end = cur_span[pos+1];
			break;
		}
	}
	//��ǰ�ļ���
	string cur_filename = cur_time.substr(0,8) + "-" + cur_time.substr(8,2)+ (cur_span_beg == 0 ? "00" : boost::lexical_cast<string>(cur_span_beg)) + ".csv";
	std::string strMarketPath = m_strDataPath + "/market/";
	cur_filename = strMarketPath + cur_filename;
	if(m_strCurMarketFileName.length() <= 0 || (cur_filename != m_strCurMarketFileName) ){
		if(m_strCurMarketFileName.length() > 0){
			m_fsMarket.close();
		}
		m_strCurMarketFileName = cur_filename;
		if (!OpenStream(m_fsMarket, m_strCurMarketFileName, "�����������ļ�ʧ�ܣ�")){
			return false;
		}
	}
	return true;
}

bool HqTdfApi3::InitDataFiles(const std::string& strDataDir){
	std::string strRedisErrLogPath = strDataDir + "rediserr.log";
	if (!OpenStream(m_fsRedisErrLog, strRedisErrLogPath, "�� REDIS ERRLOG �ļ�ʧ�ܣ�")){
		return false;
	}
	m_fsRedisErrLog << "Redis :" << m_objRedisOpt.m_strHost << ":" << m_objRedisOpt.m_nPort << ", dbindex:" << m_objRedisOpt.m_nDbindex << std::endl;
    return true;
}

void HqTdfApi3::DumpMarketInfo(DataBlockHead* pHead){
	MarketInfo* pInfo = (MarketInfo*)((char*)pHead+sizeof(DataBlockHead));
	char* pEnd = (char*)pHead + pHead->m_nOffset;
	//
	const int nMaxBufSize = 4096;
	char szBuf[nMaxBufSize];
	char szBufSmaller1[160];
	char szBufSmaller2[160];
	char szBufSmaller3[160];
	char szBufSmaller4[160];
	char szBufSmaller5[64];
	//
#if 0
	int nOffset = 0;

	char        szWindCode[32];         //600001.SH 
	char        szCode[32];             //ԭʼCode
	int         nActionDay;             //ҵ������(��Ȼ��)
	int         nTradingDay;            //������
	int			 nTime;					//ʱ��(HHMMSSmmm)
	int			 nStatus;				//״̬
	unsigned int nPreClose;				//ǰ���̼�
	unsigned int nOpen;					//���̼�
	unsigned int nHigh;					//��߼�
	unsigned int nLow;					//��ͼ�
	unsigned int nMatch;				//���¼�
	unsigned int nAskPrice[10];			//������
	unsigned int nAskVol[10];			//������
	unsigned int nBidPrice[10];			//�����
	unsigned int nBidVol[10];			//������
	unsigned int nNumTrades;			//�ɽ�����
	__int64		 iVolume;				//�ɽ�����
	__int64		 iTurnover;				//�ɽ��ܽ��
	__int64		 nTotalBidVol;			//ί����������
	__int64		 nTotalAskVol;			//ί����������
	unsigned int nWeightedAvgBidPrice;	//��Ȩƽ��ί��۸�
	unsigned int nWeightedAvgAskPrice;  //��Ȩƽ��ί���۸�
	int			 nIOPV;					//IOPV��ֵ��ֵ
	int			 nYieldToMaturity;		//����������
	unsigned int nHighLimited;			//��ͣ��
	unsigned int nLowLimited;			//��ͣ��
	char		 chPrefix[4];			//֤ȯ��Ϣǰ׺
	int			 nSyl1;					//��ӯ��1
	int			 nSyl2;					//��ӯ��2
	int			 nSD2;					//����2���Ա���һ�ʣ�
	"����, ����ʱ��, ������ʱ��, ������ʱ��, ��ô���, ԭʼ����, ҵ������(��Ȼ��), ������,״̬,ǰ��,���̼�,��߼�,��ͼ�,���¼�,������,������,�����,������,�ɽ�����,�ɽ�����,�ɽ��ܽ��,ί����������,ί����������,��Ȩƽ��ί��۸�,��Ȩƽ��ί���۸�,IOPV��ֵ��ֵ,����������,��ͣ��,��ͣ��,֤ȯ��Ϣǰ׺,��ӯ��1,��ӯ��2,����2���Ա���һ�ʣ�",
#endif
		//std::ofstream dataLog;
		//dataLog.open("e:\\market.csv", std::ios_base::app);
		while ((char*)pInfo < pEnd){
			for (int i=0; i<pInfo->nItems; i++){
				TDF_MARKET_DATA* pTDFMarket = pInfo->rawData + i;
				_snprintf(szBuf, nMaxBufSize, "%d,%d,%d,%d,%s,%s,%d,%d,"
					"%d(%c),"         //status
					"%d,%d,%d,%d,%d," //preclose,open, high, low, last,
					"%s,%s,%s,%s,"
					"%d,%I64d,%I64d,%I64d,%I64d," 
					"%u,%u,%d,%d,"
					"%u,%u,%s,"
					"%d,%d,%d"
					,
					MAKEYMD(pInfo->st.wYear, pInfo->st.wMonth, pInfo->st.wDay), 
					MAKEHHMMSSsss(pInfo->st.wHour, pInfo->st.wMinute, pInfo->st.wSecond, pInfo->st.wMilliseconds), 
					pInfo->nServerTime, 
					pInfo->rawData[i].nTime, 
					pTDFMarket->szWindCode, pTDFMarket->szCode, pTDFMarket->nActionDay, pTDFMarket->nTradingDay, 
					pTDFMarket->nStatus, SAFE_CHAR(pTDFMarket->nStatus),
					pTDFMarket->nPreClose, pTDFMarket->nOpen, pTDFMarket->nHigh ,pTDFMarket->nLow, pTDFMarket->nMatch,
					intarr2str(szBufSmaller1, sizeof(szBufSmaller1), (int*)pTDFMarket->nAskPrice, ELEM_COUNT(pTDFMarket->nAskPrice)),
					intarr2str(szBufSmaller2, sizeof(szBufSmaller2), (int*)pTDFMarket->nAskVol, ELEM_COUNT(pTDFMarket->nAskVol)),
					intarr2str(szBufSmaller3, sizeof(szBufSmaller3), (int*)pTDFMarket->nBidPrice, ELEM_COUNT(pTDFMarket->nBidPrice)),
					intarr2str(szBufSmaller4, sizeof(szBufSmaller4), (int*)pTDFMarket->nBidVol, ELEM_COUNT(pTDFMarket->nBidVol)),
					pTDFMarket->nNumTrades, pTDFMarket->iVolume, pTDFMarket->iTurnover, pTDFMarket->nTotalBidVol, pTDFMarket->nTotalAskVol, 
					pTDFMarket->nWeightedAvgBidPrice, pTDFMarket->nWeightedAvgAskPrice,pTDFMarket->nIOPV, pTDFMarket->nYieldToMaturity, 
					pTDFMarket->nHighLimited, pTDFMarket->nLowLimited, chararr2str(szBufSmaller5, sizeof(szBufSmaller5), pTDFMarket->chPrefix, ELEM_COUNT(pTDFMarket->chPrefix)),
					pTDFMarket->nSyl1, pTDFMarket->nSyl2, pTDFMarket->nSD2);
				m_fsMarket<<szBuf<<std::endl;
				//dataLog << szBuf << endl;
			}
			pInfo =(MarketInfo*)((char*)pInfo + GetInfoSize<MarketInfo, TDF_MARKET_DATA>(pInfo->nItems));
		}
}

void HqTdfApi3::DumpMarketInfoForPandas(DataBlockHead* pHead){
	MarketInfo* pInfo = (MarketInfo*)((char*)pHead+sizeof(DataBlockHead));
	char* pEnd = (char*)pHead + pHead->m_nOffset;
	//
	const int nMaxBufSize = 4096;
	char szBuf[nMaxBufSize];
	//
	checkFileAndCloseOldFile();
	//
	while ((char*)pInfo < pEnd){
		for (int i=0; i<pInfo->nItems; i++){
			TDF_MARKET_DATA* pTDFMarket = pInfo->rawData + i;
				
			string str_day = boost::lexical_cast<string>(pTDFMarket->nTradingDay);
			string str_time = boost::lexical_cast<string>(pTDFMarket->nTime);
			string str_code = boost::lexical_cast<string>(pTDFMarket->szCode);
			string str_price = boost::lexical_cast<string>(pTDFMarket->nMatch);
				
			string str_timestamp = str_day.substr(0,4) + "-" +  str_day.substr(4,2) + "-" + str_day.substr(6,2) + " " + str_time.substr(0,2) + ":" + str_time.substr(2,2) + ":" + str_time.substr(4,2);
			//
			if(str_code.length() == 6 && (str_code.c_str()[0] == '6' || str_code.c_str()[0] == '0' || str_code.c_str()[0] == '3' )){
				_snprintf(szBuf, nMaxBufSize, "%s,%s,%s",str_timestamp.c_str(),str_code.c_str(),str_price.c_str());
				m_fsMarket<<szBuf<<std::endl;
			}
		}
		pInfo =(MarketInfo*)((char*)pInfo + GetInfoSize<MarketInfo, TDF_MARKET_DATA>(pInfo->nItems));
	}
}
void HqTdfApi3::DumpScreenMarket(TDF_MARKET_DATA* pMarket, int nItems){
	Print("-------- Market, Count:%d --------\n", nItems);
	//
	char szBuf1[512];
	char szBuf2[512];
	char szBuf3[512];
	char szBuf4[512];
	char szBufSmall[64];
	//
	for (int i=0; i<nItems; i++){
		const TDF_MARKET_DATA& marketData = pMarket[i];
		Print("��ô��� szWindCode: %s\n", marketData.szWindCode);
		Print("ԭʼ���� szCode: %s\n", marketData.szCode);
		Print("ҵ������(��Ȼ��) nActionDay: %d\n", marketData.nActionDay);
		Print("������ nTradingDay: %d\n", marketData.nTradingDay);
		Print("ʱ��(HHMMSSmmm) nTime: %d\n", marketData.nTime);
		Print("״̬ nStatus: %d(%c)\n", marketData.nStatus, SAFE_CHAR(marketData.nStatus));
		Print("ǰ���̼� nPreClose: %d\n", marketData.nPreClose);
		Print("���̼� nOpen: %d\n", marketData.nOpen);
		Print("��߼� nHigh: %d\n", marketData.nHigh);
		Print("��ͼ� nLow: %d\n", marketData.nLow);
		Print("���¼� nMatch: %d\n", marketData.nMatch);
		Print("������ nAskPrice: %s \n", intarr2str(szBuf1, sizeof(szBuf1), (int*)marketData.nAskPrice, ELEM_COUNT(marketData.nAskPrice)));
		Print("������ nAskVol: %s \n", intarr2str(szBuf2, sizeof(szBuf2), (int*)marketData.nAskVol, ELEM_COUNT(marketData.nAskVol)));
		Print("����� nBidPrice: %s \n", intarr2str(szBuf3, sizeof(szBuf3), (int*)marketData.nBidPrice, ELEM_COUNT(marketData.nBidPrice)));
		Print("������ nBidVol: %s \n", intarr2str(szBuf4, sizeof(szBuf4), (int*)marketData.nBidVol, ELEM_COUNT(marketData.nBidVol)));
		Print("�ɽ����� nNumTrades: %d\n", marketData.nNumTrades);
		Print("�ɽ����� iVolume: %I64d\n", marketData.iVolume);
		Print("�ɽ��ܽ�� iTurnover: %I64d\n", marketData.iTurnover);
		Print("ί���������� nTotalBidVol: %I64d\n", marketData.nTotalBidVol);
		Print("ί���������� nTotalAskVol: %I64d\n", marketData.nTotalAskVol);
		Print("��Ȩƽ��ί��۸� nWeightedAvgBidPrice: %u\n", marketData.nWeightedAvgBidPrice);
		Print("��Ȩƽ��ί���۸� nWeightedAvgAskPrice: %u\n", marketData.nWeightedAvgAskPrice);
		Print("IOPV��ֵ��ֵ nIOPV: %d\n",  marketData.nIOPV);
		Print("���������� nYieldToMaturity: %d\n", marketData.nYieldToMaturity);
		Print("��ͣ�� nHighLimited: %d\n", marketData.nHighLimited);
		Print("��ͣ�� nLowLimited: %d\n", marketData.nLowLimited);
		Print("֤ȯ��Ϣǰ׺ chPrefix: %s\n", chararr2str(szBufSmall, sizeof(szBufSmall), (char*)marketData.chPrefix, ELEM_COUNT(marketData.chPrefix)));
		Print("��ӯ��1 nSyl1: %d\n", marketData.nSyl1);
		Print("��ӯ��2 nSyl2: %d\n", marketData.nSyl2);
		Print("����2���Ա���һ�ʣ� nSD2: %d\n", marketData.nSD2);
		//
		if(nItems > 1){
			Print("\n");
		}
	}
	Print("\n");
}

const char* HqTdfApi3::GetErrStr(TDF_ERR nErr){
	static std::map<TDF_ERR, const char*> mapErrStr;
	static bool init = false;
	if(false == init){
		mapErrStr.insert(std::make_pair(TDF_ERR_UNKOWN, "TDF_ERR_UNKOWN"));
		mapErrStr.insert(std::make_pair(TDF_ERR_INITIALIZE_FAILURE, "TDF_ERR_INITIALIZE_FAILURE"));
		mapErrStr.insert(std::make_pair(TDF_ERR_NETWORK_ERROR, "TDF_ERR_NETWORK_ERROR"));
		mapErrStr.insert(std::make_pair(TDF_ERR_INVALID_PARAMS, "TDF_ERR_INVALID_PARAMS"));
		mapErrStr.insert(std::make_pair(TDF_ERR_VERIFY_FAILURE, "TDF_ERR_VERIFY_FAILURE"));
		mapErrStr.insert(std::make_pair(TDF_ERR_NO_AUTHORIZED_MARKET, "TDF_ERR_NO_AUTHORIZED_MARKET"));
		mapErrStr.insert(std::make_pair(TDF_ERR_NO_CODE_TABLE, "TDF_ERR_NO_CODE_TABLE"));
		mapErrStr.insert(std::make_pair(TDF_ERR_SUCCESS, "TDF_ERR_SUCCESS"));
		init = true;
	}
	//
	if (mapErrStr.find(nErr) == mapErrStr.end()){
		return "TDF_ERR_UNKOWN";
	}else{
		return mapErrStr[nErr];
	}
}

template<class InfoType, class DataType, TDF_MSG_ID nMsgId>
void HqTdfApi3::DoRecv(THANDLE hTdf, unsigned int nItemCount, 
	unsigned int nItemSize, __time64_t* nLastUpdateTime, char** ppDataBlock, 
	void* pDataSource, unsigned int nServerTime)
{
	assert(ppDataBlock);
	assert(nItemSize == sizeof(DataType));
	DataBlockHead* pBlockHead = (DataBlockHead*)*ppDataBlock;
	unsigned int nAppendSize = GetInfoSize<InfoType, DataType>(nItemCount);

	if (!*ppDataBlock ||  nAppendSize + pBlockHead->m_nOffset > pBlockHead->m_nTotalSize)
	{
		if (*ppDataBlock)
		{
			PushToListAndNotify(*ppDataBlock, m_listDataBlock, m_lockDataBlockList, m_hEventDataArrived);
			*nLastUpdateTime = ::_time64(NULL);
		}
		unsigned int nSizeNeeded = GetInfoTotalSize<InfoType, DataType>(nItemCount);
		unsigned int nSizeAllocated = nSizeNeeded > m_nMaxBuf ? nSizeNeeded : m_nMaxBuf;
		*ppDataBlock = new char[nSizeAllocated];
		//Print("--------------new data block:0x%x--------------\n", *ppDataBlock);
		InitDataBlockHead<InfoType>(&pBlockHead, *ppDataBlock, nSizeAllocated);
		pBlockHead->hTdf = hTdf;
		pBlockHead->nMsgId = (TDF_MSG_ID)nMsgId;
	}

	InfoType* pInfo = (InfoType*)& (*ppDataBlock) [pBlockHead->m_nOffset];
	pInfo->nItems = nItemCount;
	pInfo->nServerTime = nServerTime;
	GetLocalTime(&pInfo->st);

	memcpy(pInfo->rawData, pDataSource, nItemSize*nItemCount);
	pBlockHead->m_nOffset += nAppendSize;

	if (pBlockHead->m_nOffset > pBlockHead->m_nTotalSize)
	{
		assert(0);
	}
	else if (pBlockHead->m_nOffset == pBlockHead->m_nTotalSize || pBlockHead->m_nOffset > pBlockHead->m_nTotalSize-sizeof(InfoType))
	{
		PushToListAndNotify(*ppDataBlock, m_listDataBlock, m_lockDataBlockList, m_hEventDataArrived);
		*ppDataBlock = NULL;
		*nLastUpdateTime = ::_time64(NULL);
	}
	else
	{
		unsigned int nCurTick = ::_time64(NULL);
		Print("curTick = %d, nLastUpdateTime=%d,nCurTick - nLastUpdateTime = %d\n",nCurTick,*nLastUpdateTime,nCurTick - *nLastUpdateTime);
		//if (nLastUpdateTime!=0 && nCurTick - *nLastUpdateTime > m_nMaxWriteTimeGap)
		{
			PushToListAndNotify(*ppDataBlock, m_listDataBlock, m_lockDataBlockList, m_hEventDataArrived);
			*ppDataBlock = NULL;
			*nLastUpdateTime = nCurTick;
		}
	}
}


}}} // namespace
