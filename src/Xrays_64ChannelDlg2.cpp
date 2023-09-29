#include "json/json.h"

#include "pch.h"
#include "framework.h"
#include "Xrays_64Channel.h"
#include "Xrays_64ChannelDlg.h"
#include "afxdialogex.h"
#include "Order.h"
#include "Log.h"
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

	//���Ϳ̶�����,ֻ�����������ʹ��
	GetDlgItem(IDC_CALIBRATION)->EnableWindow(!flag);
}

//�������ò������ʹ��״̬
void CXrays_64ChannelDlg::SetParameterInputStatus(BOOL flag) {
	//����ˢ��ʱ��
	GetDlgItem(IDC_RefreshTimeEdit)->EnableWindow(flag); 
	//���ײ���ʱ��
	GetDlgItem(IDC_MeasureTime)->EnableWindow(flag);
	//����ģʽѡ��
	GetDlgItem(IDC_WAVE_MODE)->EnableWindow(flag);
}

// ��UDPͨ��
void CXrays_64ChannelDlg::OpenUDP()
{
	CLog::WriteMsg(_T("���Դ�UDP��"));
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
	m_page1.PrintLog(info);
	m_page2.PrintLog(info);
	
	UDPStatus = TRUE;
	GetDlgItem(IDC_UDPPORT)->EnableWindow(FALSE);
	// UDP��TCP���򿪺������ʹ���Զ�����
	if (connectStatus) { 
		GetDlgItem(IDC_AutoMeasure)->EnableWindow(TRUE);
	}
}

void CXrays_64ChannelDlg::CloseUDP() {
	if (m_UDPSocket != NULL) delete m_UDPSocket;
	m_UDPSocket = NULL;
	CString info = _T("UDP�����ѹر�");
	m_page1.PrintLog(info);
	m_page2.PrintLog(info);
	UDPStatus = FALSE;
	GetDlgItem(IDC_UDPPORT)->EnableWindow(TRUE);
	GetDlgItem(IDC_AutoMeasure)->EnableWindow(FALSE);
}

//����TCP��������Ĳ�������
void CXrays_64ChannelDlg::SaveFile(CString myID, char* mk, int length) {
	if (length < 1) return;
	CString filename = myID + _T(".dat");
	CString wholePath = saveAsTargetPath + filename;
	fstream datafile(wholePath, ios::out | ios::app | ios::binary);   // ׷��
	if (datafile.is_open()) {
		datafile.write(mk, length);
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
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
	ConfinePortRange(&m_UDPPort);
}

// ����TCP���ڽ��յĻ�������
void CXrays_64ChannelDlg::ResetTCPData()
{
	memset(DataCH1, 0, DataMaxlen);
	memset(DataCH2, 0, DataMaxlen);
	memset(DataCH3, 0, DataMaxlen);
	memset(DataCH4, 0, DataMaxlen);
	CH1_RECVLength = 0;
	CH2_RECVLength = 0;
	CH3_RECVLength = 0;
	CH4_RECVLength = 0;
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
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
	UpdateData(true);
	int MaxTime = 60*1000; //��λms
	if ((RefreshTime < 10) || (RefreshTime > MaxTime))
	{
		CString message;
		message.Format(_T("����ˢ��ʱ�䷶ΧΪ10~%dms\n"), MaxTime);
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

//�������ײ���ʱ�䷶Χ����λms
void CXrays_64ChannelDlg::OnEnKillfocusMeasureTime()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
	UpdateData(true);
	int MaxTime = 60 * 1000; //��λms
	if ((MeasureTime < 1) || (MeasureTime > MaxTime))
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
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
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
		m_page1.PrintLog(info);
		UpdateData(FALSE);
	}
}

//���ò���,ͨ����������ư巢�Ͳ���
void CXrays_64ChannelDlg::SendParameterToTCP()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
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
	MySend(mySocket, Order::WaveRefreshTime, 12, 0, 1);
	//MySend(mySocket2, Order::WaveRefreshTime, 12, 0, 1);
	//MySend(mySocket3, Order::WaveRefreshTime, 12, 0, 1);
	//MySend(mySocket4, Order::WaveRefreshTime, 12, 0, 1);

	MySend(mySocket, Order::TriggerThreshold, 12, 0, 1);
	//MySend(mySocket2, Order::TriggerThreshold, 12, 0, 1);
	//MySend(mySocket3, Order::TriggerThreshold, 12, 0, 1);
	//MySend(mySocket4, Order::TriggerThreshold, 12, 0, 1);

	MySend(mySocket, Order::TriggerIntervalTime, 12, 0, 1);
	//MySend(mySocket2, Order::TriggerIntervalTime, 12, 0, 1);
	//MySend(mySocket3, Order::TriggerIntervalTime, 12, 0, 1);
	//MySend(mySocket4, Order::TriggerIntervalTime, 12, 0, 1);

	if (m_WaveMode.GetCurSel() == 0) { //512������
		MySend(mySocket, Order::WorkMode0, 12, 0, 1);
		//MySend(mySocket2, Order::WorkMode0, 12, 0, 1);
		//MySend(mySocket3, Order::WorkMode0, 12, 0, 1);
		//MySend(mySocket4, Order::WorkMode0, 12, 0, 1);
		CString info;
		info.Format(_T("����ˢ��ʱ��:%dms,512�����׹���ģʽ"), RefreshTime);
		m_page1.PrintLog(info);
	}
	else if (m_WaveMode.GetCurSel() == 1) { //16������
		MySend(mySocket, Order::WorkMode3, 12, 0, 1);
		//MySend(mySocket2, Order::WorkMode3, 12, 0, 1);
		//MySend(mySocket3, Order::WorkMode3, 12, 0, 1);
		//MySend(mySocket4, Order::WorkMode3, 12, 0, 1);
		CString info;
		info.Format(_T("����ˢ��ʱ��:%dms,16�����׹���ģʽ"), RefreshTime);
		m_page1.PrintLog(info);
	}
}

