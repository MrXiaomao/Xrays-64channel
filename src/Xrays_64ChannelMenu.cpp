#include "pch.h"
#include "Xrays_64Channel.h"
#include "Xrays_64ChannelDlg.h"
#include "PowerDlg.h"
#include "NetSettingDlg.h"
#include "Order.h"

void CXrays_64ChannelDlg::OnBnClickedRelayConnect()
{
	// CPowerDlg powerdlg; // 创建一个模态对话框 
	// powerdlg.DoModal(); // 显示模态对话框 其中参数用swp_SHOWNOMAL,  SW_SHOW, SW_VISION 好像效果是一样的
	GetDlgItem(IDC_POWER_NET)->EnableWindow(FALSE); //禁用,防止用户重复点击

	CString strTemp;
	GetDlgItemText(IDC_POWER_NET, strTemp);
	if (strTemp == _T("电源网络连接")) {
		netRelayStatus = ConnectRelayTCP();
		if (netRelayStatus) {
			// 开启线程接收数据
			AfxBeginThread(&Recv_Relay, this);
			SetDlgItemText(IDC_POWER_NET, _T("电源网络断开"));
			m_RelayNetStatusLED.RefreshWindow(1, _T("已连接"));//打开指示灯
			GetDlgItem(IDC_POWER_ONOFF)->EnableWindow(TRUE); //允许使用
			CString info = _T("电源网络（继电器）已连接");
			m_page1.PrintLog(info);
		}
		else {
			m_RelayNetStatusLED.RefreshWindow(0, _T("连接失败"));//指示灯
			CString info = _T("电源网络（继电器）连接失败，请检查网络IP和端口号是否正确");
			m_page1.PrintLog(info);
		}
	}
	else {
		netRelayStatus = FALSE; // 用来控制关闭线程
		closesocket(relaySocket); // 关闭套接字
		SetDlgItemText(IDC_POWER_NET, _T("电源网络连接"));
		m_RelayNetStatusLED.RefreshWindow(0, _T("已断开"));//指示灯
		m_RelayStatusLED.RefreshWindow(2, _T("Unknow"));
		GetDlgItem(IDC_POWER_ONOFF)->EnableWindow(FALSE); //禁用
		CString info = _T("电源网络（继电器）已断开");
		m_page1.PrintLog(info);
	}
	GetDlgItem(IDC_POWER_NET)->EnableWindow(TRUE); //恢复使用
}

