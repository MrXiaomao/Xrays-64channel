// UDP_RecieveLog.cpp: 实现文件
//

#include "pch.h"
#include "Xrays_64Channel.h"
#include "UDP_RecieveLog.h"
#include "afxdialogex.h"
#include "Log.h"
#include "LayoutInit.h"

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
	ON_WM_SIZE()
END_MESSAGE_MAP()


BOOL UDP_RecieveLog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	InitLayoutUDPLog(m_layoutUDP, this);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void UDP_RecieveLog::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	m_layoutUDP.OnSize(cx, cy);
}


// UDP_RecieveLog 消息处理程序
void UDP_RecieveLog::PrintLog(CString info, BOOL isShow)
{
	// 添加日志到文件
	CLog::SetPrefix(_T("UDP_RecieveLog::PrintLog"));
	CLog::WriteMsg(info);

	// 添加日志到界面
	CTime t = CTime::GetCurrentTime();
	CString strTime = t.Format(_T("[%Y-%m-%d %H:%M:%S]# "));
	m_Information = m_Information + strTime + info + _T("\r\n");
	UpdateData(FALSE);
	m_LogEdit.LineScroll(m_LogEdit.GetLineCount()); //每次刷新后都显示最底部
}
