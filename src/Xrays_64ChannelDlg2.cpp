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
// ���ļ���Ų����ú������Լ����ܺ���

// �ļ�����
#include <iostream>
#include <fstream>
using namespace std;
extern const int TIMER_INTERVAL;
//CMutex Mutex; //mutex���߳���

// ����TCP��IP��PORT����ѡ�������ʹ��״̬
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

	//���Ϳ̶�����,ֻ�����������ʹ��
	GetDlgItem(IDC_CALIBRATION)->EnableWindow(!flag);
}

//�������ò������ʹ��״̬
void CXrays_64ChannelDlg::SetParameterInputStatus(BOOL flag)
{
	//����ˢ��ʱ��
	GetDlgItem(IDC_RefreshTimeEdit)->EnableWindow(flag);
	//���ײ���ʱ��
	GetDlgItem(IDC_MeasureTime)->EnableWindow(flag);
	//����ģʽѡ��
	GetDlgItem(IDC_WAVE_MODE)->EnableWindow(flag);
	//��ֵ����
	GetDlgItem(IDC_CH1Threahold)->EnableWindow(flag);
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
	WriteSetting(_T("Setting.json"), jsonSetting);

	//--------------1.����UDPSocket------------
	if (!m_UDPSocket)
		delete m_UDPSocket;
	m_UDPSocket = new CUDP_Socket(this); //��ʼ��,�´���һ���Ի���Socket
	m_UDPSocket->Create(m_UDPPort, SOCK_DGRAM, NULL);

	//--------------2.��ȡSocket�󶨵�ip�Ͷ˿�--------------
	//��ȡ������IP�Ͷ˿ں�
	CString strIp;
	UINT uiPort;

	//��ȡ���صķ���źͶ˿ں�
	m_UDPSocket->GetSockName(strIp, uiPort);

	//��ʾ���صĶ˿ںź�IP��
	// SetDlgItemText(IDC_UDPIP, strIp);
	SetDlgItemInt(IDC_UDPPORT, uiPort);
	CString info;
	info.Format(_T("UDP�Ѵ򿪣��˿ں�Ϊ:%d"), uiPort);
	m_page1.PrintLog(info);
	m_page2.PrintLog(info);

	UDPStatus = TRUE;
	GetDlgItem(IDC_UDPPORT)->EnableWindow(FALSE);

	// 2���ж�TCP����״̬
	BOOL AllconnectStatus = TRUE;
	for (int num = 0; num < 4; num++) {
		if(connectStatusList[num] != NetSwitchList[num+1]) AllconnectStatus = FALSE;
	}

	// UDP��TCP���򿪺������ʹ���Զ�����
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
	CString info = _T("UDP�����ѹر�");
	m_page1.PrintLog(info);
	m_page2.PrintLog(info);
	UDPStatus = FALSE;
	GetDlgItem(IDC_UDPPORT)->EnableWindow(TRUE);
	GetDlgItem(IDC_AutoMeasure)->EnableWindow(FALSE);
}

//����TCP��������Ĳ�������
void CXrays_64ChannelDlg::SaveFile(CString myID, char *mk, int length)
{
	if (length < 1)
		return;
	CString filename = myID + _T(".dat");
	CString wholePath = saveAsTargetPath + filename;
	fstream datafile(wholePath, ios::out | ios::app | ios::binary); // ׷��
	if (datafile.is_open())
	{
		datafile.write(mk, length);
		datafile.close();
	}
}

//����ARM�����¶�����
void CXrays_64ChannelDlg::SaveEnviromentFile(double data[])
{	
	CTime ct = CTime::GetCurrentTime();
	CString filename = ct.Format(_T("Temperatue_%Y%m%d.dat"));
	CString strPart_Time = ct.Format(_T("%Y-%m-%d %H:%M:%S"));

	CString parentPath = _T("Enviroment\\");
	CString wholePath = parentPath + filename;
	if (!IsPathExit(parentPath)) Mkdir(parentPath);
	if (!IsFileExit(wholePath)) {
		//�״δ����ļ���ʱ�������ͷ
		fstream datafile(wholePath, ios::out | ios::app| ios::left); // ׷��
		if (datafile.is_open())
		{
			datafile << setw(25) << "Time";
			datafile << setw(10)<< "Temp1(��)";
			datafile << setw(10) << "Temp2(��)";
			datafile << setw(10) << "Temp3(��)";
			datafile << setw(10) << "Temp4(��)";
			datafile << setw(10) << "Temp5(��)";
			datafile << setw(10) << "Temp6(��)";
			datafile << setw(10) << "Volt(V)";
			datafile << setw(10) << "I(A)";
			datafile << endl;
			datafile.close();
		}
	}

	fstream datafile(wholePath, ios::out | ios::app | ios::left); // ׷��
	datafile.setf(ios::fixed, ios::floatfield);  // �趨Ϊ fixed ģʽ����С�����ʾ������
	datafile.precision(2);  // ���þ��� 2
	//datafile.width(25);
	if (datafile.is_open())
	{
		datafile << setw(25) << _UnicodeToUtf8(strPart_Time);
		if (feedbackARM[0]) {
			datafile << setw(10) << data[0];
			datafile << setw(10) << data[1];
			datafile << setw(10) << data[2];
		}
		else {
			datafile << setw(10) << "--";
			datafile << setw(10) << "--";
			datafile << setw(10) << "--";
		}
		if (feedbackARM[1]) {
			datafile << setw(10) << data[3];
			datafile << setw(10) << data[4];
			datafile << setw(10) << data[5];
		}
		else {
			datafile << setw(10) << "--";
			datafile << setw(10) << "--";
			datafile << setw(10) << "--";
		}
		if (feedbackARM[2]) {
			datafile << setw(10) << data[6];
			datafile << setw(10) << data[7];
		}
		else {
			datafile << setw(10) << "--";
			datafile << setw(10) << "--";
		}
		datafile << endl;
		datafile.close();
	}
}

