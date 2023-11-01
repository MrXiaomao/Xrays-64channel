
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
	CSingleLock singleLock(&Mutex); //线程锁
	// 解析炮号 +PLS_12345
	const int Len = 30;
	char recBuf[Len] = {0};
	int sockAddrLen = sizeof(SOCKADDR_IN);
	int nLength = ReceiveFrom(recBuf, Len, (SOCKADDR*)&ClientAddr, &sockAddrLen, 0);
	
	//发送消息通知主界面处理,通知接收到炮号数据 (LPARAM)(LPCTSTR)p_str,
	//注意，要想传递字符串是必须使用SendMessage函数，因为堆栈数据PostMessage之后已经不再有效
	//SendMessage为阻塞式函数，待界面线程处理完后该线程继续运行
	::SendMessage(this->m_pMainDlg->m_hWnd, WM_UPDATE_SHOT, nLength, (LPARAM)(char*)recBuf);
	//接受函数信息
	CSocket::OnReceive(nErrorCode);
}