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
			m_pThread_ARM = AfxBeginThread(&Recv_ARM, this);

			//更新相关按钮以及状态
			ARMnetStatus = TRUE;
			SetDlgItemText(IDC_TEMP_VOLT, _T("关闭")); 
			m_AMR_LED.RefreshWindow(TRUE, _T("ON"));
			
			info = _T("温度/电压/电流监测已开启");
			m_page1.PrintLog(info);
			
			//首次连接上时直接查询一次温度/电压/电流
			send(armSocket, (char*)Order::ARM_Temperature1, 12, 0);
			Sleep(10);//是否有必要
			send(armSocket, (char*)Order::ARM_Temperature2, 12, 0);
			Sleep(10);//是否有必要
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

LRESULT CXrays_64ChannelDlg::OnUpdateARMStatic(WPARAM wParam, LPARAM lParam){
	GetTemperature(); //尝试解析温度
	GetVoltCurrent(); //尝试解析电压电流
	return 0;
}

void CXrays_64ChannelDlg::GetTemperature() {
	//包头包尾判断
	BYTE head[2] = { 0xAA, 0xBB };
	BYTE tail[2] = { 0xCC, 0xDD };
	CByteArray outArray;
	BOOL foundPackage = GetOnePackage(TotalARMArray, outArray, head, tail, 2, 11);

	if(foundPackage)
	{
		int equiID = (outArray[2] & 0xFF); 
		int num = 0;
		if (equiID == 1) {
			num = 0;
			feedbackARM[0] = TRUE; //表明获取到了温度1的数据
		}
		if (equiID == 2) {
			num = 1;
			feedbackARM[1] = TRUE; //表明获取到了温度2的数据
		}
		else {
			return;
		}
		temperature[0 + num*3] = ((outArray[3] & 0xFF) * 256.0 + (outArray[4] & 0xFF)) / 10.0;
		temperature[1 + num * 3] = ((outArray[5] & 0xFF) * 256.0 + (outArray[6] & 0xFF)) / 10.0;
		temperature[2 + num * 3] = ((outArray[7] & 0xFF) * 256.0 + (outArray[8] & 0xFF)) / 10.0;
	}
	/*
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
		int equiID = (OnePackArray[2] & 0xFF); 
		int num = 0;
		if (equiID == 1) {
			num = 0;
			feedbackARM[0] = TRUE; //表明获取到了温度1的数据
		}
		if (equiID == 2) {
			num = 1;
			feedbackARM[1] = TRUE; //表明获取到了温度2的数据
		}
		temperature[0 + num*3] = ((OnePackArray[3] & 0xFF) * 256.0 + (OnePackArray[4] & 0xFF)) / 10.0;
		temperature[1 + num * 3] = ((OnePackArray[5] & 0xFF) * 256.0 + (OnePackArray[6] & 0xFF)) / 10.0;
		temperature[2 + num * 3] = ((OnePackArray[7] & 0xFF) * 256.0 + (OnePackArray[8] & 0xFF)) / 10.0;
	}*/
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
		
		feedbackARM[2] = TRUE; //表明获取到了电压电流的数据
		// 根据查询的顺序，先返回温度指令，再返回电压电流指令，因此在电流返回后再刷新
		refreshBar();
	}
}

void CXrays_64ChannelDlg::refreshBar() {
	//刷新状态栏
	CString strInfo1;
	if(feedbackARM[0]){
		strInfo1.Format(_T("%.2f℃,%.2f℃,%.2f℃,"), temperature[0], temperature[1], temperature[2]);
	}
	else {
		strInfo1 = _T("--℃,--℃,--℃,");
	}

	CString strInfo2;
	if (feedbackARM[1]) {
		strInfo2.Format(_T("%.2f℃,%.2f℃,%.2f℃,"), temperature[3], temperature[4], temperature[5]);
	}
	else {
		strInfo2 = _T("--℃,--℃,--℃,");
	}

	CString strInfo3;
	if (feedbackARM[2]) {
		strInfo3.Format(_T("%.2fV,%.2fA"), powerVolt, powerCurrent);
	}
	else {
		strInfo3 = _T("--V,--A");
	}

	CString strInfo = strInfo1 + strInfo2 + strInfo3;
	m_statusBar.SetPaneText(1, strInfo);
	
	// 保存数据到文件
	double data[8] = { temperature[0], temperature[1], temperature[2], temperature[3], temperature[4], temperature[5], 
						powerVolt, powerCurrent};
	SaveEnviromentFile(data);

	// 重置温度/电压/电流查询反馈状态
	for (int i = 0; i < 3; i++) {
		feedbackARM[i] = FALSE;
	}
}