//��ҳ�Ի���ѡ��
void CXrays_64ChannelDlg::OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
	m_currentTab = m_Tab.GetCurSel();
	switch (m_currentTab)
	{
	case 0:
		m_page1.ShowWindow(SW_SHOW);
		m_page2.ShowWindow(SW_HIDE);
		//m_page1.UpdateData(FALSE);
		break;
	case 1:
		m_page1.ShowWindow(SW_HIDE);
		m_page2.ShowWindow(SW_SHOW);
		//m_page2.UpdateData(FALSE);
		break;
	*pResult = 0;
	}
}

//�����ǰ��־��ϵͳ��־��UDP������־��
void CXrays_64ChannelDlg::OnBnClickedClearLog()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
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

// ������������
void CXrays_64ChannelDlg::AddTCPData(int channel, char* tempChar, int len) {
	switch (channel)
	{
	case 1:
		/*for (int i = 0; i < len; i++) {
			if (CH1_RECVLength + i < DataMaxlen)
			{
				DataCH1[CH1_RECVLength + i] = tempChar[i];
			}
		}*/
		CH1_RECVLength += len;
		break;
	case 2:
		CH2_RECVLength += len;
		break;
	case 3:
		CH3_RECVLength += len;
		break;
	case 4:
		CH4_RECVLength += len;
		break;
	}
}

// �������ڻ�������С
void CXrays_64ChannelDlg::SetSocketSize(SOCKET sock, int nsize)
{
	int nErrCode = 0;//����ֵ
	unsigned int uiRcvBuf = 0;
	unsigned int uiNewRcvBuf = 0;
	int uiRcvBufLen = sizeof(uiRcvBuf);
	nErrCode = getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*)&uiRcvBuf, &uiRcvBufLen);
	if (SOCKET_ERROR == nErrCode)
	{
		MessageBox(_T("��ȡ���������SOCKET���ͻ�������Сʧ��"));
		return;
	}
	//uiRcvBuf *= nsize;//����ϵͳ��������ΪĬ�ϵı��� uiRcvBuf=100kB
	uiRcvBuf = nsize;
	nErrCode = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*)&uiRcvBuf, uiRcvBufLen);
	if (SOCKET_ERROR == nErrCode)
	{
		MessageBox(_T("����SOCKET���ͻ�������Сʧ��"));
		return;
	}
}

void CXrays_64ChannelDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	//��״̬��λ�����²���
	ResizeBar();
	// �Խ���ؼ����²���
	m_layout.OnSize(cx, cy);

	if(m_page1.GetSafeHwnd() || m_page2.GetSafeHwnd())
	{
		CRect rctab;
		CRect reItem;
		int m_nWidth, m_nHeight;

		m_Tab.GetClientRect(&rctab);

		m_nWidth = rctab.right - rctab.left;
		m_nHeight = rctab.bottom-rctab.top;

		m_Tab.GetItemRect(0,&reItem);
		rctab.DeflateRect(0, reItem.bottom, 0,0);

		m_page1.MoveWindow(&rctab);
		m_page2.MoveWindow(&rctab);
	}
}

