/***************************************
*	Description: Os Systerm interface, Thread.
*	Author: wilson
*	Date: 2015/11/18
*
*	CopyRight: Adirect Technology Co, Ltd
*
****************************************/

#ifndef __AD_THREAD_H__
#define	__AD_THREAD_H__

#include "AdGlobal.h"
#include <unistd.h>
#include <pthread.h>
#include <errno.h>


class AdThread
{
public:
	AdThread();
	virtual ~AdThread();

	void clear();

	int destroy();
	
	int create( int iCpu = AD_FAILURE);
	
	int  waitExit(void ** ppRet = NULL);

	inline int cpu()
	{
		int num = sysconf(_SC_NPROCESSORS_CONF);
		return num;
	};

	int setAffinity( int iCpu);

	virtual void TaskLoop()
	{
		AD_INFO("Test: Thread Start Ok\n");
	};

	inline pthread_t getPrivateId()
	{
		return m_iID;
	}

private:
	
	pthread_t m_iID;
	int m_iCpu;
	pthread_attr_t m_stAttr;
	bool m_bNoAttr;
};

#endif 

