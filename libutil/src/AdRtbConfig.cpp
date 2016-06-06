/***************************************
*	Description: Ad config module,  config file parse and input.
*	Author: wilson
*	Date: 2016/01/05
*
*	CopyRight: Adirect Technology Co, Ltd
*
****************************************/

#include <algorithm>
#include <stdio.h>
#include "AdRtbConfig.h"

AdRtbConfig::AdRtbConfig()
{
	Clear();
	m_vInvalidChar.push_back(' ');
	m_vInvalidChar.push_back('\t');
	m_vInvalidChar.push_back('\n');
	m_vInvalidChar.push_back('\r');

}

AdRtbConfig::~AdRtbConfig()
{
	Clear();
	m_vInvalidChar.clear();
	m_sConfigFile.clear();
}

void AdRtbConfig::Clear()
{
	m_mChangedFile.clear();
	m_mFileTimeStamp.clear();
	m_mConfig.clear();
}

int AdRtbConfig::StrDeleteInvalid(string& str)
{
	string::iterator it = str.begin();
	while(it != str.end())
	{
		if(find(m_vInvalidChar.begin(), m_vInvalidChar.end(), *it) != m_vInvalidChar.end())
			str.erase(it);
		else
			it ++;
	}
	return AD_SUCCESS;
}


int AdRtbConfig::Load(string & file)
{
	string sSection;
	
	char  line[AD_CONFIG_MAX];
	string sLine;
	
	vector<string> vStr;
	vStr.reserve(4);

	Clear();

	if(!file.empty())
		m_sConfigFile = file;
	
	FILE * pFile = fopen(m_sConfigFile.c_str(), "r");
	if (NULL == pFile) {
		AD_ERROR("%s fopen error\n", file.c_str());
		return AD_FAILURE;
	}
	
	 while(fgets(line, AD_CONFIG_MAX, pFile) != NULL)
	 {
	 	sLine.clear();
	 	sLine = line;
		StrDeleteInvalid(sLine);
		if(sLine.empty() || sLine[0]  == '#')
			continue;
		
		vStr.clear();
		AdString::StrSplit(sLine, vStr, "#");
		sLine.clear();
		sLine = vStr[0];

		vStr.clear();
		AdString::StrSplit(sLine, vStr, "=");

		if(vStr.size() == 1)
		{
			sSection = vStr[0];
			continue;
		}
		else if(vStr.size() == 2)
		{
			if(!sSection.empty())
				m_mConfig[sSection][vStr[0]] = vStr[1];
		}
	 }

	vStr.clear();
 	fclose(pFile);

	//get check file 
	ADCONFIG & mCheckFile = m_mConfig["[CHECK]"];
	//add config for check
	mCheckFile[m_sConfigFile] = m_sConfigFile;

	//get timestamp
	struct stat stFile;
	if (AD_SUCCESS == stat(m_sConfigFile.c_str(), &stFile))
	{
		memcpy(&m_mFileTimeStamp[m_sConfigFile], &stFile.st_atim, sizeof(struct timespec));
	}

	//get Generate Dev Id
	GetDevId(m_mConfig["[GLOBAL]"]["DEV"]);
	return AD_SUCCESS;
}

int AdRtbConfig::Check()
{
	struct stat stFile;

	//clear result
	m_mChangedFile.clear(); 

	//get check file 
	ADCONFIG & mCheckFile = m_mConfig["[CHECK]"];
	ADCONFIG_ITER iter;

	//init file timestamp
	for(iter = mCheckFile.begin(); iter != mCheckFile.end(); iter ++)
	{
		//get timestamp
		if (AD_SUCCESS == stat(iter->second.c_str(), &stFile))
		{
			if (AD_SUCCESS != memcmp(&m_mFileTimeStamp[iter->first], &stFile.st_atim, sizeof(struct timespec)))
			{
				memcpy(&m_mFileTimeStamp[iter->first], &stFile.st_atim, sizeof(struct timespec));
				m_mChangedFile[iter->first] ++;
			}
		}
	}
	
	return m_mChangedFile.size();
}

map<string, string> &   AdRtbConfig::Get(string & Section)
{
	return m_mConfig[Section];
}

bool  AdRtbConfig::FileIsChanged(string & sFile)
{
	if(sFile == "NULL")
		return (m_mChangedFile.size() > 0);
	if(m_mChangedFile.find(sFile) != m_mChangedFile.end())
		return true;
	return false;
}

bool  AdRtbConfig::FileIsChanged()
{
	return (m_mChangedFile.size() > 0);
}

int  AdRtbConfig::GetDevId(string & sId)
{
	int ret;
	string sEth = "eth0";
	string sIp;
	ret = GetIp(sEth, sIp);
	if(ret != AD_SUCCESS)
	{
		sEth = "em1";
		ret = GetIp(sEth, sIp);
		if(ret != AD_SUCCESS)
		{
			return ret;
		}
	}

	vector<string> vStr;
	AdString::StrSplit(sIp,  vStr, ".");
	if(vStr.size() == 4)
	{
		sId = vStr[3];
		cout<<"Generate Dev Id: "<<sId<<endl;
		return AD_SUCCESS;
	}
	
	return AD_FAILURE;
}

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
int AdRtbConfig::GetIp(string & sEth,string  & sIp)
{
	int iSocket;
	struct  sockaddr_in stAddrIn;
	struct ifreq stIfr;

	/**//* Get socket file descriptor */
	if ((iSocket = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
	{
		return AD_FAILURE;
	}

	/**//* Get IP Address */
	strncpy(stIfr.ifr_name, sEth.c_str(), IF_NAMESIZE);
	stIfr.ifr_name[IFNAMSIZ-1]='\0';

	if (ioctl(iSocket, SIOCGIFADDR, &stIfr) < 0)
	{
		close(iSocket);
		return AD_FAILURE;
	}
	close(iSocket);
	
	memcpy(&stAddrIn, &stIfr.ifr_addr, sizeof(stAddrIn));
	sIp = inet_ntoa(stAddrIn.sin_addr);
	
	return AD_SUCCESS;
}