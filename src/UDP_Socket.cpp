
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
	::SendMessage(this->m_pMainDlg->m_hWnd, WM_UPDATE_SHOT, nLength, (LPARAM)(char*)recBuf); 
	// ��־��¼
	/*CString tempStr(recBuf);
	this->m_pMainDlg->m_page2.PrintLog(_T("RECV ASCII: ") + tempStr);

	if (recBuf[0] == '+' && recBuf[1] == 'P' && recBuf[2] == 'L' && recBuf[3] == 'S') {
		int tmp = '0';
		int newID = (recBuf[5] - tmp) * 10000 + (recBuf[6] - tmp) * 1000 + (recBuf[7] - tmp) * 100
			+ (recBuf[8] - tmp) * 10 + (recBuf[9] - tmp);
		str_new_ID.Format(_T("%d"), newID);
		
		CString str_new_ID;
		str_new_ID = CString(recBuf[5]) + CString(recBuf[6]) + CString(recBuf[7]) + CString(recBuf[8]) +
			CString(recBuf[9]);

		
		if (str_new_ID != this->m_pMainDlg->m_targetID) {
			singleLock.Lock(); //Mutex�߳���
			if (singleLock.IsLocked()){
				this->m_pMainDlg->m_targetID = str_new_ID;
				this->m_pMainDlg->m_getTargetChange = TRUE;
			}
			singleLock.Unlock();
			recBuf[nLength] = (char)'\r\n'; // ׷��һ�����з���
			int saveLength = nLength + 1;
			this->m_pMainDlg->SaveFile(this->m_pMainDlg->saveAsPath + _T("ShotNumber"), recBuf, saveLength);
		}
		
	}*/
	//���ܺ�����Ϣ
	CSocket::OnReceive(nErrorCode);
}