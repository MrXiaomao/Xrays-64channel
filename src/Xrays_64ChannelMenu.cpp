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
		StrIP_ARM = jsonSetting["IP_ARM"].asCString();
		portARM = jsonSetting["Port_ARM"].asInt();

		CString info;
		if (ConnectGeneralTCP(armSocket, StrIP_ARM, portARM)) {
			// ���ӳɹ�
			// �����߳̽�������
			AfxBeginThread(&Recv_ARM, this);

			//������ذ�ť�Լ�״̬
			ARMnetStatus = TRUE;
			SetDlgItemText(IDC_TEMP_VOLT, _T("�ر�")); 
			m_AMR_LED.RefreshWindow(TRUE, _T("ON"));
			
			info = _T("�¶�/��ѹ/��������ѿ���");
			m_page1.PrintLog(info);
			
			//�״�������ʱֱ�Ӳ�ѯһ���¶�/��ѹ/����
			send(armSocket, (char*)Order::ARM_Temperature, 12, 0);
			Sleep(1);//�Ƿ��б�Ҫ
			send(armSocket, (char*)Order::ARM_VoltCurrent, 12, 0);
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
		char mk[dataLen];
		int nLength;
		nLength = recv(dlg->armSocket, mk, dataLen, 0); //����ģʽ

		if (nLength == -1)
		{
			return 0;
		}
		else {
			// ת��ΪCByteArray
			CByteArray buffer;
			for (int i = 0; i < nLength; i++) {
				buffer.Add((BYTE)mk[i]);
			}
			dlg->TotalARMArray.Append(buffer); //ƴ��

			dlg->GetTemperature(); //���Խ����¶�
			dlg->GetVoltCurrent(); //���Խ�����ѹ����
			dlg->refreshBar(); //ˢ�½����¶ȡ���ѹ��������ʾ
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

void CXrays_64ChannelDlg::GetTemperature() {

	int StandardPackLength = 11;
	if (TotalARMArray.GetSize() >= StandardPackLength)
	{
		// һ����11���ֽ�
		int StandardPackLength = 11;
		//��ͷ��β�ж�
		BYTE head[2] = { 0xAA, 0xBB };
		BYTE tail[2] = { 0xCC, 0xDD };
		//----------------------------------Ѱ�Ұ�ͷ��β---------------------------------//
		int HeadIndex = -1; // ����ֵ��0-258֮��
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

		//-----------------------���ݰ��쳣����------------------------//
		// ���û�м�⵽��ͷ���߰�β�򷵻ء���ִ�к������
		if ((HeadIndex == -1) || (TailIndex == -1))  return;

		if (HeadIndex > TailIndex) // �����ͷ���ڰ�β�������ͷ֮ǰ������
		{
			TotalARMArray.RemoveAt(0, HeadIndex);
			return;
		}

		//��ȡһ�����ݰ�
		CByteArray OnePackArray;
		OnePackArray.Copy(TotalARMArray);
		TotalARMArray.RemoveAt(0, StandardPackLength);
		temperature[0] = (OnePackArray[3] & 0xFF) * 256.0 + (OnePackArray[4] & 0xFF) / 10.0;
		temperature[1] = (OnePackArray[5] & 0xFF) * 256.0 + (OnePackArray[6] & 0xFF) / 10.0;
		temperature[2] = (OnePackArray[7] & 0xFF) * 256.0 + (OnePackArray[8] & 0xFF) / 10.0;
	}
}

void CXrays_64ChannelDlg::GetVoltCurrent() {

	// һ����8���ֽ�
	int StandardPackLength = 8;
	if (TotalARMArray.GetSize() >= StandardPackLength)
	{
		//��ͷ��β�ж�
		BYTE head[2] = { 0xAA, 0xCC };
		BYTE tail[2] = { 0xBB, 0xDD };
		//----------------------------------Ѱ�Ұ�ͷ��β---------------------------------//
		int HeadIndex = -1; // ����ֵ��0-258֮��
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

		//-----------------------���ݰ��쳣����------------------------//
		// ���û�м�⵽��ͷ���߰�β�򷵻ء���ִ�к������
		if ((HeadIndex == -1) || (TailIndex == -1))  return;

		if (HeadIndex > TailIndex) // �����ͷ���ڰ�β�������ͷ֮ǰ������
		{
			TotalARMArray.RemoveAt(0, HeadIndex);
			return;
		}

		//��ȡһ�����ݰ�
		CByteArray OnePackArray;
		OnePackArray.Copy(TotalARMArray);
		TotalARMArray.RemoveAt(0, StandardPackLength);

		powerVolt = ((OnePackArray[2] & 0xFF) * 256.0 + (OnePackArray[3] & 0xFF)) / 100.0;
		powerCurrent = ((OnePackArray[4] & 0xFF) * 256.0 + (OnePackArray[5] & 0xFF)) /1000.0;
		
		// ���ݲ�ѯ��˳���ȷ����¶�ָ��ٷ��ص�ѹ����ָ�����ڵ������غ���ˢ��
		refreshBar();
	}
}

void CXrays_64ChannelDlg::refreshBar() {
	//ˢ��״̬��
	CString strInfo;
	strInfo.Format(_T("%.2f��,%.2f��,%.2f��,Volt:%.2fV,I:%.2fA"), temperature[0], temperature[1],
		temperature[2], powerVolt, powerCurrent);
	m_statusBar.SetPaneText(1, strInfo);
	
	// �������ݵ��ļ�
	double data[5] = { temperature[0], temperature[1], temperature[2], powerVolt, powerCurrent};
	SaveEnviromentFile(data);
}