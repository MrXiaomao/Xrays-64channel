#pragma once


// CNetSetting 对话框

class CNetSetting : public CDialogEx
{
	DECLARE_DYNAMIC(CNetSetting)

public:
	CNetSetting(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CNetSetting();

// 对话框数据
	BOOL isDataChange;

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_NETSETTING_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void SaveNetSetting();//“保存”按钮
	afx_msg void OnBnClickedCancel();
	virtual BOOL OnInitDialog();

	// 端口号
	int m_PortUDP;
	int PortCHList[3];
	int m_PortARM;
	CString StrIP_CH[3];
	CString StrIP_ARM;
	//监测各个文本控件是否有过编辑痕迹（不检测内容的前后一致性），哪怕跟原始值相同仍然认为发生改动。
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedOk();
	int m_ARM_RefreshTime;
};
