
// Xrays_64ChannelDlg.h: 头文件
//

#pragma once

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

	void InitBarSettings(); //初始化状态栏
	void InitTabSettings(); //初始化Tab对话框
	void InitOtherSettings(); // 初始化界面其他控件参数
	void ResizeBar(); //重新绘制状态栏
	void OpenUDP(); //打开UDP通信
	void CloseUDP(); //关闭UDP通信，以及相应资源
	void SendParameterToTCP(); //发送配置参数
	void SendCalibration(CString fileName); //发送刻度曲线
	BOOL ConnectTCP(int num); //连接TCP网络
	BOOL ConnectTCP1(); //连接网络1
	BOOL ConnectTCP2(); //连接网络2
	BOOL ConnectTCP3(); //连接网络3
	BOOL ConnectTCP4(); //连接网络4
	void ConfinePortRange(int &myPort); //限制端口号输入范围
	void SetTCPInputStatus(BOOL flag); //设置TCP的IP、PORT、复选框的输入使能状态
	void SetParameterInputStatus(BOOL flag); //设置配置参数框的使能状态
	void SaveFile(CString myID, char* mk, int length); // 保存文件
	void AddTCPData(int channel, char* tempChar, int len); // 缓存网口数据
	void ResetTCPData(); // 重置缓存数据
	void SetSocketSize(SOCKET sock, int nsize); //设置Socket缓冲区的大小
	
	/*阻塞式发送，网口发送数据到FPGA，直到检测到指令反馈成功或者等待超时才退出。
	* num 网口编号，从0开始
	* msg 发送信息
	* msgLength 数据长度
	* flags 标志位
	* sleepTime 发送指令后程序Sleep时间，单位：ms
	* maxWaitingTime 最大等待时间，单位：s
	* isShow TRUE:即保存日志信息，也在界面显示日志信息。 FALSE：只保存日志信息，不在界面显示日志信息。
	*/
	BOOL BackSend(int num, BYTE *msg, int msgLength, int flags, 
		int sleepTime = 1, int maxWaitingTime = 1, BOOL isShow = FALSE);
	
	/*非阻塞式发送，不进行指令反馈检测
	* num 网口编号，从0开始
	* msg 发送信息
	* msgLength 数据长度
	* flags 标志位
	* sleepTime 发送指令后程序Sleep时间，单位：ms
	*/
	void NoBackSend(int num, BYTE* msg, int msgLength, int flags,
		int sleepTime = 1);

	CClientSocket* m_UDPSocket; //本地UDP服务

	SOCKET SocketList[4];
	BOOL NetSwitchList[5]; // 网络开关,其中0位置对应总开关

	// 单个包：512能谱=516*4字节，（单个包长=516*4*16=33024字节,10ms刷新，10秒测量时长对应总包长=100*10*516*4=）
	// 16通道=20*4字节（1ms刷新，10秒测量时长对应总包长=1000*10*20*4）
	const int DataMaxlen;
	BOOL connectStatusList[4]; // 各网络联网状态
	BOOL UDPStatus; // UDP工作状态
	int MeasureMode[4]; // 测量模式。0:非测量状态，1:软件触发模式，2：硬件触发模式（带硬件触发反馈）。用于处理数据内容判别（指令反馈/测量数据）。
	BOOL AutoMeasureStatus; // 自动测量状态
	BOOL GetDataStatus; // 是否接受到TCP网口的数据
	BOOL m_getTargetChange; // 检测炮号是否变化
	BOOL sendStopFlag; // 用来告知是否发送停止指令的标志，防止重复发送停止指令
	
	// ------------------TCP网络指令反馈的相关变量--------------------------------
	BOOL ifFeedback[4]; //用于判断当前接收数据是否为指令反馈。
	BOOL TCPfeedback[4]; // 发送数据后，网口指令反馈状态.无正确反馈则禁止发送下一条指令。
	char* LastSendMsg[4]; // 上一次发送的指令
	char* RecvMsg[4]; // 网口接收数据
	int recievedFBLength[4]; //已接收网口数据长度，取前N个字节
	int FeedbackLen[4]; //指令反馈字节长度

	// -------------------TCP网络接受数据相关变量----------------------------
	char* DataCH1; // 网口接收的数据，缓存下来，接收完后再存储到文件中。
	char* DataCH2;
	char* DataCH3;
	char* DataCH4;
	int RECVLength[4];//网口已接收数据长度

	int timer; // 计时器，满测量时长后则发送停止测量
	CString saveAsPath; // 数据存储根路径
	CString saveAsTargetPath; // 数据存储炮号路径
	CStatusBar m_statusBar; // 状态栏

	RunningLog m_page1;
	UDP_RecieveLog m_page2;
	CRect m_rect;
	CLayout m_layout;
	int m_currentTab; //Tab子窗口序号

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
	afx_msg void OnSize(UINT nType, int cx, int cy); //变化后的尺寸
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect); //鼠标拖动时的尺寸变化
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

	afx_msg void OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedCalibration();
	afx_msg void OnCbnSelchangeWaveMode(); //选择能谱模式
	afx_msg void OnBnClickedCheck0(); //网络选择，总开关
	afx_msg void OnBnClickedCheck1(); //网络选择，CH1
	afx_msg void OnBnClickedCheck2(); //网络选择，CH2
	afx_msg void OnBnClickedCheck3(); //网络选择，CH3
	afx_msg void OnBnClickedCheck4(); //网络选择，CH4

	// 网址IP
	CIPAddressCtrl ServerIP;
	// 网络状态LED灯
	LEDButton m_NetStatusLEDList[4];

	// TCP端口号
	int PortList[4];

	// 触发方式下拉框
	CComboBox m_TriggerType;
	// 能谱模式选择下拉框
	CComboBox m_WaveMode;
	// 炮号，一种打靶序列号
	CString m_targetID;
	// UDP端口号
	int m_UDPPort;
	CTabCtrl m_Tab;
	// 界面输入的能谱总测量时间,ms
	int MeasureTime;
	// 界面输入的能谱刷新时间，ms
	int RefreshTime;
};
