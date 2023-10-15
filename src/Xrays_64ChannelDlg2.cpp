#include "json/json.h"

#include "pch.h"
#include "framework.h"
#include "Xrays_64Channel.h"
#include "Xrays_64ChannelDlg.h"
#include "PowerDlg.h"
#include "afxdialogex.h"
#include "Order.h"
#include "Log.h"
#include "afx.h"
// 该文件存放不常用函数，以及功能函数

// 文件操作
#include <iostream>
#include <fstream>
using namespace std;

// 设置TCP的IP、PORT、复选框的输入使能状态
void CXrays_64ChannelDlg::SetTCPInputStatus(BOOL flag)
{
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

	//发送刻度数据,只有联网后才能使用
	GetDlgItem(IDC_CALIBRATION)->EnableWindow(!flag);
}

//设置配置参数框的使能状态
void CXrays_64ChannelDlg::SetParameterInputStatus(BOOL flag)
{
	//能谱刷新时间
	GetDlgItem(IDC_RefreshTimeEdit)->EnableWindow(flag);
	//能谱测量时间
	GetDlgItem(IDC_MeasureTime)->EnableWindow(flag);
	//能谱模式选择
	GetDlgItem(IDC_WAVE_MODE)->EnableWindow(flag);
	//阈值设置
	GetDlgItem(IDC_CH1Threahold)->EnableWindow(flag);
}

// 打开UDP通信
void CXrays_64ChannelDlg::OpenUDP()
{
	CLog::WriteMsg(_T("尝试打开UDP！"));
	UpdateData(TRUE); //读取界面控件的输入值
	// 读取配置参数并设置到相应控件上
	Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
	//配置文件不存在，则生成配置文件
	jsonSetting["Port_UDP"] = m_UDPPort;
	WriteSetting(_T("Setting.json"), jsonSetting);

	//--------------1.创建UDPSocket------------
	if (!m_UDPSocket)
		delete m_UDPSocket;
	m_UDPSocket = new CUDP_Socket(this); //初始化,新创建一个对话框Socket
	m_UDPSocket->Create(m_UDPPort, SOCK_DGRAM, NULL);

	//--------------2.获取Socket绑定的ip和端口--------------
	//获取本机的IP和端口号
	CString strIp;
	UINT uiPort;

	//获取本地的服务号和端口号
	m_UDPSocket->GetSockName(strIp, uiPort);

	//显示本地的端口号和IP号
	// SetDlgItemText(IDC_UDPIP, strIp);
	SetDlgItemInt(IDC_UDPPORT, uiPort);
	CString info;
	info.Format(_T("UDP已打开，端口号为:%d"), uiPort);
	m_page1.PrintLog(info);
	m_page2.PrintLog(info);

	UDPStatus = TRUE;
	GetDlgItem(IDC_UDPPORT)->EnableWindow(FALSE);

	// 2、判断TCP连接状态
	BOOL AllconnectStatus = TRUE;
	for (int num = 0; num < 4; num++) {
		if(connectStatusList[num] != NetSwitchList[num+1]) AllconnectStatus = FALSE;
	}

	// UDP和TCP都打开后才允许使用自动测量
	if (AllconnectStatus)
	{
		GetDlgItem(IDC_AutoMeasure)->EnableWindow(TRUE);
	}
}

void CXrays_64ChannelDlg::CloseUDP()
{
	if (m_UDPSocket != NULL)
		delete m_UDPSocket;
	m_UDPSocket = NULL;
	CString info = _T("UDP网络已关闭");
	m_page1.PrintLog(info);
	m_page2.PrintLog(info);
	UDPStatus = FALSE;
	GetDlgItem(IDC_UDPPORT)->EnableWindow(TRUE);
	GetDlgItem(IDC_AutoMeasure)->EnableWindow(FALSE);
}

//保存TCP传输过来的测量数据
void CXrays_64ChannelDlg::SaveFile(CString myID, char *mk, int length)
{
	if (length < 1)
		return;
	CString filename = myID + _T(".dat");
	CString wholePath = saveAsTargetPath + filename;
	fstream datafile(wholePath, ios::out | ios::app | ios::binary); // 追加
	if (datafile.is_open())
	{
		datafile.write(mk, length);
		datafile.close();
	}
}

//限制TCP端口输入范围
void CXrays_64ChannelDlg::OnEnKillfocusPort1()
{
	ConfinePortRange(PortList[0]);
}

//限制TCP端口输入范围
void CXrays_64ChannelDlg::OnEnKillfocusPort2()
{
	ConfinePortRange(PortList[1]);
}

