#include "pch.h"
#include "Xrays_64Channel.h"
#include "Xrays_64ChannelDlg.h"
#include "PowerDlg.h"
#include "NetSettingDlg.h"
#include "Order.h"

void CXrays_64ChannelDlg::OnBnClickedPowerButton()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CPowerDlg powerdlg; // ����һ��ģ̬�Ի��� 
	powerdlg.DoModal(); // ��ʾģ̬�Ի��� ���в�����swp_SHOWNOMAL,  SW_SHOW, SW_VISION ����Ч����һ����
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

void CXrays_64ChannelDlg::TempVoltMonitorON_OFF() 
{
	GetDlgItem(IDC_TEMP_VOLT)->EnableWindow(FALSE); //����
	CString strTemp;
	GetDlgItemText(IDC_TEMP_VOLT, strTemp);
	if (strTemp == _T("����")) {

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
			SetDlgItemText(IDC_TEMP_VOLT, _T("�ر�")); 
			m_AMR_LED.RefreshWindow(TRUE, _T("ON"));
			
			info = _T("�¶�/��ѹ/��������ѿ���");
			m_page1.PrintLog(info);
			
			//�״�������ʱֱ�Ӳ�ѯһ���¶�/��ѹ/����
			send(armSocket, (char*)Order::ARM_Temperature1, 12, 0);
		}
		else {
			info = _T("�޷������¶�/��ѹ/�����������");
			m_page1.PrintLog(info);
		}
	}
	else {
		ARMnetStatus = FALSE; //�������̣߳��ȹر��̣߳��ٹر��׽���
		closesocket(armSocket); // �ر��׽���
		SetDlgItemText(IDC_TEMP_VOLT, _T("����"));
		m_AMR_LED.RefreshWindow(FALSE, _T("OFF"));
		CString info;
		info = _T("�¶�/��ѹ/��������ѹر�");
		m_page1.PrintLog(info);
	}
	GetDlgItem(IDC_TEMP_VOLT)->EnableWindow(TRUE); //�ָ�ʹ��
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
		strInfo2.Format(_T("%.2fV,%.2fA"), powerVolt, powerCurrent);
	}

	CString strInfo = strInfo1 + strInfo2;
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