//����TCP�˿����뷶Χ
void CXrays_64ChannelDlg::OnEnKillfocusPort1()
{
	ConfinePortRange(PortList[0]);
}

//����TCP�˿����뷶Χ
void CXrays_64ChannelDlg::OnEnKillfocusPort2()
{
	ConfinePortRange(PortList[1]);
}

//����TCP�˿����뷶Χ
void CXrays_64ChannelDlg::OnEnKillfocusPort3()
{
	ConfinePortRange(PortList[2]);
}

//����TCP�˿����뷶Χ
void CXrays_64ChannelDlg::OnEnKillfocusPort4()
{
	ConfinePortRange(PortList[3]);
}

//����UDP�˿����뷶Χ
void CXrays_64ChannelDlg::OnEnKillfocusUDPPort()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	ConfinePortRange(m_UDPPort);
}

// ����TCP���ڽ��յĻ�������
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

//���ƶ˿ں����뷶Χ0~65535
void CXrays_64ChannelDlg::ConfinePortRange(int &myPort)
{
	UpdateData(true);
	if ((myPort < 0) || (myPort > 65535))
	{
		MessageBox(_T("�˿ڵķ�ΧΪ0~65535\n"));
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

//��������ˢ��ʱ�䷶Χ,��λms��
//����ˢ��ʱ��ָ����FPGA�ɼ�һ��������������ʱ��
void CXrays_64ChannelDlg::OnEnKillfocusRefreshTime()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(true);
	int MaxTime = 60 * 1000; //��λms
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	Json::Value myJson = ReadSetting(_T("Setting.json"));
	CString dir;
	if (myJson["SaveDir"].isString())
	{
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	// ��ȡ����������޸�ָ��
	UpdateData(TRUE);

	//�ӽ����ȡˢ��ʱ��
	char res[5];
	DecToHex(RefreshTime, res);
	Order::WaveRefreshTime[6] = res[0];
	Order::WaveRefreshTime[7] = res[1];
	Order::WaveRefreshTime[8] = res[2];
	Order::WaveRefreshTime[9] = res[3];

	//����ˢ��ʱ�䣬���δ�����������δ�����ֵ
	for (int num = 0; num < 4; num++) {
		if(connectStatusList[num]) {
			BackSend(num, Order::WaveRefreshTime, 12, 0, 1);
			BackSend(num, Order::TriggerThreshold, 12, 0, 1);
			BackSend(num, Order::TriggerIntervalTime, 12, 0, 1);
		}
	}
	
	CString info;
	if (m_WaveMode.GetCurSel() == 0)
	{ //512������
		for (int num = 0; num < 4; num++) {
			if(connectStatusList[num]) {
				BackSend(num, Order::WorkMode0, 12, 0, 1);
			}
		}
		info.Format(_T("����ˢ��ʱ��:%dms,512�����׹���ģʽ"), RefreshTime);
	}
	else if (m_WaveMode.GetCurSel() == 1)
	{ // 16������
		for (int num = 0; num < 4; num++) {
			if(connectStatusList[num]) {
				BackSend(num, Order::WorkMode3, 12, 0, 1);
			}
		}
		info.Format(_T("����ˢ��ʱ��:%dms,16�����׹���ģʽ"), RefreshTime);
	}
	m_page1.PrintLog(info);
}

//��ҳ�Ի���ѡ��
void CXrays_64ChannelDlg::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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

//������������
void CXrays_64ChannelDlg::AddTCPData(int num, char *tempChar, int len)
{
	/*for (int i = 0; i < len; i++) {
		if (RECVLength[num] + i < DataMaxlen)
		{
			DataCH1[RECVLength[num] + i] = tempChar[i];
		}
	}*/
	RECVLength[num] += len;
}

//�������ڻ�������С
void CXrays_64ChannelDlg::SetSocketSize(SOCKET &sock, int nsize)
{
	int nErrCode = 0;
	unsigned int uiRcvBuf = 0;
	unsigned int uiNewRcvBuf = 0;
	int uiRcvBufLen = sizeof(uiRcvBuf);
	nErrCode = getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&uiRcvBuf, &uiRcvBufLen);
	if (SOCKET_ERROR == nErrCode)
	{
		MessageBox(_T("��ȡ���������SOCKET���ͻ�������Сʧ��"));
		return;
	}
	// uiRcvBuf *= nsize;//����ϵͳ��������ΪĬ�ϵı��� uiRcvBuf=100kB
	uiRcvBuf = nsize;
	nErrCode = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&uiRcvBuf, uiRcvBufLen);
	if (SOCKET_ERROR == nErrCode)
	{
		MessageBox(_T("����SOCKET���ͻ�������Сʧ��"));
		return;
	}
}