//限制TCP端口输入范围
void CXrays_64ChannelDlg::OnEnKillfocusPort3()
{
	ConfinePortRange(PortList[2]);
}

//限制TCP端口输入范围
void CXrays_64ChannelDlg::OnEnKillfocusPort4()
{
	ConfinePortRange(PortList[3]);
}

//限制UDP端口输入范围
void CXrays_64ChannelDlg::OnEnKillfocusUDPPort()
{
	// TODO: 在此添加控件通知处理程序代码
	ConfinePortRange(m_UDPPort);
}

// 重置TCP网口接收的缓存数据
void CXrays_64ChannelDlg::ResetTCPData()
{
	memset(DataCH1, 0, DataMaxlen);
	memset(DataCH2, 0, DataMaxlen);
	memset(DataCH3, 0, DataMaxlen);
	memset(DataCH4, 0, DataMaxlen);

	for(int num=0; num<4; num++){
		RECVLength[num] = 0;
	}
}

//限制端口号输入范围0~65535
void CXrays_64ChannelDlg::ConfinePortRange(int &myPort)
{
	UpdateData(true);
	if ((myPort < 0) || (myPort > 65535))
	{
		MessageBox(_T("端口的范围为0~65535\n"));
		if (myPort > 65535)
		{
			myPort = 65535;
		}
		else
		{
			myPort = 1;
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
	int MaxTime = 60 * 1000; //单位ms
	if ((RefreshTime < 10) || (RefreshTime > MaxTime))
	{
		CString message;
		message.Format(_T("能谱刷新时间范围为10~%dms\n"), MaxTime);
		MessageBox(message);
		if (RefreshTime > MaxTime)
		{
			RefreshTime = MaxTime;
		}
		else
		{
			RefreshTime = 10;
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
	if ((MeasureTime < 1) || (MeasureTime > MaxTime))
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
	if (myJson["SaveDir"].isString())
	{
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
		m_page1.PrintLog(info);
		UpdateData(FALSE);
	}
}

//配置参数,通过网口向控制板发送参数
void CXrays_64ChannelDlg::SendParameterToTCP()
{
	// TODO: 在此添加控件通知处理程序代码
	// 读取界面参数并修改指令
	UpdateData(TRUE);

	//从界面获取刷新时间
	char res[5];
	DecToHex(RefreshTime, res);
	Order::WaveRefreshTime[6] = res[0];
	Order::WaveRefreshTime[7] = res[1];
	Order::WaveRefreshTime[8] = res[2];
	Order::WaveRefreshTime[9] = res[3];

	//能谱刷新时间，波形触发间隔，波形触发阈值
	for (int num = 0; num < 4; num++) {
		if(connectStatusList[num]) {
			BackSend(num, Order::WaveRefreshTime, 12, 0, 1);
			BackSend(num, Order::TriggerThreshold, 12, 0, 1);
			BackSend(num, Order::TriggerIntervalTime, 12, 0, 1);
		}
	}
	
	CString info;
	if (m_WaveMode.GetCurSel() == 0)
	{ //512道能谱
		for (int num = 0; num < 4; num++) {
			if(connectStatusList[num]) {
				BackSend(num, Order::WorkMode0, 12, 0, 1);
			}
		}
		info.Format(_T("能谱刷新时间:%dms,512道能谱工作模式"), RefreshTime);
	}
	else if (m_WaveMode.GetCurSel() == 1)
	{ // 16道能谱
		for (int num = 0; num < 4; num++) {
			if(connectStatusList[num]) {
				BackSend(num, Order::WorkMode3, 12, 0, 1);
			}
		}
		info.Format(_T("能谱刷新时间:%dms,16道能谱工作模式"), RefreshTime);
	}
	m_page1.PrintLog(info);
}

//多页对话框选择
void CXrays_64ChannelDlg::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	m_currentTab = m_Tab.GetCurSel();
	switch (m_currentTab)
	{
	case 0:
		m_page1.ShowWindow(SW_SHOW);
		m_page2.ShowWindow(SW_HIDE);
		// m_page1.UpdateData(FALSE);
		break;
	case 1:
		m_page1.ShowWindow(SW_HIDE);
		m_page2.ShowWindow(SW_SHOW);
		// m_page2.UpdateData(FALSE);
		break;
		*pResult = 0;
	}
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
		m_page1.m_Information = _T("");
		m_page1.UpdateData(FALSE);
		// m_page1.MoveWindow(&rc);
		break;
	case 1:
		m_page2.m_Information = _T("");
		m_page2.UpdateData(FALSE);
		break;
	default:
		break;
	}
}

//缓存网口数据
void CXrays_64ChannelDlg::AddTCPData(int channel, char *tempChar, int len)
{
	/*for (int i = 0; i < len; i++) {
		if (RECVLength[channel-1] + i < DataMaxlen)
		{
			DataCH1[RECVLength[channel-1] + i] = tempChar[i];
		}
	}*/
	RECVLength[channel-1] += len;
}

//设置网口缓存区大小
void CXrays_64ChannelDlg::SetSocketSize(SOCKET &sock, int nsize)
{
	int nErrCode = 0;
	unsigned int uiRcvBuf = 0;
	unsigned int uiNewRcvBuf = 0;
	int uiRcvBufLen = sizeof(uiRcvBuf);
	nErrCode = getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&uiRcvBuf, &uiRcvBufLen);
	if (SOCKET_ERROR == nErrCode)
	{
		MessageBox(_T("获取服务端设置SOCKET发送缓冲区大小失败"));
		return;
	}
	// uiRcvBuf *= nsize;//设置系统发送数据为默认的倍数 uiRcvBuf=100kB
	uiRcvBuf = nsize;
	nErrCode = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&uiRcvBuf, uiRcvBufLen);
	if (SOCKET_ERROR == nErrCode)
	{
		MessageBox(_T("设置SOCKET发送缓冲区大小失败"));
		return;
	}
}

