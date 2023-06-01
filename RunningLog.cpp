// RunningLog.cpp: 实现文件
//

#include "pch.h"
#include "Xrays_64Channel.h"
#include "RunningLog.h"
#include "afxdialogex.h"


// RunningLog 对话框

IMPLEMENT_DYNAMIC(RunningLog, CDialog)

RunningLog::RunningLog(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_RunningLog, pParent)
	, m_Information(_T(""))
{

}

RunningLog::~RunningLog()
{
}

void RunningLog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_RUNNING_LOG, m_Information);
	DDX_Control(pDX, IDC_RUNNING_LOG, m_LogEdit);
}


BEGIN_MESSAGE_MAP(RunningLog, CDialog)
END_MESSAGE_MAP()


// RunningLog 消息处理程序
void RunningLog::PrintLog(CString info)
{
	CTime t = CTime::GetCurrentTime();
	CString strTime = t.Format(_T("[%Y-%m-%d %H:%M:%S]# "));
	m_Information = m_Information + strTime + info + _T("\r\n");
	UpdateData(FALSE);
	m_LogEdit.LineScroll(m_LogEdit.GetLineCount()); //每次刷新后都显示最底部
}
