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

// 设置TCP的IP、PORT、复选框的输入使能状态
void CXrays_64ChannelDlg::SetTCPInputStatus(BOOL flag) {
	// IP
	GetDlgItem(IDC_IPADDRESS1)->EnableWindow(flag);
	GetDlgItem(IDC_IPADDRESS2)->EnableWindow(flag);
	GetDlgItem(IDC_IPADDRESS3)->EnableWindow(flag);
	GetDlgItem(IDC_IPADDRESS4)->EnableWindow(flag);
	// Port
	GetDlgItem(IDC_PORT1)->EnableWindow(flag);
	GetDlgItem(IDC_PORT2)->EnableWindow(flag);
	GetDlgItem(IDC_PORT3)->EnableWindow(flag);
	GetDlgItem(IDC_PORT4)->EnableWindow(flag);
}

//设置配置参数框的使能状态
void CXrays_64ChannelDlg::SetParameterInputStatus(BOOL flag) {
	//能谱刷新时间
	GetDlgItem(IDC_RefreshTimeEdit)->EnableWindow(flag); 
	//能谱测量时间
	GetDlgItem(IDC_MeasureTime)->EnableWindow(flag);
}

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
	//SetDlgItemText(IDC_UDPIP, strIp);
	SetDlgItemInt(IDC_UDPPORT, uiPort);
	CString info;
	info.Format(_T("UDP已打开，端口号为:%d"), uiPort);
	m_page1->PrintLog(info);
	m_page2->PrintLog(info);
	
	UDPStatus = TRUE;
	GetDlgItem(IDC_UDPPORT)->EnableWindow(FALSE);
	// UDP和TCP都打开后才允许使用自动测量
	if (connectStatus) { 
		GetDlgItem(IDC_AutoMeasure)->EnableWindow(TRUE);
	}
}

void CXrays_64ChannelDlg::CloseUDP() {
	if (m_UDPSocket != NULL) delete m_UDPSocket;
	CString info = _T("UDP网络已关闭");
	m_page1->PrintLog(info);
	m_page2->PrintLog(info);
	UDPStatus = FALSE;
	GetDlgItem(IDC_UDPPORT)->EnableWindow(FALSE);
	GetDlgItem(IDC_AutoMeasure)->EnableWindow(FALSE);
}

//保存TCP传输过来的测量数据
void CXrays_64ChannelDlg::SaveFile(CString myID, const char* mk, int length) {
	CString filename = myID + _T(".dat");
	CString wholePath = saveAsTargetPath + filename;
	fstream datafile(wholePath, ios::out | ios::app | ios::binary);   // 追加
	if (datafile.is_open()) {
		for (int i = 0; i < length; i++) {
			datafile << mk[i];
		}
		datafile.close();
	}
}

//限制TCP端口输入范围
void CXrays_64ChannelDlg::OnEnKillfocusPort1()
{
	ConfinePortRange(&sPort);
}

//限制TCP端口输入范围
void CXrays_64ChannelDlg::OnEnKillfocusPort2()
{
	ConfinePortRange(&sPort2);
}

//限制TCP端口输入范围
void CXrays_64ChannelDlg::OnEnKillfocusPort3()
{
	ConfinePortRange(&sPort3);
}

//限制TCP端口输入范围
void CXrays_64ChannelDlg::OnEnKillfocusPort4()
{
	ConfinePortRange(&sPort4);
}

//限制UDP端口输入范围
void CXrays_64ChannelDlg::OnEnKillfocusUDPPort()
{
	// TODO: 在此添加控件通知处理程序代码
	ConfinePortRange(&m_UDPPort);
}

//限制端口号输入范围0~65535
void CXrays_64ChannelDlg::ConfinePortRange(int* myPort) {
	UpdateData(true);
	if ((*myPort < 0) || (*myPort > 65535))
	{
		MessageBox(_T("端口的范围为0~65535\n"));
		if (*myPort > 65535)
		{
			*myPort = 65535;
		}
		else
		{
			*myPort = 1;
		}
		UpdateData(false);
	}
}

//限制能谱刷新时间范围,单位ms，
//能谱刷新时间指的是FPGA采集一个能谱数据所用时间
void CXrays_64ChannelDlg::OnEnKillfocusRefreshTime()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(true);
	int MaxTime = 60*1000; //单位ms
	if ((RefreshTime <= 0) || (RefreshTime > MaxTime))
	{
		CString message;
		message.Format(_T("能谱刷新时间范围为0~%dms\n"), MaxTime);
		MessageBox(message);
		if (RefreshTime > MaxTime)
		{
			RefreshTime = MaxTime;
		}
		else
		{
			RefreshTime = 1;
		}
		UpdateData(false);
	}
}

