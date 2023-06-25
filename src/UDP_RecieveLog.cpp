// UDP_RecieveLog.cpp: 实现文件
//

#include "pch.h"
#include "Xrays_64Channel.h"
#include "UDP_RecieveLog.h"
#include "afxdialogex.h"


// UDP_RecieveLog 对话框

IMPLEMENT_DYNAMIC(UDP_RecieveLog, CDialog)

UDP_RecieveLog::UDP_RecieveLog(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_UDP_RecieveLog, pParent)
	, m_Information(_T(""))
{

}

UDP_RecieveLog::~UDP_RecieveLog()
{
}

void UDP_RecieveLog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_UDP_LOG, m_Information);
	DDX_Control(pDX, IDC_UDP_LOG, m_LogEdit);
}


BEGIN_MESSAGE_MAP(UDP_RecieveLog, CDialog)
END_MESSAGE_MAP()


// UDP_RecieveLog 消息处理程序
void UDP_RecieveLog::PrintLog(CString info)
{
	CTime t = CTime::GetCurrentTime();
	CString strTime = t.Format(_T("[%Y-%m-%d %H:%M:%S]# "));
	m_Information = m_Information + strTime + info + _T("\r\n");
	UpdateData(FALSE);
	m_LogEdit.LineScroll(m_LogEdit.GetLineCount()); //每次刷新后都显示最底部
}