//��������
void CXrays_64ChannelDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	//��״̬��λ�����²���
	ResizeBar();
	// �Խ���ؼ����²���
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

//�Խ���ؼ����²���
void CXrays_64ChannelDlg::ResizeBar()
{
	CRect rectDlg;
	GetClientRect(rectDlg); //��ô���Ĵ�С
	//�ж�״̬���Ƿ񱻴���
	if (IsWindow(m_statusBar.m_hWnd))
	{
		//���������ţ�ID����ʽ�Ϳ�ȣ�SBPS_NORMALΪ��ͨ��ʽ���̶���ȣ�SBPS_STRETCHΪ������ʽ�����Զ���չ���Ŀռ�
		m_statusBar.SetPaneInfo(0, 1001, SBPS_NORMAL, int(0.6 * rectDlg.Width()));
		m_statusBar.SetPaneInfo(1, 1002, SBPS_STRETCH, int(0.2 * rectDlg.Width()));
		m_statusBar.SetPaneInfo(2, 1003, SBPS_NORMAL, int(0.2 * rectDlg.Width()));
		RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
	}
}

void CXrays_64ChannelDlg::OnSizing(UINT fwSide, LPRECT pRect)
{
	CDialogEx::OnSizing(fwSide, pRect);
	//TODO: �ڴ˴������Ϣ����������
}

//��ָ����ķ���ָ��
BOOL CXrays_64ChannelDlg::BackSend(int num, BYTE *msg, int msgLength, int flags,
								 int sleepTime, int maxWaitingTime, BOOL isShow)
{
	CSingleLock singleLock(&Mutex); //�߳���

	if (ifFeedback[num])
		return FALSE;

	// ����ʱδ��⵽����ָ����ٴη���ָ�FPGA��ѭ���ȴ����Ρ�
	for (int i = 0; i < 3; i++)
	{
		int times = 0; //�ȴ�ʱ��
		CTime tm1;
		tm1 = CTime::GetCurrentTime();
		BOOL flag = FALSE;

		// ��ʼ��������ز���

		singleLock.Lock(); //Mutex
		if (singleLock.IsLocked()){
			ifFeedback[num] = FALSE;
		}
		singleLock.Unlock(); //Mutex

		TCPfeedback[num] = FALSE;
		LastSendMsg[num] = NULL;
		RecvMsg[num] = NULL;
		recievedFBLength[num] = 0;

		// ����ָ��
		if (!ifFeedback[num])
		{
			singleLock.Lock(); //�߳���
			if (singleLock.IsLocked()){
				ifFeedback[num] = TRUE;
			}
			singleLock.Unlock(); 

			send(SocketList[num], (char *)msg, msgLength, flags);
			// Sleep(sleepTime);
			LastSendMsg[num] = (char *)msg;
			FeedbackLen[num] = msgLength;
			CString info;
			info.Format(_T("CH%d SEND HEX(%d):"), num+1, i+1);
			info = info + Char2HexCString(LastSendMsg[num], msgLength);
			m_page1.PrintLog(info, isShow); 
		}

		// ����ʽ�жϵȴ�����ָ��������ж��Ƿ��뷢��ָ����ͬ
		do
		{ 
			// �жϽ���ָ���뷢��ָ���Ƿ���ͬ
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
				info.Format(_T("CH%dָ���У��ɹ�:"), num+1);
				info += Char2HexCString(RecvMsg[num], recievedFBLength[num]);
				m_page1.PrintLog(info, isShow);

				//���յ�����ָ����³�ʼ�������������
				TCPfeedback[num] = FALSE;
				
				singleLock.Lock(); //�߳���
				if (singleLock.IsLocked()){
					ifFeedback[num] = FALSE;
				}
				singleLock.Unlock(); 

				LastSendMsg[num] = NULL;
				RecvMsg[num] = NULL;
				recievedFBLength[num] = 0;

				return TRUE;
			}

			// ����ȴ�ʱ��
			CTime tm2;
			tm2 = CTime::GetCurrentTime();
			CTimeSpan span;
			span = tm2 - tm1;
			times = span.GetSeconds();
		} while (times < maxWaitingTime); 

		CString info;
		info.Format(_T("CH%d �ȴ�ָ���ʱ��%ds,�����������ʱ��%ds,"), num+1, times, maxWaitingTime);
		m_page1.PrintLog(info, isShow);
	}

	CString info;
	info.Format(_T("CH%d ����3���·�ָ����޷����ܵ�����ָ�SEND HEX: "), num+1);
	info = info + Char2HexCString(LastSendMsg[num], msgLength);
	m_page1.PrintLog(info, TRUE);

	// �ָ�ָ�����ز���
	singleLock.Lock(); //�߳���
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

