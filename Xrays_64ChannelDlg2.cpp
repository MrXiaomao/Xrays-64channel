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
	SetDlgItemText(IDC_UDPIP, strIp);
	SetDlgItemInt(IDC_UDPPORT, uiPort);
	CString info;
	info.Format(_T("UDP�Ѵ򿪣��˿ں�Ϊ:%d"), uiPort);
	m_page1->PrintLog(info);
	m_page2->PrintLog(info);
}

void CXrays_64ChannelDlg::CloseUDP() {
	if (m_UDPSocket != NULL) delete m_UDPSocket;
	CString info = _T("UDP�����ѹر�");
	m_page1->PrintLog(info);
	m_page2->PrintLog(info);
}

void CXrays_64ChannelDlg::SaveFile(CString myID, const char* mk, int length) {
	CString filename = myID + _T(".dat");
	CString wholePath = saveAsPath + filename;
	fstream datafile(wholePath, ios::out | ios::app | ios::binary);   // ׷��
	for (int i = 0; i < length; i++) {
		datafile << mk[i];
	}
	datafile.close();
}

//���ƶ˿����뷶Χ0~65535
void CXrays_64ChannelDlg::OnEnKillfocusPort1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(true);
	if ((sPort < 0) || (sPort > 65535))
	{
		MessageBox(_T("�˿ڵķ�ΧΪ0~65535\n"));
		if (sPort > 65535)
		{
			sPort = 65535;
		}
		else
		{
			sPort = 1;
		}
		UpdateData(false);
	}
}

//���ƶ˿����뷶Χ0~65535
void CXrays_64ChannelDlg::OnEnKillfocusUDPPort()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(true);
	if ((sPort < 0) || (sPort > 65535))
	{
		MessageBox(_T("�˿ڵķ�ΧΪ0~65535\n"));
		if (sPort > 65535)
		{
			sPort = 65535;
		}
		else
		{
			sPort = 1;
		}
		UpdateData(false);
	}
}

// �����ļ��洢·��
void CXrays_64ChannelDlg::OnBnClickedSaveas()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	Json::Value myJson = ReadSetting(_T("Setting.json"));
	CString dir;
	dir = myJson["SaveDir"].asCString();

	HWND hwnd = this->GetSafeHwnd();
	if (SetSavePath(hwnd, dir, saveAsPath))
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

	//��ȡĿ¼·��
	/*TCHAR   szPath[MAX_PATH] = {0};
	LPITEMIDLIST   pitem;
	BROWSEINFO   info;
	::ZeroMemory(&info, sizeof(info));
	info.hwndOwner = this->m_hWnd;
	info.lpszTitle = _T("��ѡ��·��: ");
	info.pszDisplayName = szPath;

	//CStringתchar*
	USES_CONVERSION;
	char* result = T2A(dir);
	wcscpy_s(szPath, dir);

	if (pitem = ::SHBrowseForFolder(&info))
	{
		::SHGetPathFromIDList(pitem, szPath);
		saveAsPath = szPath;
		saveAsPath += "\\";

		// д������ļ�
		string pStr = _UnicodeToUtf8(saveAsPath);
		myJson["SaveDir"] = pStr;
		WriteSetting(_T("Setting.json"),myJson);

		// ��ӡ��־��Ϣ
		CString info = _T("ʵ�����ݱ���·����") + saveAsPath;
		m_page1->PrintLog(info);
		UpdateData(FALSE);
	}
	*/
}
