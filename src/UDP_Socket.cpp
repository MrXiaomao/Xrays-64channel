
#include "pch.h"
#include "UDP_Socket.h"
#include "Xrays_64ChannelDlg.h"

CUDP_Socket::CUDP_Socket(CXrays_64ChannelDlg* dlg)
{
	this->m_pMainDlg = dlg;
}


CUDP_Socket::~CUDP_Socket()
{
}


void CUDP_Socket::OnReceive(int nErrorCode)
{	
	CSingleLock singleLock(&Mutex); //�߳���
	// �����ں� +PLS_12345
	const int Len = 30;
	char recBuf[Len] = {0};
	int sockAddrLen = sizeof(SOCKADDR_IN);
	int nLength = ReceiveFrom(recBuf, Len, (SOCKADDR*)&ClientAddr, &sockAddrLen, 0);
	
	//������Ϣ֪ͨ�����洦��,֪ͨ���յ��ں����� (LPARAM)(LPCTSTR)p_str,
	//ע�⣬Ҫ�봫���ַ����Ǳ���ʹ��SendMessage��������Ϊ��ջ����PostMessage֮���Ѿ�������Ч
	//SendMessageΪ����ʽ�������������̴߳��������̼߳�������
	::SendMessage(this->m_pMainDlg->m_hWnd, WM_UPDATE_SHOT, nLength, (LPARAM)(char*)recBuf);
	//���ܺ�����Ϣ
	CSocket::OnReceive(nErrorCode);
}