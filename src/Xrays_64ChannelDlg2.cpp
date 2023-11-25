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

#include<iomanip>
// 该文件存放不常用函数，以及功能函数

// 文件操作
#include <iostream>
#include <fstream>
using namespace std;
extern const int TIMER_INTERVAL;
//CMutex Mutex; //mutex，线程锁

// 设置TCP的IP、PORT、复选框的输入使能状态
void CXrays_64ChannelDlg::SetTCPInputStatus(BOOL flag)
{
	//网络复选框
	GetDlgItem(IDC_ALL_CHECK)->EnableWindow(flag);
	GetDlgItem(IDC_CHECK1)->EnableWindow(flag);
	GetDlgItem(IDC_CHECK2)->EnableWindow(flag);
	GetDlgItem(IDC_CHECK3)->EnableWindow(flag);

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
	GetDlgItem(IDC_CH1Threshold)->EnableWindow(flag);
}

// 打开UDP通信
void CXrays_64ChannelDlg::OpenUDP()
{
	CLog::WriteMsg(_T("尝试打开UDP！"));
	UpdateData(TRUE); //读取界面控件的输入值
	// 读取配置参数并设置到相应控件上
	Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
	//配置文件不存在，则生成配置文件
	int m_UDPPort;
	if(jsonSetting.isNull())
	{
		if(jsonSetting.isMember("Port_UDP"))
		{
			m_UDPPort = jsonSetting["Port_UDP"].asInt();
		}
		else{
			MessageBox(_T("无法再配置文件中找到‘Port_UDP’,系统默认将Port_UDP设置为12100"));
			m_UDPPort = 12100;
			jsonSetting["Port_UDP"] = m_UDPPort;
		}
	}
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
	CString info;
	info.Format(_T("UDP已打开，端口号为:%d"), uiPort);
	m_page1.PrintLog(info);
	m_page2.PrintLog(info);

	UDPStatus = TRUE;

	// 2、判断TCP连接状态
	BOOL AllconnectStatus = TRUE;
	for (int num = 0; num < 3; num++) {
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
	GetDlgItem(IDC_AutoMeasure)->EnableWindow(FALSE);
}

//保存字符串数组数据
void CXrays_64ChannelDlg::SaveFile(CString fileName, char *mk, int length)
{
	if (length < 1)
		return;
	fstream datafile(fileName, ios::out | ios::app); // 追加  | ios::binary
	if (datafile.is_open())
	{
		datafile.write(mk, length);
		datafile.close();
	}
}

//保存ARM监测的温度数据
void CXrays_64ChannelDlg::SaveEnviromentFile(double data[])
{	
	CTime ct = CTime::GetCurrentTime();
	CString filename = ct.Format(_T("Temperatue_%Y%m%d.dat"));
	CString strPart_Time = ct.Format(_T("%Y-%m-%d %H:%M:%S"));

	CString parentPath = _T("Enviroment\\");
	CString wholePath = parentPath + filename;
	if (!IsPathExit(parentPath)) Mkdir(parentPath);
	if (!IsFileExit(wholePath)) {
		//首次创建文件的时候产生表头
		fstream datafile(wholePath, ios::out | ios::app| ios::left); // 追加
		if (datafile.is_open())
		{
			datafile << setw(25) << "Time";
			datafile << setw(10)<< "Temp1(℃)";
			datafile << setw(10) << "Temp2(℃)";
			datafile << setw(10) << "Temp3(℃)";
			datafile << setw(10) << "Temp4(℃)";
			datafile << setw(10) << "Temp5(℃)";
			datafile << setw(10) << "Temp6(℃)";
			datafile << setw(10) << "Volt(V)";
			datafile << setw(10) << "I(A)";
			datafile << endl;
			datafile.close();
		}
	}

	fstream datafile(wholePath, ios::out | ios::app | ios::left); // 追加
	datafile.setf(ios::fixed, ios::floatfield);  // 设定为 fixed 模式，以小数点表示浮点数
	datafile.precision(2);  // 设置精度 2
	//datafile.width(25);
	if (datafile.is_open())
	{
		datafile << setw(25) << _UnicodeToUtf8(strPart_Time);
		
		for(int i=0; i<8; i++){
			if(abs(data[i] - 6553.5)<0.01) {
				datafile << setw(10) << "--";
			}
			else{
				datafile << setw(10) << data[i];
			}
		}
		datafile << endl;
		datafile.close();
	}
}

// 重置TCP网口接收的缓存数据
void CXrays_64ChannelDlg::ResetTCPData()
{
	memset(DataCH1, 0, DataMaxlen);
	memset(DataCH2, 0, DataMaxlen);
	memset(DataCH3, 0, DataMaxlen);

	for(int num=0; num < 3; num++){
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

//选择能谱模式：512道/16道 能谱
//更新相关的能谱刷新时间检查
void CXrays_64ChannelDlg::OnCbnSelchangeWaveMode()
{
	UpdateData(true);
	// 保存参数设置
	Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
	int mode = m_WaveMode.GetCurSel();
	jsonSetting["WaveMode"] = m_WaveMode.GetCurSel();
	WriteSetting(_T("Setting.json"), jsonSetting);

	//检查相应的刷新时间，限定范围
	OnEnKillfocusRefreshTime();
}

//限制能谱刷新时间范围,单位ms，
//能谱刷新时间指的是FPGA采集一个能谱数据所用时间
void CXrays_64ChannelDlg::OnEnKillfocusRefreshTime()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(true);
	int minTime = 1; //单位ms
	int waveMode = 16; //能谱模式，512/16道两种
	// 512道能谱，最小刷新时间10ms，16道能谱，最小刷新时间1ms
	if (m_WaveMode.GetCurSel() == 0) {
		minTime = 10; 
		waveMode = 512;
	}
	int MaxTime = 60000 * 1000; //单位ms
	if ((RefreshTime < minTime) || (RefreshTime > MaxTime))
	{
		CString message;
		message.Format(_T("%d能谱模式下，能谱刷新时间范围为%d~%dms\n"), waveMode, minTime, MaxTime);
		MessageBox(message);
		if (RefreshTime > MaxTime)
		{
			RefreshTime = MaxTime;
		}
		else
		{
			RefreshTime = minTime;
		}
		UpdateData(false);
	}
}

//限制能谱测量时间范围，单位ms
void CXrays_64ChannelDlg::OnEnKillfocusMeasureTime()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(true);
	int MaxTime = 60000 * 1000; //单位ms
	if ((MeasureTime < 1) || (MeasureTime > MaxTime))
	{
		CString message;
		message.Format(_T("能谱测量时间范围为1~%dms\n"), MaxTime);
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

//限制触发阈值
void CXrays_64ChannelDlg::OnEnKillfocusThreshold()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(true);
	if ((m_Threshold < 1) || (m_Threshold > 2048))
	{
		CString message = _T("触发阈值范围为1~2048\n");
		MessageBox(message);
		if (m_Threshold > 2048)
		{
			m_Threshold = 2048;
		}
		else
		{
			m_Threshold = 1;
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
	BYTE res[4];
	DecToHex(RefreshTime, res);
	Order::WaveRefreshTime[6] = res[0];
	Order::WaveRefreshTime[7] = res[1];
	Order::WaveRefreshTime[8] = res[2];
	Order::WaveRefreshTime[9] = res[3];
	
	BYTE res2[4];
	DecToHex(m_Threshold, res2);
	Order::TriggerThreshold[6] = res2[0];
	Order::TriggerThreshold[7] = res2[1];
	Order::TriggerThreshold[8] = res2[2];
	Order::TriggerThreshold[9] = res2[3];

	//能谱刷新时间，波形触发间隔，波形触发阈值
	for (int num = 0; num < 3; num++) {
		if(connectStatusList[num]) {
			BackSend(num, Order::WaveRefreshTime, 12, 0, 1);
			BackSend(num, Order::TriggerThreshold, 12, 0, 1);
			BackSend(num, Order::TriggerIntervalTime, 12, 0, 1);
		}
	}
	
	CString info;
	if (m_WaveMode.GetCurSel() == 0)
	{ //512道能谱
		for (int num = 0; num < 3; num++) {
			if(connectStatusList[num]) {
				BackSend(num, Order::WorkMode0, 12, 0, 1);
			}
		}
		info.Format(_T("能谱刷新时间:%dms,512道能谱工作模式"), RefreshTime);
	}
	else if (m_WaveMode.GetCurSel() == 1)
	{ // 16道能谱
		for (int num = 0; num < 3; num++) {
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
void CXrays_64ChannelDlg::AddTCPData(int num, BYTE *tempChar, int len)
{
	/*for (int i = 0; i < len; i++) {
		if (RECVLength[num] + i < DataMaxlen)
		{
			DataCH1[RECVLength[num] + i] = tempChar[i];
		}
	}*/
	RECVLength[num] += len;
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
	CSingleLock singleLock(&Mutex); //线程锁

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

		singleLock.Lock(); //Mutex
		if (singleLock.IsLocked()){
			ifFeedback[num] = FALSE;
		}
		singleLock.Unlock(); //Mutex

		TCPfeedback[num] = FALSE;
		LastSendMsg[num] = NULL;
		RecvMsg[num] = NULL;
		recievedFBLength[num] = 0;

		// 发送指令
		if (!ifFeedback[num])
		{
			singleLock.Lock(); //线程锁
			if (singleLock.IsLocked()){
				ifFeedback[num] = TRUE;
			}
			singleLock.Unlock(); 

			send(SocketList[num], (char *)msg, msgLength, flags);
			// Sleep(sleepTime);
			LastSendMsg[num] = msg;
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
				if (compareBYTE(RecvMsg[num], LastSendMsg[num], msgLength)){
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
				
				singleLock.Lock(); //线程锁
				if (singleLock.IsLocked()){
					ifFeedback[num] = FALSE;
				}
				singleLock.Unlock(); 

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
	singleLock.Lock(); //线程锁
	if (singleLock.IsLocked()){
		ifFeedback[num] = FALSE;
	}
	singleLock.Unlock();
	
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
	info = info + Char2HexCString(msg, msgLength);
	m_page1.PrintLog(info, FALSE);
	Sleep(sleepTime);
}

//网络勾选，全选
void CXrays_64ChannelDlg::OnBnClickedCheck0()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);//从控件获得数据   获得输入数据后可以进行相应操作
	if (BST_CHECKED == IsDlgButtonChecked(IDC_ALL_CHECK)) //BST_CHECKED：表示按钮被选中。BST_UNCHECKED：表示该按钮未选中（unckecked）。
	{
		for (int i = 0; i < 4; i++) {
			NetSwitchList[i] = TRUE;
		}
	}
	else{
		for (int i = 0; i < 4; i++) {
			NetSwitchList[i] = FALSE;
		}
	}
	UpdateData(FALSE);//刷新控件
}

//网络勾选，设备1
void CXrays_64ChannelDlg::OnBnClickedCheck1()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);//从控件获得数据   获得输入数据后可以进行相应操作
	NetSwitchList[0] = NetSwitchList[1] & NetSwitchList[2] & NetSwitchList[3];
	UpdateData(FALSE);//刷新控件
}

//网络勾选，设备2
void CXrays_64ChannelDlg::OnBnClickedCheck2()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);//从控件获得数据   获得输入数据后可以进行相应操作
	NetSwitchList[0] = NetSwitchList[1] & NetSwitchList[2] & NetSwitchList[3];
	UpdateData(FALSE);//刷新控件
}

//网络勾选，设备3
void CXrays_64ChannelDlg::OnBnClickedCheck3()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);//从控件获得数据   获得输入数据后可以进行相应操作
	NetSwitchList[0] = NetSwitchList[1] & NetSwitchList[2] & NetSwitchList[3];
	UpdateData(FALSE);//刷新控件
}

LRESULT CXrays_64ChannelDlg::OnUpdateTrigerLog(WPARAM wParam, LPARAM lParam){
	int num = (int) wParam;
	CString info;
	info.Format(_T("CH%d已收到硬件触发信号"), num+1);
	m_page1.PrintLog(info,FALSE);
	return 0;
}

LRESULT CXrays_64ChannelDlg::OnUpdateTimer1(WPARAM wParam, LPARAM lParam){
	int num = (int)wParam;
	if (m_nTimerId[0]==0) {
		m_nTimerId[0] = SetTimer(1, TIMER_INTERVAL, NULL); //如果已经开启则不再重复开启

		CString info;
		info.Format(_T("CH%d开启2号定时器"), num + 1);
		m_page1.PrintLog(info, FALSE);
	}
	return 0;
}

LRESULT CXrays_64ChannelDlg::OnUpdateShot(WPARAM wParam, LPARAM lParam){
	int nLength = (int)wParam;
	char* recBuf = (char*)lParam;
	CString tempStr;
	tempStr.Format(_T("%S"), recBuf);
	m_page2.PrintLog(_T("RECV ASCII: ") + tempStr);

	if (recBuf[0] == '+' && recBuf[1] == 'P' && recBuf[2] == 'L' && recBuf[3] == 'S') {
		CString str_new_ID;
		str_new_ID = CString(recBuf[5]) + CString(recBuf[6]) + CString(recBuf[7]) 
					+ CString(recBuf[8]) + CString(recBuf[9]);
		if (str_new_ID.Compare(m_targetID)) { //不相同
			m_targetID = str_new_ID;
			m_getTargetChange = TRUE;
			recBuf[nLength] = (char)'\n'; // 追加一个换行符号
			int saveLength = nLength + 1;
			
			SaveFile(saveAsPath + _T("ShotNumber.dat"), recBuf, saveLength);

			CString info = _T("炮号已刷新：") + m_targetID;
			m_page1.PrintLog(info);
			UpdateData(FALSE);

			// 打开定时器2，开始新的一次测量
			m_nTimerId[1] = SetTimer(2, TIMER_INTERVAL, NULL);
		}
	}
	return 0;
}

BOOL CXrays_64ChannelDlg::DestroyWindow()
{
	// TODO: 在此添加专用代码和/或调用基类
	CDialogEx::OnOK();

	return CDialogEx::DestroyWindow();
}


void CXrays_64ChannelDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	BOOL AllconnectStatus = TRUE;
	// 判断探测器网络连接状态，勾选的设备是否都是连接
	for (int num = 0; num < 3; num++) {
		if (connectStatusList[num] != NetSwitchList[num + 1]) AllconnectStatus = FALSE;
	}

	// 若ARM或者探测器处于联网状态则提醒用户是否退出
	if (AllconnectStatus || ARMnetStatus) {
		int choose = MessageBox(L"是否关闭窗口？", L"提示", MB_YESNO | MB_ICONQUESTION);
		if (choose == IDYES)
		{
			CDialog::OnClose();
		}
	}
	else {
		CDialog::OnClose();
	}
}