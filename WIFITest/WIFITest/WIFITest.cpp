// WIFITest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>

#include <windows.h> 
#include <wlanapi.h> 
#include <objbase.h> 
#include <wtypes.h> 
#include <string> 
#include <stdio.h> 
#include <stdlib.h> 

using namespace std;

// Need to link with Wlanapi.lib and Ole32.lib 
#pragma comment(lib, "wlanapi.lib") 
#pragma comment(lib, "ole32.lib") 

int ListenStatus();

int main(int argc, char* argv[])
{
	HANDLE hClient = NULL;
	DWORD dwMaxClient = 2;
	DWORD dwCurVersion = 0;
	DWORD dwResult = 0;
	DWORD dwRetVal = 0;
	int iRet = 0;

	/* variables used for WlanEnumInterfaces */

	PWLAN_INTERFACE_INFO_LIST pIfList = NULL;
	PWLAN_INTERFACE_INFO pIfInfo = NULL;

	LPCWSTR pProfileName = NULL;
	LPWSTR pProfileXml = NULL;
	DWORD dwFlags = 0;

	// 1. 首先指定接收转换结果的指针
	WCHAR * wideChar;
	// 2. 然后第一次调用函数，获取存储转换结果所需缓冲区的大小
	int len = MultiByteToWideChar(CP_ACP, NULL,
		argv[1],		// 要被转换的多字节字符串
		-1,				// 要转换的长度，设为-1表示转换整串
		NULL,			// 第一次调用，接收缓冲区设为NULL
		0);				// 接收缓冲区长度，设为0代表函数返回需要的长度
	// 3. 根据返回的结果创建合适大小的缓冲区
	wideChar = new WCHAR[len];
	// 4. 第二次调用函数，进行真正的转换
	MultiByteToWideChar(CP_ACP, NULL,
		argv[1],		// 要被转换的多字节字符串
		-1,				// 要转换的长度，设为-1表示转换整串
		wideChar,		// 第二次调用，设为接收转换结果的缓冲区
		len);			// 设置接收缓冲区的大小

	pProfileName = wideChar;

	wprintf(L"Information for profile: %ws\n\n", pProfileName);

	//// 获取本地无线网卡的句柄
	dwResult = WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, &hClient);
	if (dwResult != ERROR_SUCCESS) {
		wprintf(L"WlanOpenHandle failed with error: %u\n", dwResult);
		return 1;
	}

	//// 遍历网卡，可能有多块 
	dwResult = WlanEnumInterfaces(hClient, NULL, &pIfList);
	if (dwResult != ERROR_SUCCESS) {
		wprintf(L"WlanEnumInterfaces failed with error: %u\n", dwResult);
		return 1;
	}
	else {
		dwResult = WlanDisconnect(hClient, &pIfList->InterfaceInfo[0].InterfaceGuid, NULL);//DISCONNECT FIRST
		if (dwResult != ERROR_SUCCESS)
		{
			printf("WlanDisconnect failed with error: %u\n", dwResult);
			return -1;
		}
		PWLAN_AVAILABLE_NETWORK_LIST pWLAN_AVAILABLE_NETWORK_LIST = NULL;
		// 获取某块网卡扫描到的AP；
		dwResult = WlanGetAvailableNetworkList(hClient, &pIfList->InterfaceInfo[0].InterfaceGuid,
			WLAN_AVAILABLE_NETWORK_INCLUDE_ALL_MANUAL_HIDDEN_PROFILES,
			NULL, &pWLAN_AVAILABLE_NETWORK_LIST);
		if (dwResult != ERROR_SUCCESS)
		{
			printf("WlanGetAvailableNetworkList failed with error: %u\n", dwResult);
			WlanFreeMemory(pWLAN_AVAILABLE_NETWORK_LIST);
			return -1;
		}
		WLAN_AVAILABLE_NETWORK wlanAN = pWLAN_AVAILABLE_NETWORK_LIST->Network[0];//PLEASE CHECK THIS YOURSELF 
		if (pProfileName == NULL)
			pProfileName = wlanAN.strProfileName;
		WLAN_CONNECTION_PARAMETERS wlanConnPara;
		wlanConnPara.wlanConnectionMode = wlan_connection_mode_profile; //YES,WE CONNECT AP VIA THE PROFILE 
		wlanConnPara.strProfile = pProfileName;       // set the profile name 
		wlanConnPara.pDot11Ssid = NULL;         // SET SSID NULL 
		wlanConnPara.dot11BssType = dot11_BSS_type_infrastructure;  //dot11_BSS_type_any,I do not need it this time.   
		wlanConnPara.pDesiredBssidList = NULL;       // the desired BSSID list is empty 
		wlanConnPara.dwFlags = WLAN_CONNECTION_HIDDEN_NETWORK;
		dwResult = WlanConnect(hClient, &pIfList->InterfaceInfo[0].InterfaceGuid, &wlanConnPara, NULL);
		if (dwResult == ERROR_SUCCESS)
		{
			printf("WlanConnect success!\n");
		}
		else
		{
			printf("WlanConnect failed err is %d\n", dwResult);
		}
	}
	ListenStatus(); //LISTEN THE STATUS 
	if (pProfileXml != NULL) {
		WlanFreeMemory(pProfileXml);
		pProfileXml = NULL;
	}
	if (pIfList != NULL) {
		WlanFreeMemory(pIfList);
		pIfList = NULL;
	}
	return 0;
}

