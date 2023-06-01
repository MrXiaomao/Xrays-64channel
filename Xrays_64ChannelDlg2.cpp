#include "json/json.h"

#include "pch.h"
#include "framework.h"
#include "Xrays_64Channel.h"
#include "Xrays_64ChannelDlg.h"
#include "afxdialogex.h"
#include "Order.h"

#include "afx.h"
// 该文件存放不常用函数，以及功能函数

// 文件操作
#include <iostream>
#include <fstream>
using namespace std;


// 打开UDP通信
void CXrays_64ChannelDlg::OpenUDP()
{
	UpdateData(TRUE); //读取界面控件的输入值
	// 读取配置参数并设置到相应控件上
	Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
	//配置文件不存在，则生成配置文件
	jsonSetting["Port_UDP"] = m_UDPPort;
	WriteSetting(_T("Setting.json"),jsonSetting);

	//--------------1.创建UDPSocket------------
	if (!m_UDPSocket) delete m_UDPSocket;
	m_UDPSocket = new CClientSocket(this);//初始化,新创建一个对话框Socket
	m_UDPSocket->Create(m_UDPPort, SOCK_DGRAM, NULL);

	//--------------2.获取Socket绑定的ip和端口--------------
	//获取本机的IP和端口号
	CString strIp;
	UINT uiPort;

	//获取本地的服务号和端口号
	m_UDPSocket->GetSockName(strIp, uiPort);

	//显示本地的端口号和IP号
	SetDlgItemText(IDC_UDPIP, strIp);
	SetDlgItemInt(IDC_UDPPORT, uiPort);
	CString info;
	info.Format(_T("UDP已打开，端口号为:%d"), uiPort);
	m_page1->PrintLog(info);
	m_page2->PrintLog(info);
}

void CXrays_64ChannelDlg::CloseUDP() {
	if (m_UDPSocket != NULL) delete m_UDPSocket;
	CString info = _T("UDP网络已关闭");
	m_page1->PrintLog(info);
	m_page2->PrintLog(info);
}

void CXrays_64ChannelDlg::SaveFile(CString myID, const char* mk, int length) {
	CString filename = myID + _T(".dat");
	CString wholePath = saveAsPath + filename;
	fstream datafile(wholePath, ios::out | ios::app | ios::binary);   // 追加
	for (int i = 0; i < length; i++) {
		datafile << mk[i];
	}
	datafile.close();
}

//限制端口输入范围0~65535
void CXrays_64ChannelDlg::OnEnKillfocusPort1()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(true);
	if ((sPort < 0) || (sPort > 65535))
	{
		MessageBox(_T("端口的范围为0~65535\n"));
		if (sPort > 65535)
		{
			sPort = 65535;
		}
		else
		{
			sPort = 1;
		}
		UpdateData(false);
	}
}

//限制端口输入范围0~65535
void CXrays_64ChannelDlg::OnEnKillfocusUDPPort()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(true);
	if ((sPort < 0) || (sPort > 65535))
	{
		MessageBox(_T("端口的范围为0~65535\n"));
		if (sPort > 65535)
		{
			sPort = 65535;
		}
		else
		{
			sPort = 1;
		}
		UpdateData(false);
	}
}

// 设置文件存储路径
void CXrays_64ChannelDlg::OnBnClickedSaveas()
{
	// TODO: 在此添加控件通知处理程序代码
	Json::Value myJson = ReadSetting(_T("Setting.json"));
	CString dir;
	dir = myJson["SaveDir"].asCString();

	HWND hwnd = this->GetSafeHwnd();
	if (SetSavePath(hwnd, dir, saveAsPath))
	{
		// 写入参数文件
		string pStr = _UnicodeToUtf8(saveAsPath);
		myJson["SaveDir"] = pStr;
		WriteSetting(_T("Setting.json"), myJson);

		// 打印日志信息
		CString info = _T("实验数据保存路径：") + saveAsPath;
		m_page1->PrintLog(info);
		UpdateData(FALSE);
	}

	//获取目录路径
	/*TCHAR   szPath[MAX_PATH] = {0};
	LPITEMIDLIST   pitem;
	BROWSEINFO   info;
	::ZeroMemory(&info, sizeof(info));
	info.hwndOwner = this->m_hWnd;
	info.lpszTitle = _T("请选择路径: ");
	info.pszDisplayName = szPath;

	//CString转char*
	USES_CONVERSION;
	char* result = T2A(dir);
	wcscpy_s(szPath, dir);

	if (pitem = ::SHBrowseForFolder(&info))
	{
		::SHGetPathFromIDList(pitem, szPath);
		saveAsPath = szPath;
		saveAsPath += "\\";

		// 写入参数文件
		string pStr = _UnicodeToUtf8(saveAsPath);
		myJson["SaveDir"] = pStr;
		WriteSetting(_T("Setting.json"),myJson);

		// 打印日志信息
		CString info = _T("实验数据保存路径：") + saveAsPath;
		m_page1->PrintLog(info);
		UpdateData(FALSE);
	}
	*/
}
