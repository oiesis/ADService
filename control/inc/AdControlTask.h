/***************************************
*	Description: System Task Info module.
*	Author: wilson
*	Date: 2015/11/30
*
*	CopyRight: Adirect Technology Co, Ltd
*
****************************************/

#ifndef __AD_CONTROL_TASK_H__
#define	__AD_CONTROL_TASK_H__
#include "AdGlobal.h"
#include "AdNetTask.h"
#include "AdControl.h"

//Taskcreate, manage task, 
template<typename Request, typename Para1, typename Para2>
Request *  AdControlCreateTask( UINT id, Para1 p1, Para2 p2)
{
	AdControl &ctControl = AdControl::Instance();
	
	Request *pTask = AdTaskCreate<Request, Para1, Para2>(id, p1, p2);
	if(pTask == NULL)
	{
		AD_ERROR("New Task Info Error\n");
		return NULL;
	}
	ctControl.AddTask(static_cast<AdTask *>(pTask));
	return pTask;
};

//Taskcreate, manage task, 
template<typename Request, typename Para1>
Request *  AdControlCreateTask( UINT id, Para1 p1)
{
	AdControl &ctControl = AdControl::Instance();
	
	Request *pTask = AdTaskCreate<Request, Para1>(id, p1);
	if(pTask == NULL)
	{
		AD_ERROR("New Task Info Error\n");
		return NULL;
	}
	ctControl.AddTask(static_cast<AdTask *>(pTask));
	return pTask;
};

//Taskcreate, manage task, 
template<typename Request>
Request *  AdControlCreateTask( UINT id)
{
	AdControl &ctControl = AdControl::Instance();
	
	Request *pTask = AdTaskCreate<Request>(id);
	if(pTask == NULL)
	{
		AD_ERROR("New Task Info Error\n");
		return NULL;
	}
	ctControl.AddTask(static_cast<AdTask *>(pTask));
	return pTask;
};

template< typename Para1, typename Para2>
int  AdControlNetCreateTask(Para1  p1, Para2  p2,  int iRead, int iWrite)
{
	AdServerTask * pServer; 

	pServer = AdControlCreateTask<AdServerTask, Para1, Para2>(1 | AD_TASK_SERVER, p1, p2);
	if(pServer == NULL)
	{
		AD_ERROR("AdServerTask Create Error\n");
		return AD_FAILURE;
	}

	while(iRead > 0)
	{
		AdWorkerTask * pWork= AdControlCreateTask<AdWorkerTask, Para1, Para2>(iRead | AD_TASK_WORKER, p1, p2);
		if(pWork == NULL)
		{
			AD_ERROR("AdWorkerTask Create Error\n");
			return AD_FAILURE;
		}
		iRead --;
	}

	while(iWrite > 0)
	{
		AdWriteTask * pWrite = AdControlCreateTask<AdWriteTask, Para1, Para2>(iWrite | AD_TASK_WRITE, p1, p2);
		if(pWrite == NULL)
		{
			AD_ERROR("AdWorkerTask Create Error\n");
			return AD_FAILURE;
		}
		iWrite --;
	}
	
	return AD_SUCCESS;
};

#endif
