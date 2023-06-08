
// Xrays_64ChannelDlg.h: 头文件
//

#pragma once

//#pragma comment(lib,"json.lib")
#include "json/json.h"
#include "LEDButton.h"
#include "CClientSocket.h"
#include "UDP_RecieveLog.h"
#include "RunningLog.h"
#include "MyConst.h"
using namespace std;

UINT Recv_Th1(LPVOID p); // 多线程接收网口数据
UINT Recv_Th2(LPVOID p); // 多线程接收网口数据
UINT Recv_Th3(LPVOID p); // 多线程接收网口数据
UINT Recv_Th4(LPVOID p); // 多线程接收网口数据

// CXrays_64ChannelDlg 对话框
class CXrays_64ChannelDlg : public CDialogEx
{
// 构造
public:
	CXrays_64ChannelDlg(CWnd* pParent = nullptr);	// 标准构造函数
	virtual ~CXrays_64ChannelDlg();

	void OpenUDP(); // 打开UDP通信
	void CloseUDP(); //关闭UDP通信，以及相应资源
	void SendParameterToTCP(); //发送配置参数
	void SendCalibration(CString fileName); //发送刻度曲线
	BOOL ConnectTCP1(); //连接网络1
	BOOL ConnectTCP2(); //连接网络2
	BOOL ConnectTCP3(); //连接网络3
	BOOL ConnectTCP4(); //连接网络4
	void ConfinePortRange(int* myPort); //限制端口号输入范围
	void SetTCPInputStatus(BOOL flag); //设置TCP的IP、PORT、复选框的输入使能状态
	void SetParameterInputStatus(BOOL flag); //设置配置参数框的使能状态
	void SaveFile(CString myID, const char* mk, int length); // 保存文件
	void AddTCPData(int channel, char* tempChar, int len); // 缓存网口数据
	void ResetTCPData(); // 重置缓存数据

	LEDButton m_NetStatusLED;
	LEDButton m_NetStatusLED2;
	LEDButton m_NetStatusLED3;
	LEDButton m_NetStatusLED4;
	CClientSocket* m_UDPSocket; //本地UDP服务
	SOCKET mySocket;
	SOCKET mySocket2;
	SOCKET mySocket3;
	SOCKET mySocket4;

	// 单个包：512能谱=516*4字节，（10ms刷新，10秒测量时长对应总包长=100*10*516*4）
	// 16通道=20*4字节（1ms刷新，10秒测量时长对应总包长=1000*10*20*4）
	const int DataMaxlen;
	BOOL connectStatus;
	BOOL UDPStatus; //UDP工作状态
	BOOL MeasureStatus; // 手动测量状态
	BOOL AutoMeasureStatus; // 自动测量状态
	BOOL GetDataStatus; // 是否接受到TCP网口的数据
	BOOL m_getTargetChange; // 检测炮号是否变化
	int timer; // 计时器，满三秒后则发送停止测量
	CString saveAsPath; //数据存储根路径
	CString saveAsTargetPath; //数据存储炮号路径
	CStatusBar m_statusBar;

	char* DataCH1; // 网口接收的数据，缓存下来，接收完后再存储到文件中。
	char* DataCH2;
	char* DataCH3;
	char* DataCH4;
	int CH1_RECVLength;//接受数据长度
	int CH2_RECVLength;//接受数据长度
	int CH3_RECVLength;//接受数据长度
	int CH4_RECVLength;//接受数据长度

	RunningLog* m_page1;
	UDP_RecieveLog* m_page2;
	int m_currentTab;

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_XRAYS64CHANNEL_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnConnect(); // 连接网络
	afx_msg void OnEnKillfocusPort1();
	afx_msg void OnEnKillfocusPort2();
	afx_msg void OnEnKillfocusPort3();
	afx_msg void OnEnKillfocusPort4();
	afx_msg void OnEnKillfocusUDPPort();
	afx_msg void OnEnKillfocusRefreshTime(); //限制刷新时间输入范围
	afx_msg void OnEnKillfocusMeasureTime(); //限制能谱测量总时长输入范围
	afx_msg void OnBnClickedStart(); // 开始测量（手动测量）
	afx_msg void OnBnClickedAutomeasure(); //自动测量
	afx_msg void OnTimer(UINT_PTR nIDEvent); //定时器
	afx_msg void OnBnClickedSaveas();//保存文件，设置文件存储路径
	afx_msg void OnBnClickedClearLog();// 清空日志
	afx_msg void OnBnClickedUdpButton();//UDP开关
	// 炮号，一种打靶序列号
	//CStatic m_TargetID;
	// 网址IP
	CIPAddressCtrl ServerIP;
	// TCP端口号
	int sPort;
	int sPort2;
	int sPort3;
	int sPort4;
	// 触发方式下拉框
	CComboBox m_TriggerType;
	// 能谱模式选择下拉框
	CComboBox m_WaveMode;
	// 炮号，一种打靶序列号
	CString m_targetID;
	// UDP端口号
	int m_UDPPort;
	afx_msg void OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult);
	CTabCtrl m_Tab;
	// 界面输入的能谱总测量时间,ms
	int MeasureTime;
	// 界面输入的能谱刷新时间，ms
	int RefreshTime;
	afx_msg void OnBnClickedTestButton();
	afx_msg void OnCbnSelchangeWaveMode();
};
