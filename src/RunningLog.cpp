// RunningLog.cpp: 实现文件
//

#include "pch.h"
#include "Xrays_64Channel.h"
#include "RunningLog.h"
#include "afxdialogex.h"
#include "Log.h"
#include "LayoutInit.h"
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
	ON_WM_SIZE()
	ON_WM_SIZING()
END_MESSAGE_MAP()


BOOL RunningLog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	InitLayoutRunner(m_layoutRunner,this);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void RunningLog::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	m_layoutRunner.OnSize(cx, cy);
}

void RunningLog::OnSizing(UINT fwSide, LPRECT pRect)
{
	CDialog::OnSizing(fwSide, pRect);
	//	EASYSIZE_MINSIZE(600,400,fwSide,pRect);   //最小窗口
	// TODO: 在此处添加消息处理程序代码
}

// RunningLog 消息处理程序
void RunningLog::PrintLog(CString info, BOOL isShow)
{
	// 添加日志到文件
	CLog::SetPrefix(_T("RunningLog::PrintLog"));
	CLog::WriteMsg(info);

	// 添加日志到界面
	if(!isShow) return;
	CTime t = CTime::GetCurrentTime();
	CString strTime = t.Format(_T("[%Y-%m-%d %H:%M:%S]# "));
	m_Information = m_Information + strTime + info + _T("\r\n");
	UpdateData(FALSE);
	m_LogEdit.LineScroll(m_LogEdit.GetLineCount()); //每次刷新后都显示最底部
}
