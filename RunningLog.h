#pragma once


// RunningLog 对话框

class RunningLog : public CDialog
{
	DECLARE_DYNAMIC(RunningLog)

public:
	RunningLog(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~RunningLog();

	void PrintLog(CString info);

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_RunningLog };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	// 记录接受的日志信息
	CString m_Information;
	CEdit m_LogEdit;
};
