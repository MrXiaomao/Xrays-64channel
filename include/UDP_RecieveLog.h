#pragma once
#include "Layout.h"

// UDP_RecieveLog 对话框

class UDP_RecieveLog : public CDialog
{
	DECLARE_DYNAMIC(UDP_RecieveLog)

public:
	UDP_RecieveLog(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~UDP_RecieveLog();
	
	/*添加日志信息
	* info 待添加的信息
	* isShow 控制是否在界面显示该条日志
	*/
	void PrintLog(CString info, BOOL isShow = TRUE);
	
	CRect m_rect;
	CLayout m_layoutUDP;

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_UDP_RecieveLog };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	CString m_Information;
	CEdit m_LogEdit;
};
