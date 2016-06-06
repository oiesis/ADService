///***************************************
//*	Description: Event module
//*	Author: wilson
//*	Date: 2015/09/07
//*
//*	CopyRight: Adirect Technology Co, Ltd
//*
//****************************************/
//#include <unistd.h>
//#include <string.h>
//#include "AdEpoll.h"
//
//#define	AD_EVENT			1024
//#define	AD_EPOLL			50000
//
//
//AdEvent::AdEvent()
//{
//	m_iEpoll = epoll_create(AD_EPOLL);
//	if (m_iEpoll < 0)
//	{
//		AD_ERROR("Epool create Fail\n");
//	}
//}
//
//AdEvent::~AdEvent()
//{
//	if (m_iEpoll > 0) {
//		close(m_iEpoll);
//	}
//}
//
//int AdEvent::AdEventState(int iSocket)
//{
//	if(m_mState.find(iSocket) == m_mState.end())
//		return 0;
//	return m_mState[iSocket];
//}
//
//
//int AdEvent::AdEventAdd(int iSocket, UINT uiFlag)
//{
//	struct epoll_event stEpoll;
//
//	if((uiFlag & (AD_EVENT_READ | AD_EVENT_WRITE)) == 0)
//	{
//		return AD_FAILURE;
//	}
//
//	stEpoll.events = EPOLLET;
//	if(uiFlag & AD_EVENT_READ)
//	{
//		stEpoll.events |= EPOLLIN;
//	}
//	if(uiFlag & AD_EVENT_WRITE)
//	{
//		stEpoll.events |= EPOLLOUT;
//	}
//	stEpoll.data.fd = iSocket;
//	m_mState[iSocket] = uiFlag;
//	return epoll_ctl(m_iEpoll, EPOLL_CTL_ADD, stEpoll.data.fd, &stEpoll);
//}
//
//int AdEvent::AdEventModify(int iSocket, UINT uiFlag)
//{
//	struct epoll_event stEpoll;
//
//	if((uiFlag & (AD_EVENT_READ | AD_EVENT_WRITE)) == 0)
//	{
//		return AD_FAILURE;
//	}
//
//	stEpoll.events = EPOLLET;
//	if(uiFlag & AD_EVENT_READ)
//	{
//		stEpoll.events |= EPOLLIN;
//	}
//	if(uiFlag & AD_EVENT_WRITE)
//	{
//		stEpoll.events |= EPOLLOUT;
//	}
//	stEpoll.data.fd = iSocket;
//	m_mState[iSocket] = uiFlag;
//	return epoll_ctl(m_iEpoll, EPOLL_CTL_MOD, stEpoll.data.fd, &stEpoll);
//}
//
//
//int AdEvent::AdEventHandleMsg(int iSocket, int iMsgType)
//{
//	return 0;
//}
//
//int AdEvent::AdEventDel(int iSocket)
//{
//	struct epoll_event stEpoll;
//	stEpoll.data.fd = iSocket;
//	m_mState.erase(iSocket);
//	return epoll_ctl(m_iEpoll, EPOLL_CTL_DEL, stEpoll.data.fd, &stEpoll);
//}
//
//void AdEvent::AdEventWait(int iMilSeconds)
//{
//	int i = 0;
//	int iTotal = 0;
//	struct epoll_event stEvent[AD_EVENT];
//
//	memset(&stEvent, '\0', sizeof(stEvent));
//	iTotal = epoll_wait(m_iEpoll, stEvent, AD_EVENT, iMilSeconds);
//	for (i = 0;  i < iTotal;  i ++)
//	{
//		if ((stEvent[i].events & EPOLLERR) || (stEvent[i].events & EPOLLHUP))
//        	{
//        		cout<<stEvent[i].events <<":"<< EPOLLERR <<endl;
//			cout<< stEvent[i].events <<":"<< EPOLLHUP<<endl;
//            		AdEventHandleMsg(stEvent[i].data.fd, AD_EVENT_ERROR);
//          		continue;
//       		}
//		else
//		{
//			if (stEvent[i].events & EPOLLIN)
//        		{
//            			AdEventHandleMsg(stEvent[i].data.fd, AD_EVENT_READ);
//       			}
//			if (stEvent[i].events & EPOLLOUT)
//        		{
//            			AdEventHandleMsg(stEvent[i].data.fd, AD_EVENT_WRITE);
//       			}
//		}
//	}
//
//	return;
//}
