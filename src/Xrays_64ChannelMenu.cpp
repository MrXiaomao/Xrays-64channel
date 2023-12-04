#include "pch.h"
#include "Xrays_64Channel.h"
#include "Xrays_64ChannelDlg.h"
#include "PowerDlg.h"
#include "NetSettingDlg.h"
#include "Order.h"

void CXrays_64ChannelDlg::OnBnClickedRelayConnect()
{
	// CPowerDlg powerdlg; // ����һ��ģ̬�Ի��� 
	// powerdlg.DoModal(); // ��ʾģ̬�Ի��� ���в�����swp_SHOWNOMAL,  SW_SHOW, SW_VISION ����Ч����һ����
	GetDlgItem(IDC_POWER_NET)->EnableWindow(FALSE); //����,��ֹ�û��ظ����

	CString strTemp;
	GetDlgItemText(IDC_POWER_NET, strTemp);
	if (strTemp == _T("��Դ��������")) {
		netRelayStatus = ConnectRelayTCP();
		if (netRelayStatus) {
			// �����߳̽�������
			AfxBeginThread(&Recv_Relay, this);
			SetDlgItemText(IDC_POWER_NET, _T("��Դ����Ͽ�"));
			m_RelayNetStatusLED.RefreshWindow(1, _T("������"));//��ָʾ��
			GetDlgItem(IDC_POWER_ONOFF)->EnableWindow(TRUE); //����ʹ��
			CString info = _T("��Դ���磨�̵�����������");
			m_page1.PrintLog(info);
		}
		else {
			m_RelayNetStatusLED.RefreshWindow(0, _T("����ʧ��"));//ָʾ��
			CString info = _T("��Դ���磨�̵���������ʧ�ܣ���������IP�Ͷ˿ں��Ƿ���ȷ");
			m_page1.PrintLog(info);
		}
	}
	else {
		netRelayStatus = FALSE; // �������ƹر��߳�
		closesocket(relaySocket); // �ر��׽���
		SetDlgItemText(IDC_POWER_NET, _T("��Դ��������"));
		m_RelayNetStatusLED.RefreshWindow(0, _T("�ѶϿ�"));//ָʾ��
		m_RelayStatusLED.RefreshWindow(2, _T("Unknow"));
		GetDlgItem(IDC_POWER_ONOFF)->EnableWindow(FALSE); //����
		CString info = _T("��Դ���磨�̵������ѶϿ�");
		m_page1.PrintLog(info);
	}
	GetDlgItem(IDC_POWER_NET)->EnableWindow(TRUE); //�ָ�ʹ��
}

