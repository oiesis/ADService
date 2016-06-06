/***************************************
*	Description: Os Systerm interface, Thread.
*	Author: wilson
*	Date: 2015/11/18
*
*	CopyRight: Adirect Technology Co, Ltd
*
****************************************/
#include <string.h>
#include "AdThread.h"

void* ThreadHandle(void*pData)
{
	AdThread * pThread = static_cast<AdThread  * >(pData);
	if(pThread != NULL)
		pThread->TaskLoop();
	else
		AD_INFO("Thread No Input Para, Stop\n");

	return NULL;
}

AdThread::AdThread()
{
	clear();
}

AdThread::~AdThread()
{
	destroy();
	clear();
}

void AdThread::clear()
{
	m_iID = 0;
	m_bNoAttr = true;
	memset(&m_stAttr, 0 , sizeof(pthread_attr_t));
}

int AdThread::destroy()
{
	if(m_iID != 0)
	{
		if (AD_SUCCESS != pthread_cancel(m_iID)) 
		{
			AD_ERROR("Thread destroy error %d  %s\n", (int)m_iID, strerror(errno));
		}
	}

	return AD_SUCCESS;
}
	
int AdThread::create( int iCpu)
{
	int iRet;
	pthread_attr_t * pAttr;
	if(m_bNoAttr)
		pAttr = NULL;
	else
		pAttr = &m_stAttr;
		
	if(m_iID != 0)
	{
		AD_ERROR("Last thread exist, destroy\n");
		destroy();
		waitExit();
	}

	iRet = pthread_create(&m_iID, pAttr, ThreadHandle, static_cast<void *>(this));
	if(iRet != AD_SUCCESS)
	{
		AD_ERROR("Thread Create Error %d, %s\n",(int)m_iID, strerror(errno));
		return AD_FAILURE;
	}

	if(iCpu > 0)
	{
		setAffinity(iCpu);
	}

	return AD_SUCCESS;
};
	
int  AdThread::waitExit(void ** ppRet)
{
	int iRet;
	iRet = pthread_join(m_iID, ppRet);
	if(iRet != AD_SUCCESS)
	{
		AD_ERROR("Thread Wait exit Error %d, %s\n",(int)m_iID, strerror(errno));
		return AD_FAILURE;
	}
	clear();
	return AD_SUCCESS;
};


int AdThread::setAffinity( int iCpu)
{	
	cpu_set_t mask;
	cpu_set_t get;
	int j;
	int num = cpu();

	if((iCpu < 0) || (iCpu >= num))
	{
		return AD_FAILURE;
	}
		
	m_iCpu = iCpu;
	CPU_ZERO(&mask);
	CPU_SET(m_iCpu, &mask);
	if (pthread_setaffinity_np(m_iID, sizeof(mask), &mask) < 0) 
	{
		AD_ERROR("Set thread affinity failed  %d, %s\n",(int)m_iID, strerror(errno));
		return AD_FAILURE;
	}

	CPU_ZERO(&get);
	if (pthread_getaffinity_np(m_iID, sizeof(get), &get) < 0) 
	{
		AD_ERROR("Get thread affinity failed %d, %s\n",(int)m_iID, strerror(errno));
		return AD_FAILURE;
	}

	for (j = 0; j < num; j++) 
	{
		if (CPU_ISSET(j, &get)) 
		{
			printf("thread %d is running in processor %d(set: %d)\n", (int)m_iID,  j, m_iCpu);
		}
	}

	return AD_SUCCESS;
}