int ListenStatus() {
	HANDLE hClient = NULL;
	DWORD dwMaxClient = 2;
	DWORD dwCurVersion = 0;
	DWORD dwResult = 0;
	DWORD dwRetVal = 0;
	int iRet = 0;

	WCHAR GuidString[39] = { 0 };

	//Listen the status of the AP you connected. 
	Sleep(5000);
	PWLAN_INTERFACE_INFO_LIST pIfList = NULL;//I think wlan interface means network card 
	PWLAN_INTERFACE_INFO pIfInfo = NULL;
	DWORD dwFlags = 0;

	//// 获取本地无线网卡的句柄
	dwResult = WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, &hClient);
	if (dwResult != ERROR_SUCCESS) {
		wprintf(L"WlanOpenHandle failed with error: %u\n", dwResult);
		return 1;
	}

	//// 遍历网卡，可能有多块 
	dwResult = WlanEnumInterfaces(hClient, NULL, &pIfList);
	if (dwResult != ERROR_SUCCESS) {
		wprintf(L"WlanEnumInterfaces failed with error: %u\n", dwResult);
		return 1;
	}
	else
	{
		wprintf(L"WLAN_INTERFACE_INFO_LIST for this system\n");
		wprintf(L"Num Entries: %lu\n", pIfList->dwNumberOfItems);
		wprintf(L"Current Index: %lu\n\n", pIfList->dwIndex);

		int i;
		for (i = 0; i < (int)pIfList->dwNumberOfItems; i++) {
			pIfInfo = (WLAN_INTERFACE_INFO *)&pIfList->InterfaceInfo[i];
			wprintf(L" Interface Index[%u]:\t %lu\n", i, i);
			iRet = StringFromGUID2(pIfInfo->InterfaceGuid, (LPOLESTR)&GuidString,
				sizeof(GuidString) / sizeof(*GuidString));
			if (iRet == 0)
				wprintf(L"StringFromGUID2 failed\n");
			else {
				wprintf(L" InterfaceGUID[%d]: %ws\n", i, GuidString);
			}
			wprintf(L" Interface Description[%d]: %ws", i,
				pIfInfo->strInterfaceDescription);
			wprintf(L"\n");
			wprintf(L" Interface State[%d]:\t ", i);
			switch (pIfInfo->isState) {
			case wlan_interface_state_not_ready:
				wprintf(L"Not ready\n");
				break;
			case wlan_interface_state_connected:// 用来判断某个信号是否为当前链接的AP 
				wprintf(L"Connected\n");
				break;
			case wlan_interface_state_ad_hoc_network_formed:
				wprintf(L"First node in a ad hoc network\n");
				break;
			case wlan_interface_state_disconnecting:
				wprintf(L"Disconnecting\n");
				break;
			case wlan_interface_state_disconnected:
				wprintf(L"Not connected\n");
				break;
			case wlan_interface_state_associating:
				wprintf(L"Attempting to associate with a network\n");
				break;
			case wlan_interface_state_discovering:
				wprintf(L"Auto configuration is discovering settings for the network\n");
				break;
			case wlan_interface_state_authenticating:
				wprintf(L"In process of authenticating\n");
				break;
			default:
				wprintf(L"Unknown state %ld\n", pIfInfo->isState);
				break;
			}
		}
		return 0;
	}
}