BOOL CXrays_64ChannelDlg::ConnectRelayTCP() {
	// 1�������׽���
	relaySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (relaySocket == INVALID_SOCKET) {
		CString info = _T("�̵��������ʼ������ʧ�ܣ�");
		m_page1.PrintLog(info);
		return FALSE;
	}

	// 2�����ӷ�����
	CString StrSerIp = _T("192.168.10.22"); //Ĭ��ֵ
	int RelayPort = 1030; //Ĭ��ֵ

	// ��ȡ�����ļ�
	Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
	if(!jsonSetting.isNull()){
		if(jsonSetting.isMember("IP_Relay")) {
			StrSerIp = jsonSetting["IP_Relay"].asCString();
		}
		else{
			char* pStrIP = CstringToWideCharArry(StrSerIp);
			jsonSetting["IP_Relay"] = pStrIP;
			CString info = _T("�����ļ���δ���ҵ��̵�����ַIP�����ؼ���\"IP_Relay\",����Ĭ��ֵ��");
			info += StrSerIp;
			m_page1.PrintLog(info);
		}
		if(jsonSetting.isMember("Port_Relay")) {
			RelayPort = jsonSetting["Port_Relay"].asInt();
		}
		else{
			jsonSetting["Port_Relay"] = RelayPort;
			CString info;
			info.Format(_T("�����ļ���δ���ҵ��̵�����ַ�˿ںţ����ؼ���\"Port_Relay\",����Ĭ��ֵ��%d"),RelayPort);
			m_page1.PrintLog(info);
		}
	}
	char* pStrIP = CstringToWideCharArry(StrSerIp);
	// ���ܷ����Ķ������浽�����ļ�
	WriteSetting(_T("Setting.json"), jsonSetting);

	// 3�������������
	sockaddr_in server_addr;
	inet_pton(AF_INET, pStrIP, (void*)&server_addr.sin_addr.S_un.S_addr);
	server_addr.sin_family = AF_INET;  // ʹ��IPv4��ַ
	server_addr.sin_port = htons(RelayPort); //���أ�5000

	// 4����������Ƿ�����,�Լ���ʾ�豸����״��
	if (connect(relaySocket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
		return FALSE;
	}
	return TRUE;
}

//���ռ̵���TCP��������
UINT Recv_Relay(LPVOID p)
{
	CXrays_64ChannelDlg* dlg = (CXrays_64ChannelDlg*)p;
	while (1)
	{
		// �Ͽ������رձ��߳�
		if (!dlg->netRelayStatus) return 0;
		const int dataLen = 10; //���յ����ݰ�����
		BYTE mk[dataLen];
		int nLength;
		nLength = recv(dlg->relaySocket, (char*)mk, dataLen, 0); //����ģʽ

		if (nLength == -1)
		{
			return 0;
		}
		else {
			::PostMessage(dlg->m_hWnd, WM_UPDATE_RELAY, nLength, (LPARAM)(BYTE*)mk); //������Ϣ֪ͨ�����洦�����¼̵���״̬
		}
	}
}

void CXrays_64ChannelDlg::OnRelayChange() {
	GetDlgItem(IDC_POWER_ONOFF)->EnableWindow(FALSE); //����
	CString strTemp;
	GetDlgItemText(IDC_POWER_ONOFF, strTemp);
	if (strTemp == _T("��Դ����")) {
		send(relaySocket, (char*)Order::relay_ON, 10, 0);
		SetDlgItemText(IDC_POWER_ONOFF, _T("��Դ�ر�"));
	}
	else {
		send(relaySocket, (char*)Order::relay_OFF, 10, 0);
		SetDlgItemText(IDC_POWER_ONOFF, _T("��Դ����"));
	}
	GetDlgItem(IDC_POWER_ONOFF)->EnableWindow(TRUE); //�ָ�ʹ��
}

void CXrays_64ChannelDlg::OnPowerMenu()
{
	// TODO: �ڴ���������������
	CPowerDlg powerdlg; // ����һ��ģ̬�Ի��� 
	powerdlg.DoModal(); // ��ʾģ̬�Ի��� ���в�����swp_SHOWNOMAL,  SW_SHOW, SW_VISION ����Ч����һ����
}

void CXrays_64ChannelDlg::OnNetSettingMenu()
{
	// TODO: �ڴ���������������
	CNetSetting netsetdlg; // ����һ��ģ̬�Ի���
	netsetdlg.DoModal(); // ��ʾģ̬�Ի��� ���в�����swp_SHOWNOMAL,  SW_SHOW, SW_VISION ����Ч����һ����
}

BOOL CXrays_64ChannelDlg::TempVoltMonitorON() 
{
	//�������ļ���ȡIP��Port
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
		// ���ӳɹ�
		// �����߳̽�������
		m_pThread_ARM = AfxBeginThread(&Recv_ARM, this);

		//������ذ�ť�Լ�״̬
		ARMnetStatus = TRUE;
		
		info = _T("�¶�/��ѹ/��������ѿ���");
		m_page1.PrintLog(info);
		
		//�״�������ʱֱ�Ӳ�ѯһ���¶�/��ѹ/����
		send(armSocket, (char*)Order::ARM_Temperature1, 12, 0);
		return TRUE;
	}
	else {
		info = _T("�޷������¶�/��ѹ/�����������");
		m_page1.PrintLog(info);
		return FALSE;
	}
}