//界面缩放
void CXrays_64ChannelDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	//对状态栏位置重新布局
	ResizeBar();
	// 对界面控件重新布局
	m_layout.OnSize(cx, cy);

	if (m_page1.GetSafeHwnd() || m_page2.GetSafeHwnd())
	{
		CRect rctab;
		CRect reItem;
		int m_nWidth, m_nHeight;

		m_Tab.GetClientRect(&rctab);

		m_nWidth = rctab.right - rctab.left;
		m_nHeight = rctab.bottom - rctab.top;

		m_Tab.GetItemRect(0, &reItem);
		rctab.DeflateRect(0, reItem.bottom, 0, 0);

		m_page1.MoveWindow(&rctab);
		m_page2.MoveWindow(&rctab);
	}
}

//对界面控件重新布局
void CXrays_64ChannelDlg::ResizeBar()
{
	CRect rectDlg;
	GetClientRect(rectDlg); //获得窗体的大小
	//判断状态栏是否被创建
	if (IsWindow(m_statusBar.m_hWnd))
	{
		//设置面板序号，ID，样式和宽度，SBPS_NORMAL为普通样式，固定宽度，SBPS_STRETCH为弹簧样式，会自动扩展它的空间
		m_statusBar.SetPaneInfo(0, 1001, SBPS_NORMAL, int(0.6 * rectDlg.Width()));
		m_statusBar.SetPaneInfo(1, 1002, SBPS_STRETCH, int(0.2 * rectDlg.Width()));
		m_statusBar.SetPaneInfo(2, 1003, SBPS_NORMAL, int(0.2 * rectDlg.Width()));
		RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
	}
}

void CXrays_64ChannelDlg::OnSizing(UINT fwSide, LPRECT pRect)
{
	CDialogEx::OnSizing(fwSide, pRect);
	//TODO: 在此处添加消息处理程序代码
}

