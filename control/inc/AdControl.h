/***************************************
*	Description: output module, store output info
*	Author: wilson
*	Date: 2015/09/10
*
*	CopyRight: Adirect Technology Co, Ltd
*
****************************************/

#ifndef   __AD_OUTPUT_H__
#define __AD_OUTPUT_H__

#include <map>
#include <list>
#include <vector>
#include <iostream>
#include "AdGlobal.h"
#include "AdBuff.h"
#include "AdServer.h"
#include "AdNetTask.h"
#include "AdTime.h"

using namespace std;

class AdOutputCache
{
public:
	AdOutputCache(){};
	~AdOutputCache(){};
	void Write(char * buff)
	{
		string & sCache = m_ctCaches.Get();
		sCache += buff;
	}

	const char *  GetRead(int * iLen)
	{
		string & sCache = m_ctCaches.GetBack();
		
		if((m_iGets == 0) &&(sCache.size() > 0))
			sCache.clear();
		
		if(sCache.empty())
		{
			m_iGets ++;
			if(m_iGets >= 2)
			{
				if(m_ctCaches.Get().size() > 0)
				{
					m_ctCaches.Switch();
				}
			}
			return NULL;
		}
		m_iGets = 0;
		*iLen = sCache.size();
		return sCache.c_str();
	}
	
	
private:
	int m_iGets;
	AdPingPongBuffer<string> m_ctCaches;
};


class AdControl:public AdNetTaskInterface
{
public:
	AdControl()
	{
		string sName = "Control";
		SetName(sName);
	};
	virtual ~AdControl(){};

	//Init Output
	int OutputInit(int iPort);
	//Write Output msg
	int WriteOutput(char * pBuff);
	//Output Server
	virtual  int Run(AdCache * pCache, AdSession * pSession, AdTask & pTask);

	//add task for managment
	void AddTask(AdTask * pTask);
	//get Control mudule
	static AdControl & Instance();

	void TaskDetect(int iSecond);
private:
	//task list
	map<UINT, AdTask *>  m_mTasks;

	//output
	int m_iPort;
        AdOutputCache m_ctCache;
	AdServer m_ctServer;

	//project path, version
	string m_sProjectPath;
};
#endif
