#pragma once

#include "Layout.h"
// RunningLog 对话框

// 用于界面上记录系统运行日志的Tab子对话框
class RunningLog : public CDialog
{
	DECLARE_DYNAMIC(RunningLog)

public:
	RunningLog(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~RunningLog();

	/*添加日志信息
	* info 待添加的信息
	* isShow 控制是否在界面显示该条日志
	*/
	void PrintLog(CString info, BOOL isShow = TRUE);
	
	CRect m_rect;
	CLayout m_layoutRunner;

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_RunningLog };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	// 记录接受的日志信息
	CString m_Information;
	CEdit m_LogEdit;
};