//带指令反馈的发送指令
BOOL CXrays_64ChannelDlg::BackSend(int num, BYTE *msg, int msgLength, int flags,
								 int sleepTime, int maxWaitingTime, BOOL isShow)
{
	if (ifFeedback[num])
		return FALSE;

	// 若超时未检测到反馈指令，则再次发送指令到FPGA。循环等待三次。
	for (int i = 0; i < 3; i++)
	{
		int times = 0; //等待时长
		CTime tm1;
		tm1 = CTime::GetCurrentTime();
		BOOL flag = FALSE;

		// 初始化反馈相关参数
		ifFeedback[num] = FALSE;
		TCPfeedback[num] = FALSE;
		LastSendMsg[num] = NULL;
		RecvMsg[num] = NULL;
		recievedFBLength[num] = 0;

		// 发送指令
		if (!ifFeedback[num])
		{
			ifFeedback[num] = TRUE;
			send(SocketList[num], (char *)msg, msgLength, flags);
			// Sleep(sleepTime);
			LastSendMsg[num] = (char *)msg;
			FeedbackLen[num] = msgLength;
			CString info;
			info.Format(_T("CH%d SEND HEX(%d):"), num+1, i+1);
			info = info + Char2HexCString(LastSendMsg[num], msgLength);
			m_page1.PrintLog(info, isShow); 
		}

		// 阻塞式判断等待反馈指令，并进行判断是否与发送指令相同
		do
		{ 
			// 判断接收指令与发送指令是否相同
			if (recievedFBLength[num] == msgLength)
			{
				CString info;
				info.Format(_T("CH%d RECV HEX:"), num+1);
				info += Char2HexCString(RecvMsg[num], recievedFBLength[num]);
				m_page1.PrintLog(info, isShow);
				if (strncmp(RecvMsg[num], LastSendMsg[num], msgLength) == 0){
					TCPfeedback[num] = TRUE;
				}
				if (!TCPfeedback[num]) {
					RecvMsg[num] = NULL;
					recievedFBLength[num] = 0;
				}
			}

			if (TCPfeedback[num])
			{
				CString info;
				info.Format(_T("CH%d指令反馈校验成功:"), num+1);
				info += Char2HexCString(RecvMsg[num], recievedFBLength[num]);
				m_page1.PrintLog(info, isShow);

				//接收到反馈指令，重新初始化反馈相关数据
				TCPfeedback[num] = FALSE;
				ifFeedback[num] = FALSE;
				LastSendMsg[num] = NULL;
				RecvMsg[num] = NULL;
				recievedFBLength[num] = 0;

				return TRUE;
			}

			// 计算等待时长
			CTime tm2;
			tm2 = CTime::GetCurrentTime();
			CTimeSpan span;
			span = tm2 - tm1;
			times = span.GetSeconds();
		} while (times < maxWaitingTime); 

		CString info;
		info.Format(_T("CH%d 等待指令反馈时间%ds,超出最大设置时长%ds,"), num+1, times, maxWaitingTime);
		m_page1.PrintLog(info, isShow);
	}

	CString info;
	info.Format(_T("CH%d 尝试3次下发指令，均无法接受到反馈指令，SEND HEX: "), num+1);
	info = info + Char2HexCString(LastSendMsg[num], msgLength);
	m_page1.PrintLog(info, TRUE);

	// 恢复指令反馈相关参数
	ifFeedback[num] = FALSE;
	TCPfeedback[num] = FALSE;
	LastSendMsg[num] = NULL;
	RecvMsg[num] = NULL;
	recievedFBLength[num] = 0;

	return FALSE;
}

//不带指令反馈的发送指令
void CXrays_64ChannelDlg::NoBackSend(int num, BYTE* msg, int msgLength, int flags,
	int sleepTime){
	send(SocketList[num], (char*)msg, msgLength, flags);
	CString info;
	info.Format(_T("CH%d SEND HEX :"), num + 1);
	info = info + Char2HexCString((char*)msg, msgLength);
	Sleep(sleepTime);
}

void CXrays_64ChannelDlg::OnBnClickedCheck0()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);//从控件获得数据   获得输入数据后可以进行相应操作
	if (BST_CHECKED == IsDlgButtonChecked(IDC_CHECK1)) //BST_CHECKED：表示按钮被选中。BST_UNCHECKED：表示该按钮未选中（unckecked）。
	{
		for (int i = 0; i < 5; i++) {
			NetSwitchList[i] = TRUE;
		}
	}
	else{
		for (int i = 0; i < 5; i++) {
			NetSwitchList[i] = FALSE;
		}
	}
	UpdateData(FALSE);//刷新控件
}

void CXrays_64ChannelDlg::OnBnClickedCheck1()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);//从控件获得数据   获得输入数据后可以进行相应操作
	NetSwitchList[0] = NetSwitchList[1] & NetSwitchList[2] & NetSwitchList[3] & NetSwitchList[4];
	UpdateData(FALSE);//刷新控件
}

void CXrays_64ChannelDlg::OnBnClickedCheck2()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);//从控件获得数据   获得输入数据后可以进行相应操作
	NetSwitchList[0] = NetSwitchList[1] & NetSwitchList[2] & NetSwitchList[3] & NetSwitchList[4];
	UpdateData(FALSE);//刷新控件
}

void CXrays_64ChannelDlg::OnBnClickedCheck3()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);//从控件获得数据   获得输入数据后可以进行相应操作
	NetSwitchList[0] = NetSwitchList[1] & NetSwitchList[2] & NetSwitchList[3] & NetSwitchList[4];
	UpdateData(FALSE);//刷新控件
}

void CXrays_64ChannelDlg::OnBnClickedCheck4()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);//从控件获得数据   获得输入数据后可以进行相应操作
	NetSwitchList[0] = NetSwitchList[1] & NetSwitchList[2] & NetSwitchList[3] & NetSwitchList[4];
	UpdateData(FALSE);//刷新控件
}