//限制能谱测量时间范围，单位ms
void CXrays_64ChannelDlg::OnEnKillfocusMeasureTime()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(true);
	int MaxTime = 60 * 1000; //单位ms
	if ((MeasureTime <= 0) || (MeasureTime > MaxTime))
	{
		CString message;
		message.Format(_T("能谱测量时间范围为0~%dms\n"), MaxTime);
		MessageBox(message);
		if (MeasureTime > MaxTime)
		{
			MeasureTime = MaxTime;
		}
		else
		{
			MeasureTime = 1;
		}
		UpdateData(false);
	}
}

//设置文件存储路径
void CXrays_64ChannelDlg::OnBnClickedSaveas()
{
	// TODO: 在此添加控件通知处理程序代码
	Json::Value myJson = ReadSetting(_T("Setting.json"));
	CString dir;
	if (myJson["SaveDir"].isString()) {
		dir = myJson["SaveDir"].asCString();
	}

	HWND hwnd = this->GetSafeHwnd();
	if (BrowserMyPath(hwnd, dir, saveAsPath))
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
}

//配置参数,通过网口向控制板发送参数
void CXrays_64ChannelDlg::SendParameterToTCP()
{
	// TODO: 在此添加控件通知处理程序代码
	// 读取界面参数并修改指令
	UpdateData(TRUE);
	char res[5];
	//能谱刷新时间
	DecToHex(RefreshTime, res);
	Order::WaveRefreshTime[6] = res[0];
	Order::WaveRefreshTime[7] = res[1];
	Order::WaveRefreshTime[8] = res[2];
	Order::WaveRefreshTime[9] = res[3];

	//发送指令
	send(mySocket, Order::WaveRefreshTime, 12, 0);
	send(mySocket2, Order::WaveRefreshTime, 12, 0);
	send(mySocket3, Order::WaveRefreshTime, 12, 0);
	send(mySocket4, Order::WaveRefreshTime, 12, 0);

	send(mySocket, Order::TriggerThreshold, 12, 0);
	send(mySocket2, Order::TriggerThreshold, 12, 0);
	send(mySocket3, Order::TriggerThreshold, 12, 0);
	send(mySocket4, Order::TriggerThreshold, 12, 0);

	send(mySocket, Order::TriggerIntervalTime, 12, 0);
	send(mySocket2, Order::TriggerIntervalTime, 12, 0);
	send(mySocket3, Order::TriggerIntervalTime, 12, 0);
	send(mySocket4, Order::TriggerIntervalTime, 12, 0);

	send(mySocket, Order::WorkMode, 12, 0);
	send(mySocket2, Order::WorkMode, 12, 0);
	send(mySocket3, Order::WorkMode, 12, 0);
	send(mySocket4, Order::WorkMode, 12, 0);

	send(mySocket, Order::HardTouchStart, 12, 0);
	send(mySocket2, Order::HardTouchStart, 12, 0);
	send(mySocket3, Order::HardTouchStart, 12, 0);
	send(mySocket4, Order::HardTouchStart, 12, 0);
}

//多页对话框选择
void CXrays_64ChannelDlg::OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	m_currentTab = m_Tab.GetCurSel();
	switch (m_currentTab)
	{
	case 0:
		m_page1->ShowWindow(SW_SHOW);
		m_page2->ShowWindow(SW_HIDE);
		m_page1->UpdateData(FALSE);
		break;
	case 1:
		m_page1->ShowWindow(SW_HIDE);
		m_page2->ShowWindow(SW_SHOW);
		m_page2->UpdateData(FALSE);
		break;
	}
	*pResult = 0;
}

//清除当前日志（系统日志，UDP接受日志）
void CXrays_64ChannelDlg::OnBnClickedClearLog()
{
	// TODO: 在此添加控件通知处理程序代码
	CRect rc;
	m_Tab.GetClientRect(rc);
	rc.top += 20;
	switch (m_currentTab)
	{
	case 0:
		m_page1->m_Information = _T("");
		m_page1->UpdateData(FALSE);
		m_page1->MoveWindow(&rc);
		break;
	case 1:
		m_page2->m_Information = _T("");
		m_page2->m_Information2 = _T("");
		m_page2->UpdateData(FALSE);
		m_page2->MoveWindow(&rc);
		break;
	default:
		break;
	}
}
