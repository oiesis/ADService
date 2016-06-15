/***************************************
*	Description: output module, store output info
*	Author: wilson
*	Date: 2015/09/10
*
*	CopyRight: Adirect Technology Co, Ltd
*
****************************************/

#include "AdGlobal.h"
#include "AdControl.h"
#include "AdControlTask.h"

struct AdDebug
{
	UINT   uiThread;
	UINT   uiType;
	UINT   uiData;
};

#define AD_DEBUG_PRINT_REALTIME                     1
#define AD_DEBUG_PRINT_PROCESS                         2
#define AD_DEBUG_PRINT_INFO                                 3

static AdControl g_ctControl;

void AdControl::AddTask(AdTask * pTask)
{
	m_mTasks[m_mTasks.size()] = pTask;
}

void AdControl::TaskDetect(int iSecond)
{
	map<UINT, AdTask *>::iterator  iter;
	map<UINT, UINT> mLastCounter;
	while(running)
	{
		Time::wait(iSecond);
		for(iter = m_mTasks.begin(); iter != m_mTasks.end(); iter ++)
		{
			if(iter->second->m_uiLoop == mLastCounter[iter->first])
			{
				if(!iter->second->TaskWaitting())
					iter->second->create();
			}
			else
			{
				mLastCounter[iter->first] = iter->second->m_uiLoop;
			}
			
			if(iter->second->m_bTaskRun == false)
			{
				AD_ERROR("Server error , task 0x%x exit\n", iter->second->m_uiTaskId);
				goto EXIT;
			}
		}
	}

EXIT:
	for(iter = m_mTasks.begin(); iter != m_mTasks.end(); iter ++)
	{
		iter->second->destroy();
	}

	Time::wait(1);

	for(iter = m_mTasks.begin(); iter != m_mTasks.end(); iter ++)
	{
		delete iter->second;
	}
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

//Init Output
int AdControl::OutputInit(int iPort)
{
	int ret;
	if(iPort > 0)
		m_iPort = iPort;
	
	//Get System path
	char path[AD_BYTE512];
	getcwd(path,AD_BYTE512);
	m_sProjectPath = path;
	
	//init net module
	cout<<"***********Output Server Init**************\n"<<endl;
	ret = m_ctServer.AdServerInit(m_iPort, 1, 0);
	if(ret != AD_SUCCESS)
	{
		AD_ERROR("AdServer init error\n");
		return ret;
	}
	
	//create net (server,  work,  write) task
	ret = AdControlNetCreateTask( static_cast<AdNetBase *>(&m_ctServer), 
			static_cast<AdNetTaskInterface*>(this), 1, 0);
	if(ret != AD_SUCCESS)
	{
		AD_ERROR("AdServer Create error\n");
		return ret;
	}
	return AD_SUCCESS;
}

//Write Output msg
int AdControl::WriteOutput(char * pBuff)
{
	m_ctCache.Write(pBuff);
	return AD_SUCCESS;
}

//Get Control
AdControl & AdControl::Instance()
{
	return g_ctControl;
}

//Output Server
int AdControl::Run(AdCache * pCache, AdSession * pSession, AdTask & ctTask)
{
	int ret;
	int iLen;
	
	map<UINT, AdTask *>::iterator iter;
	AdTask * pTask = static_cast<AdTask *>(pSession->AdSessionGetPrivate());

	if(pCache != NULL) 
	{
		pSession->AdSessionSetTimeout(100);
		//get command info, parse it
		if(pCache->Size()== sizeof(AdDebug))
		{
			struct AdDebug * pDebug = (struct AdDebug *)pCache->Get();
			switch(pDebug->uiType)
			{
				case AD_DEBUG_PRINT_REALTIME:
					break;
				case AD_DEBUG_PRINT_PROCESS:
					iter = m_mTasks.find(pDebug->uiThread);
					if(iter != m_mTasks.end())
					{
						iter->second->m_uiPrint = pDebug->uiData;
						pSession->AdSessionSetPrivate(static_cast<void *>(iter->second));
					}
					break;
				case AD_DEBUG_PRINT_INFO:
				default:
					AD_PRINT("Ad Rtb System Info(%s):\n", m_sProjectPath.c_str());
					for(iter = m_mTasks.begin(); iter != m_mTasks.end(); iter ++)
					{
						AD_PRINT("%02d  : %s( 0x%x & 0x%x)\n", iter->first,iter->second->m_sDescription.c_str(),
							iter->second->m_uiTaskId, iter->second->getPrivateId());
						iter->second->TaskPrintState();
					}
					break;
			}
		}
	}

	if(!pSession->AdSessionOk())
	{
		if(pTask != NULL)
		{
			pTask->m_uiPrint = 0;
			pSession->AdSessionGetPrivate(true);
			pSession->AdSessionSetTimeout(0);
		}
	}
	else
	{
		const char *pSend = m_ctCache.GetRead(&iLen);
		if(pSend != NULL)
		{
			ret = pSession->AdSessionWrite(pSend, iLen);
			if(ret == AD_FAILURE)
			{
				AD_ERROR("Msg Output Error\n");
			}
		}
	}

	ctTask.m_uiLoop++;
	return  AD_SUCCESS;
}

int AdRtbOutput(char * buff, int len)
{
	AdControl::Instance().WriteOutput(buff);
	return AD_SUCCESS;
}

