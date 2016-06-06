/***************************************
*	Description: System Task Info module.
*	Author: wilson
*	Date: 2015/11/30
*
*	CopyRight: Adirect Technology Co, Ltd
*
****************************************/

#ifndef __AD_TASK_H__
#define	__AD_TASK_H__
#include "AdThread.h"
#include "AdTime.h"

#define AD_TASK_SERVER           	0x80000
#define AD_TASK_WORKER            	0x90000
#define AD_TASK_BIDDING                0xa0000
#define AD_TASK_WRITE              	0xb0000
#define AD_TASK_OTHER            	0xc0000
#define AD_TASK_MASK_ID        	0xffff
#define AD_TASK_MASK_TYPE  	0xF0000

class AdTaskState
{
public:
	AdTaskState(){};
	virtual ~AdTaskState(){};
	virtual void Print()
	{
	};
};

class AdTask:public AdThread
{
public:
	AdTask()
	{
		m_bTaskRun = true;
		m_uiPrint = 0;
		m_uiTaskId = 0;
		m_uiLoop = 0;
		m_bTaskWaitting = false;
		m_pState = NULL;
	};

	virtual ~AdTask()
	{
		destroy();
	};

	void destroy()
	{
		AdThread::destroy();
		m_bTaskRun = false;
		m_bTaskWaitting = false;
		m_uiPrint = 0;
		m_uiTaskId = 0;
		m_uiLoop = 0;
		m_pState = NULL;
		AdThread::waitExit(NULL);
		AdThread::clear();
	};

	int  create(int iCpu = AD_FAILURE)
	{
		m_bTaskRun = true;
		AD_INFO("    Create Task: %s(0x%x)\n", m_sDescription.c_str(), m_uiTaskId);
		return AdThread::create(iCpu);
	};

	void SetTaskWaitting(bool bWaitting)
	{
		m_bTaskWaitting = bWaitting;
	};

	bool TaskWaitting()
	{
		return m_bTaskWaitting;
	};
		

	virtual void TaskRun()
	{
		AD_INFO("Test: Task Run Ok\n");
		Time::wait(10);
	};

	virtual void TaskLoop()
	{
		while(m_bTaskRun)
		{
			TaskRun();
			m_uiLoop ++;
		}
	};

	void TaskPrintState()
	{
		if(m_pState != NULL)
		{
			m_pState->Print();
		}
		else
		{
			AD_PRINT("	Loop:           %d\n", m_uiLoop);
		}
	};
	
	//task print msg 
	UINT m_uiPrint;
	//task descreption
	std::string m_sDescription;
	//task run or stop
	bool m_bTaskRun;
	//task id
	UINT m_uiTaskId;
	//task loop counter
	UINT m_uiLoop;
	//task state
	AdTaskState *m_pState;
	//Task Status
	bool m_bTaskWaitting;
};

template<typename Request, typename Para1, typename Para2>
Request *  AdTaskCreate(UINT id, Para1 p1, Para2 p2)
{
	int ret;

	Request *pTask = new Request(p1, p2);
	if(pTask == NULL)
	{
		AD_ERROR("New NetTask Info Error\n");
		return NULL;
	}
	pTask->m_uiTaskId= id;
	
	ret = pTask->create();
	if (ret != AD_SUCCESS) 
	{
		delete pTask;
		AD_ERROR("Task create error\n");
		return NULL;
	}
	return pTask;
};

template<typename Request, typename Para1>
Request *  AdTaskCreate(UINT id, Para1 p1)
{
	int ret;

	Request *pTask = new Request(p1);
	if(pTask == NULL)
	{
		AD_ERROR("New NetTask Info Error\n");
		return NULL;
	}
	pTask->m_uiTaskId= id;
	
	ret = pTask->create();
	if (ret != AD_SUCCESS) 
	{
		delete pTask;
		AD_ERROR("Task create error\n");
		return NULL;
	}
	return pTask;
};

template<typename Request>
Request *  AdTaskCreate(UINT id)
{
	int ret;

	Request *pTask = new Request();
	if(pTask == NULL)
	{
		AD_ERROR("New NetTask Info Error\n");
		return NULL;
	}
	pTask->m_uiTaskId= id;
	
	ret = pTask->create();
	if (ret != AD_SUCCESS) 
	{
		delete pTask;
		AD_ERROR("Task create error\n");
		return NULL;
	}
	return pTask;
};

#endif

