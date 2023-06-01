
#include "pch.h"
#include "CClientSocket.h"
#include "Xrays_64ChannelDlg.h"

CClientSocket::CClientSocket(CXrays_64ChannelDlg* dlg)
{
	this->m_pMainDlg = dlg;
}


CClientSocket::~CClientSocket()
{
}


void CClientSocket::OnReceive(int nErrorCode)
{	
	// �����ں� +PLS12345
	const int Len = 20;
	char recBuf[Len] = {0};
	int sockAddrLen = sizeof(SOCKADDR_IN);
	int nLength = ReceiveFrom(recBuf, Len, (SOCKADDR*)&ClientAddr, &sockAddrLen, 0);
	
	// ��־��¼
	CString tempStr(recBuf);
	this->m_pMainDlg->m_page2->PrintLog(_T("RECV ASCII: ") + tempStr);

	if (recBuf[0] == '+' && recBuf[1] == 'P' && recBuf[2] == 'L' && recBuf[3] == 'S') {
		//int tmp = '0';
		//int newID = (recBuf[5] - tmp) * 10000 + (recBuf[6] - tmp) * 1000 + (recBuf[7] - tmp) * 100
		//	+ (recBuf[8] - tmp) * 10 + (recBuf[9] - tmp);
		CString str_new_ID;
		str_new_ID = CString(recBuf[5]) + CString(recBuf[6]) + CString(recBuf[7]) + CString(recBuf[8]) +
			CString(recBuf[9]);
		//str_new_ID.Format(_T("%d"), newID);
		if (str_new_ID != this->m_pMainDlg->m_targetID) {
			this->m_pMainDlg->m_targetID = str_new_ID;
			this->m_pMainDlg->m_getTargetChange = TRUE;
			this->m_pMainDlg->SaveFile(_T("ShotNumber"), recBuf, nLength);
		}
	}
	//���ܺ�����Ϣ
	CSocket::OnReceive(nErrorCode);
	
	/*
	//--------------1.����UDPSocket------------
	if (m_UDPSocket != NULL) delete m_UDPSocket;
	m_UDPSocket = new CClientSocket();//��ʼ��,�´���һ���Ի���Socket
	m_UDPSocket->Create(0, SOCK_DGRAM, NULL);
	m_UDPSocket->Bind(12100, "192.168.0.175");

	//--------------2.��ȡSocket�󶨵�ip�Ͷ˿�--------------
	//��ȡ������IP�Ͷ˿ں�
	CString strIp;
	UINT uiPort;

	//��ȡ���صķ���źͶ˿ں�
	m_UDPSocket->GetSockName(strIp, uiPort);

	//��ʾ���صĶ˿ںź�IP��
	SetDlgItemText(IDC_UDPIP, strIp);
	SetDlgItemInt(IDC_UDPPORT, uiPort);
	*/
}