//����ָ����ķ���ָ��
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);//�ӿؼ��������   ����������ݺ���Խ�����Ӧ����
	if (BST_CHECKED == IsDlgButtonChecked(IDC_CHECK1)) //BST_CHECKED����ʾ��ť��ѡ�С�BST_UNCHECKED����ʾ�ð�ťδѡ�У�unckecked����
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
	UpdateData(FALSE);//ˢ�¿ؼ�
}

void CXrays_64ChannelDlg::OnBnClickedCheck1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);//�ӿؼ��������   ����������ݺ���Խ�����Ӧ����
	NetSwitchList[0] = NetSwitchList[1] & NetSwitchList[2] & NetSwitchList[3] & NetSwitchList[4];
	UpdateData(FALSE);//ˢ�¿ؼ�
}

void CXrays_64ChannelDlg::OnBnClickedCheck2()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);//�ӿؼ��������   ����������ݺ���Խ�����Ӧ����
	NetSwitchList[0] = NetSwitchList[1] & NetSwitchList[2] & NetSwitchList[3] & NetSwitchList[4];
	UpdateData(FALSE);//ˢ�¿ؼ�
}

void CXrays_64ChannelDlg::OnBnClickedCheck3()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);//�ӿؼ��������   ����������ݺ���Խ�����Ӧ����
	NetSwitchList[0] = NetSwitchList[1] & NetSwitchList[2] & NetSwitchList[3] & NetSwitchList[4];
	UpdateData(FALSE);//ˢ�¿ؼ�
}

void CXrays_64ChannelDlg::OnBnClickedCheck4()
{
	UpdateData(TRUE);//�ؼ���ֵ��>����
	NetSwitchList[0] = NetSwitchList[1] & NetSwitchList[2] & NetSwitchList[3] & NetSwitchList[4];
	UpdateData(FALSE);//����ֵ��>�ؼ���ʾ
}

LRESULT CXrays_64ChannelDlg::OnUpdateTrigerLog(WPARAM wParam, LPARAM lParam){
	int num = (int) wParam;
	CString info;
	info.Format(_T("CH%d���յ�Ӳ�������ź�"), num+1);
	m_page1.PrintLog(info,FALSE);
	return 0;
}

LRESULT CXrays_64ChannelDlg::OnUpdateTimer2(WPARAM wParam, LPARAM lParam){
	int num = (int)wParam;
	if (m_nTimerId[1]==0) {
		m_nTimerId[1] = SetTimer(2, TIMER_INTERVAL, NULL); //����Ѿ����������ظ�����

		CString info;
		info.Format(_T("CH%d����2�Ŷ�ʱ��"), num + 1);
		m_page1.PrintLog(info, FALSE);
	}
	return 0;
}

LRESULT CXrays_64ChannelDlg::OnUpdateShot(WPARAM wParam, LPARAM lParam){
	
	int nLength = (int)wParam;
	char* recBuf = (char*)lParam;
	CString tempStr(recBuf);
	m_page2.PrintLog(_T("RECV ASCII: ") + tempStr);

	if (recBuf[0] == '+' && recBuf[1] == 'P' && recBuf[2] == 'L' && recBuf[3] == 'S') {
		CString str_new_ID;
		str_new_ID = CString(recBuf[5]) + CString(recBuf[6]) + CString(recBuf[7]) 
					+ CString(recBuf[8]) + CString(recBuf[9]);
		if (str_new_ID != m_targetID) {
			m_targetID = str_new_ID;
			m_getTargetChange = TRUE;
			recBuf[nLength] = (char)'\r\n'; // ׷��һ�����з���
			int saveLength = nLength + 1;
			SaveFile(saveAsPath + _T("ShotNumber"), recBuf, saveLength);
		}
	}
	return 0;
}