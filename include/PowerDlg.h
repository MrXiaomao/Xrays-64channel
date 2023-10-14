#pragma once

#include "LEDButton.h"

// CPowerDlg 对话框
//继电器的相关控制与状态查询

UINT RecvRealy_Thread(LPVOID p); // 多线程接收网口数据(继电器)

class CPowerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CPowerDlg)

public:
	CPowerDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CPowerDlg();

// 对话框数据
public:
	BOOL ConnectRelayTCP(); // 继电器连接
	SOCKET relaySocket;
	
	int m_RelayPort;// TCP端口号
	BOOL netStatus; //联网状态
	LEDButton m_NetStatusLED;// 网络状态LED灯
	LEDButton m_RelayStatusLED;// 继电器电源状态LED灯

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_POWER_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();

public:
	afx_msg void OnTimer(UINT_PTR nIDEvent); //定时器
	afx_msg void OnBnClickedConnect(); 
	afx_msg void OnBnClickedChange(); //切换继电器开关状态
};