// ���ڳߴ�仯�����»���״̬��
void CXrays_64ChannelDlg::ResizeBar() {
	CRect rectDlg;
	GetClientRect(rectDlg);//��ô���Ĵ�С
	//�ж�״̬���Ƿ񱻴���
	if (IsWindow(m_statusBar.m_hWnd))
	{
		//���������ţ�ID����ʽ�Ϳ��ȣ�SBPS_NORMALΪ��ͨ��ʽ���̶����ȣ�SBPS_STRETCHΪ������ʽ�����Զ���չ���Ŀռ�
		m_statusBar.SetPaneInfo(0, 1001, SBPS_NORMAL, int(0.6 * rectDlg.Width()));
		m_statusBar.SetPaneInfo(1, 1002, SBPS_STRETCH, int(0.2 * rectDlg.Width()));
		m_statusBar.SetPaneInfo(2, 1003, SBPS_NORMAL, int(0.2 * rectDlg.Width()));
		RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
	}
}


void CXrays_64ChannelDlg::OnSizing(UINT fwSide, LPRECT pRect)
{
	CDialogEx::OnSizing(fwSide, pRect);
	//	EASYSIZE_MINSIZE(600,400,fwSide,pRect);
	// TODO: �ڴ˴�������Ϣ�����������
}

BOOL CXrays_64ChannelDlg::MySend(SOCKET socket, BYTE *msg, int msgLength, int flags, 
	int sleepTime, int maxWaitingTime) {
	// ����ָ��������
	if (ifFeedback) return FALSE;

	for(int i =0; i<3; i++){
		int times = 0; //��ʱ��
		CTime tm1;
		tm1 = CTime::GetCurrentTime();
		BOOL flag = FALSE;

		// ��ʼ��ָ���������
		ifFeedback = FALSE;
		TCPfeedback = FALSE;
		LastSendMsg = NULL;
		RecvMsg = NULL; // ���ڽ�������
		recievedFBLength = 0;

		if (!ifFeedback) {
			ifFeedback = TRUE;
			send(socket, (char*)msg, msgLength, flags);
			//Sleep(sleepTime);
			LastSendMsg = (char*)msg;
			FeedbackLen = msgLength;
			CString info;
			info = _T("SEND HEX:") + Char2HexCString(LastSendMsg, msgLength);
			m_page1.PrintLog(info);
			//������ʱ������1��������ʾID�ţ��ڶ���������ʾˢ��ʱ��ms
			//SetTimer(5, 100, NULL);
		}

		do{ // ������û��ָ������˴�Ӧ�û���ɽ��濨�����������ǲ��ö��̴߳�����
			// �˱�־�ڼ�ʱ���б��޸ģ����ܻ������̳߳�ͻ�������ظ�����ָ�
			//�жϽ��ܷ���ָ���Ƿ���ȷ
			if(recievedFBLength == 12){
				CString info;
				info = _T("RECV HEX:") + Char2HexCString(RecvMsg, recievedFBLength);
				m_page1.PrintLog(info);
				if (strncmp(RecvMsg, LastSendMsg, msgLength) == 0) TCPfeedback = TRUE;
				// �������ڽ�����Ϣ
				RecvMsg = NULL; // ���ڽ�������
				recievedFBLength = 0;
			}

			// �жϷ���ָ��״̬
			if(TCPfeedback) {
				CString info;
				info = _T("ָ�������ɹ���") + Char2HexCString(RecvMsg, recievedFBLength);
				m_page1.PrintLog(info);
				
				//���³�ʼ�����ָ��
				TCPfeedback = FALSE;
				ifFeedback = FALSE;
				LastSendMsg = NULL;
				RecvMsg = NULL; // ���ڽ�������
				recievedFBLength = 0;

				return TRUE;
			}

			// ��ȡʱ���
			CTime tm2;
			tm2 = CTime::GetCurrentTime();
			CTimeSpan span;
			span = tm2 - tm1;
			times = span.GetSeconds();
		}while(times < maxWaitingTime); // û�з���ָ����һ��ѭ���ȴ���

		CString info;
		info.Format(_T("�ȴ�ʱ��%d�����ȴ�ʱ��%d,"), times, maxWaitingTime);
		m_page1.PrintLog(info);
	}
	
	// ����ѭ������ָ�����Ȼû�н��������ֹ��ʱ������������ϵͳ�쳣������Ի���
	//KillTimer(5);
	CString info = _T("�����޷��յ�����ָ��");
	m_page1.PrintLog(info);
	return FALSE;
}
