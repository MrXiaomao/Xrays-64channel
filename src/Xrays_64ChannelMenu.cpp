#include "pch.h"
#include "Xrays_64Channel.h"
#include "Xrays_64ChannelDlg.h"
#include "PowerDlg.h"
#include "NetSettingDlg.h"
#include "Order.h"

void CXrays_64ChannelDlg::OnBnClickedPowerButton()
{
	// TODO: 在此添加控件通知处理程序代码
	CPowerDlg powerdlg; // 创建一个模态对话框 
	powerdlg.DoModal(); // 显示模态对话框 其中参数用swp_SHOWNOMAL,  SW_SHOW, SW_VISION 好像效果是一样的
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

void CXrays_64ChannelDlg::TempVoltMonitorON_OFF() 
{
	GetDlgItem(IDC_TEMP_VOLT)->EnableWindow(FALSE); //禁用
	CString strTemp;
	GetDlgItemText(IDC_TEMP_VOLT, strTemp);
	if (strTemp == _T("开启")) {

		//从配置文件获取IP、Port
		Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
		CString StrIP_ARM = _T("192.168.10.22");
		int portARM = 1000;
		StrIP_ARM = jsonSetting["IP_ARM"].asCString();
		portARM = jsonSetting["Port_ARM"].asInt();

		CString info;
		if (ConnectGeneralTCP(armSocket, StrIP_ARM, portARM)) {
			// 连接成功
			// 开启线程接收数据
			AfxBeginThread(&Recv_ARM, this);

			//更新相关按钮以及状态
			ARMnetStatus = TRUE;
			SetDlgItemText(IDC_TEMP_VOLT, _T("关闭")); 
			m_AMR_LED.RefreshWindow(TRUE, _T("ON"));
			
			info = _T("温度/电压/电流监测已开启");
			m_page1.PrintLog(info);
			
			//首次连接上时直接查询一次温度/电压/电流
			send(armSocket, (char*)Order::ARM_Temperature, 12, 0);
			Sleep(1);//是否有必要
			send(armSocket, (char*)Order::ARM_VoltCurrent, 12, 0);
		}
		else {
			info = _T("无法连接温度/电压/电流监测网络");
			m_page1.PrintLog(info);
		}
	}
	else {
		ARMnetStatus = FALSE; //关联着线程，先关闭线程，再关闭套接字
		closesocket(armSocket); // 关闭套接字
		SetDlgItemText(IDC_TEMP_VOLT, _T("开启"));
		m_AMR_LED.RefreshWindow(FALSE, _T("OFF"));
		CString info;
		info = _T("温度/电压/电流监测已关闭");
		m_page1.PrintLog(info);
	}
	GetDlgItem(IDC_TEMP_VOLT)->EnableWindow(TRUE); //恢复使用
}


UINT Recv_ARM(LPVOID p) // 多线程接收ARM网口数据
{
	CXrays_64ChannelDlg* dlg = (CXrays_64ChannelDlg*)p;
	while (1)
	{
		// 断开网络后关闭本线程
		if (!dlg->ARMnetStatus) return 0;
		const int dataLen = 20; //接收的数据包长度
		char mk[dataLen];
		int nLength;
		nLength = recv(dlg->armSocket, mk, dataLen, 0); //阻塞模式

		if (nLength == -1)
		{
			return 0;
		}
		else {
			// 转化为CByteArray
			CByteArray buffer;
			for (int i = 0; i < nLength; i++) {
				buffer.Add((BYTE)mk[i]);
			}
			dlg->TotalARMArray.Append(buffer); //拼接

			dlg->GetTemperature(); //尝试解析温度
			dlg->GetVoltCurrent(); //尝试解析电压电流
			dlg->refreshBar(); //刷新界面温度、电压、电流显示
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

void CXrays_64ChannelDlg::GetTemperature() {

	int StandardPackLength = 11;
	if (TotalARMArray.GetSize() >= StandardPackLength)
	{
		// 一个包11个字节
		int StandardPackLength = 11;
		//包头包尾判断
		BYTE head[2] = { 0xAA, 0xBB };
		BYTE tail[2] = { 0xCC, 0xDD };
		//----------------------------------寻找包头包尾---------------------------------//
		int HeadIndex = -1; // 赋初值在0-258之外
		int TailIndex = -1;

		// DataHead
		for (int i = 0; i < TotalARMArray.GetSize() - 1; i++)
		{
			if (((char)TotalARMArray.GetAt(i) & 0xFF) == head[0])
				if (((char)TotalARMArray.GetAt(i + 1) & 0xFF) == head[1])
				{
					HeadIndex = i;
					break;
				}
		}

		// DataTail
		for (int i = 0; i < TotalARMArray.GetSize() - 1; i++)
		{
			if (((char)TotalARMArray.GetAt(i) & 0xFF) == tail[0])
				if (((char)TotalARMArray.GetAt(i + 1) & 0xFF) == tail[1])
				{
					TailIndex = i;
					break;
				}
		}

		//-----------------------数据包异常处理------------------------//
		// 如果没有检测到包头或者包尾则返回。不执行后面语句
		if ((HeadIndex == -1) || (TailIndex == -1))  return;

		if (HeadIndex > TailIndex) // 如果包头大于包尾则清除包头之前的数据
		{
			TotalARMArray.RemoveAt(0, HeadIndex);
			return;
		}

		//提取一个数据包
		CByteArray OnePackArray;
		OnePackArray.Copy(TotalARMArray);
		TotalARMArray.RemoveAt(0, StandardPackLength);
		temperature[0] = (OnePackArray[3] & 0xFF) * 256.0 + (OnePackArray[4] & 0xFF) / 10.0;
		temperature[1] = (OnePackArray[5] & 0xFF) * 256.0 + (OnePackArray[6] & 0xFF) / 10.0;
		temperature[2] = (OnePackArray[7] & 0xFF) * 256.0 + (OnePackArray[8] & 0xFF) / 10.0;
	}
}

void CXrays_64ChannelDlg::GetVoltCurrent() {

	// 一个包8个字节
	int StandardPackLength = 8;
	if (TotalARMArray.GetSize() >= StandardPackLength)
	{
		//包头包尾判断
		BYTE head[2] = { 0xAA, 0xCC };
		BYTE tail[2] = { 0xBB, 0xDD };
		//----------------------------------寻找包头包尾---------------------------------//
		int HeadIndex = -1; // 赋初值在0-258之外
		int TailIndex = -1;

		// DataHead
		for (int i = 0; i < TotalARMArray.GetSize() - 1; i++)
		{
			if (((char)TotalARMArray.GetAt(i) & 0xFF) == head[0])
				if (((char)TotalARMArray.GetAt(i + 1) & 0xFF) == head[1])
				{
					HeadIndex = i;
					break;
				}
		}

		// DataTail
		for (int i = 0; i < TotalARMArray.GetSize() - 1; i++)
		{
			if (((char)TotalARMArray.GetAt(i) & 0xFF) == tail[0])
				if (((char)TotalARMArray.GetAt(i + 1) & 0xFF) == tail[1])
				{
					TailIndex = i;
					break;
				}
		}

		//-----------------------数据包异常处理------------------------//
		// 如果没有检测到包头或者包尾则返回。不执行后面语句
		if ((HeadIndex == -1) || (TailIndex == -1))  return;

		if (HeadIndex > TailIndex) // 如果包头大于包尾则清除包头之前的数据
		{
			TotalARMArray.RemoveAt(0, HeadIndex);
			return;
		}

		//提取一个数据包
		CByteArray OnePackArray;
		OnePackArray.Copy(TotalARMArray);
		TotalARMArray.RemoveAt(0, StandardPackLength);

		powerVolt = ((OnePackArray[2] & 0xFF) * 256.0 + (OnePackArray[3] & 0xFF)) / 100.0;
		powerCurrent = ((OnePackArray[4] & 0xFF) * 256.0 + (OnePackArray[5] & 0xFF)) /1000.0;
		
		// 根据查询的顺序，先返回温度指令，再返回电压电流指令，因此在电流返回后再刷新
		refreshBar();
	}
}

void CXrays_64ChannelDlg::refreshBar() {
	//刷新状态栏
	CString strInfo;
	strInfo.Format(_T("%.2f℃,%.2f℃,%.2f℃,Volt:%.2fV,I:%.2fA"), temperature[0], temperature[1],
		temperature[2], powerVolt, powerCurrent);
	m_statusBar.SetPaneText(1, strInfo);
	
	// 保存数据到文件
	double data[5] = { temperature[0], temperature[1], temperature[2], powerVolt, powerCurrent};
	SaveEnviromentFile(data);
}