BOOL CXrays_64ChannelDlg::ConnectRelayTCP() {
	// 1、创建套接字
	relaySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (relaySocket == INVALID_SOCKET) {
		CString info = _T("继电器网络初始化创建失败！");
		m_page1.PrintLog(info);
		return FALSE;
	}

	// 2、连接服务器
	CString StrSerIp = _T("192.168.10.22"); //默认值
	int RelayPort = 1030; //默认值

	// 读取配置文件
	Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
	if(!jsonSetting.isNull()){
		if(jsonSetting.isMember("IP_Relay")) {
			StrSerIp = jsonSetting["IP_Relay"].asCString();
		}
		else{
			char* pStrIP = CstringToWideCharArry(StrSerIp);
			jsonSetting["IP_Relay"] = pStrIP;
			CString info = _T("配置文件中未查找到继电器网址IP，即关键字\"IP_Relay\",采用默认值：");
			info += StrSerIp;
			m_page1.PrintLog(info);
		}
		if(jsonSetting.isMember("Port_Relay")) {
			RelayPort = jsonSetting["Port_Relay"].asInt();
		}
		else{
			jsonSetting["Port_Relay"] = RelayPort;
			CString info;
			info.Format(_T("配置文件中未查找到继电器网址端口号，即关键字\"Port_Relay\",采用默认值：%d"),RelayPort);
			m_page1.PrintLog(info);
		}
	}
	char* pStrIP = CstringToWideCharArry(StrSerIp);
	// 可能发生改动，保存到配置文件
	WriteSetting(_T("Setting.json"), jsonSetting);

	// 3、设置网络参数
	sockaddr_in server_addr;
	inet_pton(AF_INET, pStrIP, (void*)&server_addr.sin_addr.S_un.S_addr);
	server_addr.sin_family = AF_INET;  // 使用IPv4地址
	server_addr.sin_port = htons(RelayPort); //网关：5000

	// 4、检测网络是否连接,以及显示设备联网状况
	if (connect(relaySocket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
		return FALSE;
	}
	return TRUE;
}

//接收继电器TCP网口数据
UINT Recv_Relay(LPVOID p)
{
	CXrays_64ChannelDlg* dlg = (CXrays_64ChannelDlg*)p;
	while (1)
	{
		// 断开网络后关闭本线程
		if (!dlg->netRelayStatus) return 0;
		const int dataLen = 10; //接收的数据包长度
		BYTE mk[dataLen];
		int nLength;
		nLength = recv(dlg->relaySocket, (char*)mk, dataLen, 0); //阻塞模式

		if (nLength == -1)
		{
			return 0;
		}
		else {
			::PostMessage(dlg->m_hWnd, WM_UPDATE_RELAY, nLength, (LPARAM)(BYTE*)mk); //发送消息通知主界面处理，更新继电器状态
		}
	}
}

void CXrays_64ChannelDlg::OnRelayChange() {
	GetDlgItem(IDC_POWER_ONOFF)->EnableWindow(FALSE); //禁用
	CString strTemp;
	GetDlgItemText(IDC_POWER_ONOFF, strTemp);
	if (strTemp == _T("电源开启")) {
		send(relaySocket, (char*)Order::relay_ON, 10, 0);
		SetDlgItemText(IDC_POWER_ONOFF, _T("电源关闭"));
	}
	else {
		send(relaySocket, (char*)Order::relay_OFF, 10, 0);
		SetDlgItemText(IDC_POWER_ONOFF, _T("电源开启"));
	}
	GetDlgItem(IDC_POWER_ONOFF)->EnableWindow(TRUE); //恢复使用
}

void CXrays_64ChannelDlg::OnPowerMenu()
{
	// TODO: 在此添加命令处理程序代码
	CPowerDlg powerdlg; // 创建一个模态对话框 
	powerdlg.DoModal(); // 显示模态对话框 其中参数用swp_SHOWNOMAL,  SW_SHOW, SW_VISION 好像效果是一样的
}

void CXrays_64ChannelDlg::OnNetSettingMenu()
{
	// TODO: 在此添加命令处理程序代码
	CNetSetting netsetdlg; // 创建一个模态对话框
	netsetdlg.DoModal(); // 显示模态对话框 其中参数用swp_SHOWNOMAL,  SW_SHOW, SW_VISION 好像效果是一样的
}

BOOL CXrays_64ChannelDlg::TempVoltMonitorON() 
{
	//从配置文件获取IP、Port
	Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
	CString StrIP_ARM = _T("192.168.10.22");
	int portARM = 1000;
	if (jsonSetting.isMember("IP_ARM"))
	{
		StrIP_ARM = jsonSetting["IP_ARM"].asCString();
	}
	if (jsonSetting.isMember("Port_ARM"))
	{
		portARM = jsonSetting["Port_ARM"].asInt();
	}

	CString info;
	if (ConnectGeneralTCP(armSocket, StrIP_ARM, portARM)) {
		// 连接成功
		// 开启线程接收数据
		m_pThread_ARM = AfxBeginThread(&Recv_ARM, this);

		//更新相关按钮以及状态
		ARMnetStatus = TRUE;
		
		info = _T("温度/电压/电流监测已开启");
		m_page1.PrintLog(info);
		
		//首次连接上时直接查询一次温度/电压/电流
		send(armSocket, (char*)Order::ARM_Temperature1, 12, 0);
		return TRUE;
	}
	else {
		info = _T("无法连接温度/电压/电流监测网络");
		m_page1.PrintLog(info);
		return FALSE;
	}
}

void CXrays_64ChannelDlg::TempVoltMonitorOFF() 
{
	ARMnetStatus = FALSE; //关联着线程，先关闭线程，再关闭套接字
	closesocket(armSocket); // 关闭套接字
	CString info;
	info = _T("温度/电压/电流监测已关闭");
	m_page1.PrintLog(info);
}

UINT Recv_ARM(LPVOID p) // 多线程接收ARM网口数据
{
	CXrays_64ChannelDlg* dlg = (CXrays_64ChannelDlg*)p;
	while (1)
	{
		// 断开网络后关闭本线程
		if (!dlg->ARMnetStatus) return 0;
		const int dataLen = 20; //接收的数据包长度
		BYTE mk[dataLen];
		int nLength;
		nLength = recv(dlg->armSocket, (char*)mk, dataLen, 0); //阻塞模式

		if (nLength == -1)
		{
			return 0;
		}
		else {
			// 转化为CByteArray
			CByteArray buffer;
			for (int i = 0; i < nLength; i++) {
				buffer.Add(mk[i]);
			}
			dlg->TotalARMArray.Append(buffer); //拼接
			::PostMessage(dlg->m_hWnd, WM_UPDATE_ARM, 0, 0); //发送消息通知主界面处理，更新温度、电压、电流
		}
	}
}


BOOL CXrays_64ChannelDlg::ConnectGeneralTCP(SOCKET &my_socket, CString strIP, int port) {
	// 1、创建套接字
	my_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (my_socket == INVALID_SOCKET) {
		// 设备网络初始化创建失败
		return FALSE;
	}

	// 2、连接服务器
	char* pStrIP = CstringToWideCharArry(strIP);

	// 3、设置网络参数
	sockaddr_in server_addr;
	inet_pton(AF_INET, pStrIP, (void*)&server_addr.sin_addr.S_un.S_addr);
	server_addr.sin_family = AF_INET;  // 使用IPv4地址
	server_addr.sin_port = htons(port); //网关

	// 4、检测网络是否连接,以及显示设备联网状况
	if (connect(my_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
		return FALSE;
	}
	return TRUE;
}

//读取ARM消息，6个温度、电压、电流
LRESULT CXrays_64ChannelDlg::OnUpdateARMStatic(WPARAM wParam, LPARAM lParam){
	GetARMData();
	return 0;
}

void CXrays_64ChannelDlg::GetARMData() {
	//包头包尾判断
	BYTE head[2] = { 0xAA, 0xBB };
	BYTE tail[2] = { 0xCC, 0xDD };
	int headLen = 2;
	int packageLen = 26;
	CByteArray outArray;
	BOOL foundPackage = GetOnePackage(TotalARMArray, outArray, head, tail, headLen, packageLen);

	if(foundPackage)
	{
		powerVolt = ((outArray[3] & 0xFF) * 256.0 + (outArray[4] & 0xFF)) / 100.0;
		powerCurrent = ((outArray[5] & 0xFF) * 256.0 + (outArray[6] & 0xFF)) /1000.0;

		temperature[0] = ((outArray[8] & 0xFF) * 256.0 + (outArray[9] & 0xFF)) / 10.0;
		temperature[1] = ((outArray[10] & 0xFF) * 256.0 + (outArray[11] & 0xFF)) / 10.0;
		temperature[2] = ((outArray[12] & 0xFF) * 256.0 + (outArray[13] & 0xFF)) / 10.0;
		
		temperature[3] = ((outArray[15] & 0xFF) * 256.0 + (outArray[16] & 0xFF)) / 10.0;
		temperature[4] = ((outArray[17] & 0xFF) * 256.0 + (outArray[18] & 0xFF)) / 10.0;
		temperature[5] = ((outArray[19] & 0xFF) * 256.0 + (outArray[20] & 0xFF)) / 10.0;

		refreshBar();
	}
}


//刷新状态栏
void CXrays_64ChannelDlg::refreshBar() {
	//温度监控设备1和2
	CString strInfo1;
	//出现数值6553.5，则是温度传感器不存在，或者接触异常（如松动）等原因。
	for(int i=0; i<6; i++){
		CString tempStr;
		if(abs(temperature[i] - 6553.5)<0.01){
			tempStr = _T("--℃,");
		}
		else{
			tempStr.Format(_T("%.2f℃,"),temperature[i]);
		}
		strInfo1 += tempStr;
	}

	//电压电流监控设备
	CString strInfo2;
	if(abs(powerVolt - 6553.5)<0.01 || abs(powerCurrent - 6553.5)<0.01){
		strInfo2 = _T("--V,--A");
	}
	else {
		strInfo2.Format(_T("Volt:%.2fV,Current:%.2fA"), powerVolt, powerCurrent);
	}

	// CString strInfo = strInfo1 + strInfo2;
	CString strInfo = strInfo2;
	m_statusBar.SetPaneText(1, strInfo);
	m_page1.PrintLog(_T("温度、电压、电流采集数据：") + strInfo,FALSE);

	// 保存数据到文件
	double data[8] = { temperature[0], temperature[1], temperature[2], temperature[3], temperature[4], temperature[5], 
						powerVolt, powerCurrent};
	SaveEnviromentFile(data);

	// 重置温度/电压/电流查询反馈状态
	for (int i = 0; i < 3; i++) {
		feedbackARM[i] = FALSE;
	}
}