void CXrays_64ChannelDlg::TempVoltMonitorOFF() 
{
	ARMnetStatus = FALSE; //�������̣߳��ȹر��̣߳��ٹر��׽���
	closesocket(armSocket); // �ر��׽���
	CString info;
	info = _T("�¶�/��ѹ/��������ѹر�");
	m_page1.PrintLog(info);
}

UINT Recv_ARM(LPVOID p) // ���߳̽���ARM��������
{
	CXrays_64ChannelDlg* dlg = (CXrays_64ChannelDlg*)p;
	while (1)
	{
		// �Ͽ������رձ��߳�
		if (!dlg->ARMnetStatus) return 0;
		const int dataLen = 20; //���յ����ݰ�����
		BYTE mk[dataLen];
		int nLength;
		nLength = recv(dlg->armSocket, (char*)mk, dataLen, 0); //����ģʽ

		if (nLength == -1)
		{
			return 0;
		}
		else {
			// ת��ΪCByteArray
			CByteArray buffer;
			for (int i = 0; i < nLength; i++) {
				buffer.Add(mk[i]);
			}
			dlg->TotalARMArray.Append(buffer); //ƴ��
			::PostMessage(dlg->m_hWnd, WM_UPDATE_ARM, 0, 0); //������Ϣ֪ͨ�����洦�������¶ȡ���ѹ������
		}
	}
}


BOOL CXrays_64ChannelDlg::ConnectGeneralTCP(SOCKET &my_socket, CString strIP, int port) {
	// 1�������׽���
	my_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (my_socket == INVALID_SOCKET) {
		// �豸�����ʼ������ʧ��
		return FALSE;
	}

	// 2�����ӷ�����
	char* pStrIP = CstringToWideCharArry(strIP);

	// 3�������������
	sockaddr_in server_addr;
	inet_pton(AF_INET, pStrIP, (void*)&server_addr.sin_addr.S_un.S_addr);
	server_addr.sin_family = AF_INET;  // ʹ��IPv4��ַ
	server_addr.sin_port = htons(port); //����

	// 4����������Ƿ�����,�Լ���ʾ�豸����״��
	if (connect(my_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
		return FALSE;
	}
	return TRUE;
}

//��ȡARM��Ϣ��6���¶ȡ���ѹ������
LRESULT CXrays_64ChannelDlg::OnUpdateARMStatic(WPARAM wParam, LPARAM lParam){
	GetARMData();
	return 0;
}

void CXrays_64ChannelDlg::GetARMData() {
	//��ͷ��β�ж�
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


//ˢ��״̬��
void CXrays_64ChannelDlg::refreshBar() {
	//�¶ȼ���豸1��2
	CString strInfo1;
	//������ֵ6553.5�������¶ȴ����������ڣ����߽Ӵ��쳣�����ɶ�����ԭ��
	for(int i=0; i<6; i++){
		CString tempStr;
		if(abs(temperature[i] - 6553.5)<0.01){
			tempStr = _T("--��,");
		}
		else{
			tempStr.Format(_T("%.2f��,"),temperature[i]);
		}
		strInfo1 += tempStr;
	}

	//��ѹ��������豸
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
	m_page1.PrintLog(_T("�¶ȡ���ѹ�������ɼ����ݣ�") + strInfo,FALSE);

	// �������ݵ��ļ�
	double data[8] = { temperature[0], temperature[1], temperature[2], temperature[3], temperature[4], temperature[5], 
						powerVolt, powerCurrent};
	SaveEnviromentFile(data);

	// �����¶�/��ѹ/������ѯ����״̬
	for (int i = 0; i < 3; i++) {
		feedbackARM[i] = FALSE;
	}
}