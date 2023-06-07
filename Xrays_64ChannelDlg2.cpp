#include "json/json.h"

#include "pch.h"
#include "framework.h"
#include "Xrays_64Channel.h"
#include "Xrays_64ChannelDlg.h"
#include "afxdialogex.h"
#include "Order.h"

#include "afx.h"
// ���ļ���Ų����ú������Լ����ܺ���

// �ļ�����
#include <iostream>
#include <fstream>
using namespace std;

// ����TCP��IP��PORT����ѡ�������ʹ��״̬
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

//�������ò������ʹ��״̬
void CXrays_64ChannelDlg::SetParameterInputStatus(BOOL flag) {
	//����ˢ��ʱ��
	GetDlgItem(IDC_RefreshTimeEdit)->EnableWindow(flag); 
	//���ײ���ʱ��
	GetDlgItem(IDC_MeasureTime)->EnableWindow(flag);
}

// ��UDPͨ��
void CXrays_64ChannelDlg::OpenUDP()
{
	UpdateData(TRUE); //��ȡ����ؼ�������ֵ
	// ��ȡ���ò��������õ���Ӧ�ؼ���
	Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
	//�����ļ������ڣ������������ļ�
	jsonSetting["Port_UDP"] = m_UDPPort;
	WriteSetting(_T("Setting.json"),jsonSetting);

	//--------------1.����UDPSocket------------
	if (!m_UDPSocket) delete m_UDPSocket;
	m_UDPSocket = new CClientSocket(this);//��ʼ��,�´���һ���Ի���Socket
	m_UDPSocket->Create(m_UDPPort, SOCK_DGRAM, NULL);

	//--------------2.��ȡSocket�󶨵�ip�Ͷ˿�--------------
	//��ȡ������IP�Ͷ˿ں�
	CString strIp;
	UINT uiPort;

	//��ȡ���صķ���źͶ˿ں�
	m_UDPSocket->GetSockName(strIp, uiPort);

	//��ʾ���صĶ˿ںź�IP��
	//SetDlgItemText(IDC_UDPIP, strIp);
	SetDlgItemInt(IDC_UDPPORT, uiPort);
	CString info;
	info.Format(_T("UDP�Ѵ򿪣��˿ں�Ϊ:%d"), uiPort);
	m_page1->PrintLog(info);
	m_page2->PrintLog(info);
	
	UDPStatus = TRUE;
	GetDlgItem(IDC_UDPPORT)->EnableWindow(FALSE);
	// UDP��TCP���򿪺������ʹ���Զ�����
	if (connectStatus) { 
		GetDlgItem(IDC_AutoMeasure)->EnableWindow(TRUE);
	}
}

void CXrays_64ChannelDlg::CloseUDP() {
	if (m_UDPSocket != NULL) delete m_UDPSocket;
	CString info = _T("UDP�����ѹر�");
	m_page1->PrintLog(info);
	m_page2->PrintLog(info);
	UDPStatus = FALSE;
	GetDlgItem(IDC_UDPPORT)->EnableWindow(FALSE);
	GetDlgItem(IDC_AutoMeasure)->EnableWindow(FALSE);
}

//����TCP��������Ĳ�������
void CXrays_64ChannelDlg::SaveFile(CString myID, const char* mk, int length) {
	CString filename = myID + _T(".dat");
	CString wholePath = saveAsTargetPath + filename;
	fstream datafile(wholePath, ios::out | ios::app | ios::binary);   // ׷��
	if (datafile.is_open()) {
		for (int i = 0; i < length; i++) {
			datafile << mk[i];
		}
		datafile.close();
	}
}

//����TCP�˿����뷶Χ
void CXrays_64ChannelDlg::OnEnKillfocusPort1()
{
	ConfinePortRange(&sPort);
}

//����TCP�˿����뷶Χ
void CXrays_64ChannelDlg::OnEnKillfocusPort2()
{
	ConfinePortRange(&sPort2);
}

//����TCP�˿����뷶Χ
void CXrays_64ChannelDlg::OnEnKillfocusPort3()
{
	ConfinePortRange(&sPort3);
}

//����TCP�˿����뷶Χ
void CXrays_64ChannelDlg::OnEnKillfocusPort4()
{
	ConfinePortRange(&sPort4);
}

//����UDP�˿����뷶Χ
void CXrays_64ChannelDlg::OnEnKillfocusUDPPort()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	ConfinePortRange(&m_UDPPort);
}

//���ƶ˿ں����뷶Χ0~65535
void CXrays_64ChannelDlg::ConfinePortRange(int* myPort) {
	UpdateData(true);
	if ((*myPort < 0) || (*myPort > 65535))
	{
		MessageBox(_T("�˿ڵķ�ΧΪ0~65535\n"));
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

//��������ˢ��ʱ�䷶Χ,��λms��
//����ˢ��ʱ��ָ����FPGA�ɼ�һ��������������ʱ��
void CXrays_64ChannelDlg::OnEnKillfocusRefreshTime()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(true);
	int MaxTime = 60*1000; //��λms
	if ((RefreshTime <= 0) || (RefreshTime > MaxTime))
	{
		CString message;
		message.Format(_T("����ˢ��ʱ�䷶ΧΪ0~%dms\n"), MaxTime);
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

//�������ײ���ʱ�䷶Χ����λms
void CXrays_64ChannelDlg::OnEnKillfocusMeasureTime()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(true);
	int MaxTime = 60 * 1000; //��λms
	if ((MeasureTime <= 0) || (MeasureTime > MaxTime))
	{
		CString message;
		message.Format(_T("���ײ���ʱ�䷶ΧΪ0~%dms\n"), MaxTime);
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

//�����ļ��洢·��
void CXrays_64ChannelDlg::OnBnClickedSaveas()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	Json::Value myJson = ReadSetting(_T("Setting.json"));
	CString dir;
	if (myJson["SaveDir"].isString()) {
		dir = myJson["SaveDir"].asCString();
	}

	HWND hwnd = this->GetSafeHwnd();
	if (BrowserMyPath(hwnd, dir, saveAsPath))
	{
		// д������ļ�
		string pStr = _UnicodeToUtf8(saveAsPath);
		myJson["SaveDir"] = pStr;
		WriteSetting(_T("Setting.json"), myJson);

		// ��ӡ��־��Ϣ
		CString info = _T("ʵ�����ݱ���·����") + saveAsPath;
		m_page1->PrintLog(info);
		UpdateData(FALSE);
	}
}

//���ò���,ͨ����������ư巢�Ͳ���
void CXrays_64ChannelDlg::SendParameterToTCP()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	// ��ȡ����������޸�ָ��
	UpdateData(TRUE);
	char res[5];
	//����ˢ��ʱ��
	DecToHex(RefreshTime, res);
	Order::WaveRefreshTime[6] = res[0];
	Order::WaveRefreshTime[7] = res[1];
	Order::WaveRefreshTime[8] = res[2];
	Order::WaveRefreshTime[9] = res[3];

	//����ָ��
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

//��ҳ�Ի���ѡ��
void CXrays_64ChannelDlg::OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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

//�����ǰ��־��ϵͳ��־��UDP������־��
void CXrays_64ChannelDlg::OnBnClickedClearLog()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
