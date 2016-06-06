#include <map>
#include <list>
#include <vector>
#include <iostream>
#include <pthread.h>
#include <getopt.h>
#include <stdlib.h>
#include "AdGlobal.h"
#include "AdServer.h"
#include "AdSession.h"
#include "AdNetTask.h"

using namespace std;

map<UINT, AdTaskInfo>  g_mThreadList;

int AdRtbOutput(char * buff, int len)
{
	return AD_FAILURE;
}

int main(int argc, char ** argv)
{
	int iThread = 1;
	int iIndex;
	int ret;
	string  sPort  = "20251";
	
	struct option stOptions[] = 
	{
		{"port", 1, 0, 'p'},
		{"thread", 1, 0, 't'},
		{"help", 0, 0, 'h'},
		{0, 0, 0, 0},
	};
	
	while (true)
	{
		ret = getopt_long(argc, argv, "p:t:h", stOptions, &iIndex);
		if (AD_FAILURE != ret) 
		{
			switch (ret)
			{
				case 'p':
					sPort = optarg;
					break;
				case 't':
					iThread = atoi(optarg);
					break;
				case 'h':
					AD_PRINT("Usage: %s [options]\n", argv[0]);
					AD_PRINT("Valid options are:\n");
					AD_PRINT("[0] -p or --port: 	  set listen port\n");
					AD_PRINT("[0] -t or --thread:	  set thread number\n");
					AD_PRINT("[0] -h or --help: 	  show help.\n");
					AD_PRINT("Adirect Technology Co, Ltd\n");
					AD_PRINT("AdRtbAdapter Build on %s %s.\n", __DATE__, __TIME__);
					return AD_SUCCESS;
				default:
					return AD_SUCCESS;
			}
		}
		else 
		{
			break;
		}
	}

	AdServer cServer;
	cout<<"***********Server Init**************\n"<<endl;
	ret = cServer.AdServerInit(atoi(sPort.c_str()), iThread, 1);
	if(ret != AD_SUCCESS)
	{
		AD_ERROR("AdServer init error\n");
		return ret;
	}

	AdTaskInfo cInfo;
	cInfo.m_pNetBase = static_cast<AdNetBase *>(&cServer);
	cInfo.m_pData = NULL;
	cInfo.m_pFunction = NULL;

	ret = AdNetTaskCreate(cInfo, g_mThreadList, iThread, 1);
	if(ret != AD_SUCCESS)
	{
		AD_ERROR("AdServer Create error\n");
		return ret;
	}

	map<UINT, AdTaskInfo>::iterator  iter;
	while(true)
	{
		sleep(10);
		for(iter = g_mThreadList.begin(); iter != g_mThreadList.end(); iter ++)
		{
			if(iter->second.m_bTaskRun == false)
				AD_ERROR("Server error , task 0x%x exit\n", iter->second.m_uiTaskId);
		}
	}
	
	return AD_SUCCESS;
}
