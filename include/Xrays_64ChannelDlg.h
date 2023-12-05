
// Xrays_64ChannelDlg.h: 头文件
//

#pragma once

#include "json/json.h"
#include "LEDButton.h"
#include "UDP_Socket.h"
#include "UDP_Log.h"
#include "RunningLog.h"
#include "MyConst.h"

//自定义消息，必须大于100，100以内系统自己使用
#define WM_UPDATE_ARM (WM_USER + 100)
#define WM_UPDATE_TRIGER_LOG (WM_USER + 120)
#define WM_UPDATE_CH_DATA (WM_USER + 130)
#define WM_UPDATE_SHOT (WM_USER + 140)  //炮号刷新
#define WM_UPDATE_RELAY (WM_USER + 150)  //继电器状态查询刷新
using namespace std;

UINT Recv_Th1(LPVOID p); // 多线程接收CH1网口数据
UINT Recv_Th2(LPVOID p); // 多线程接收CH2网口数据
UINT Recv_Th3(LPVOID p); // 多线程接收CH3网口数据
UINT Recv_Th4(LPVOID p); // 多线程接收CH4网口数据
UINT Recv_ARM(LPVOID p); // 多线程接收ARM网口数据
UINT Recv_Relay(LPVOID p); // 多线程接收网口数据(继电器)
UINT Recv_Thread(const int num, LPVOID p); //四个探测器接受数据线程公有部分

//线程锁
extern CMutex Mutex; 
//定时器时间间隔
extern const int TIMER_INTERVAL;

// CXrays_64ChannelDlg 对话框
class CXrays_64ChannelDlg : public CDialogEx
{
// 构造
public:
	CXrays_64ChannelDlg(CWnd* pParent = nullptr);	// 标准构造函数
	virtual ~CXrays_64ChannelDlg();

	//初始化状态栏
	void InitBarSettings(); 
	//初始化Tab对话框
	void InitTabSettings(); 
	// 初始化界面其他控件参数
	void InitOtherSettings(); 
	//重新绘制状态栏
	void ResizeBar(); 
	//打开UDP通信
	void OpenUDP(); 
	//关闭UDP通信，以及相应资源
	void CloseUDP();
	//温度、电压电流监测打开
	BOOL TempVoltMonitorON();
	//温度、电压电流监测关闭
	void TempVoltMonitorOFF();

	//----------------其他----------------
	//连接一般TCP网络
	BOOL ConnectGeneralTCP(SOCKET& my_socket, CString strIP, int port); 
	//设置Socket缓冲区的大小
	void SetSocketSize(SOCKET &sock, int nsize); 
	
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

	//-------------------四个探测器网络相关设置函数--------------
	//发送配置参数
	void SendParameterToTCP(); 
	//发送刻度曲线
	void SendCalibration(CString fileName); 
	//连接探测器设备网络
	BOOL ConnectTCP(int num); 
	//继电器连接
	BOOL ConnectRelayTCP();	
	//限制端口号输入范围
	void ConfinePortRange(int &myPort); 
	//设置TCP的IP、PORT、复选框的输入使能状态
	void SetTCPInputStatus(BOOL flag); 
	//设置配置参数框的使能状态
	void SetParameterInputStatus(BOOL flag); 
	/* 保存文件
		fileName:文件名
		mk: 待存储的字符串
		length：字符串长度
	*/
	void SaveFile(CString fileName, char* mk, int length); 
	// 缓存网口数据
	void AddTCPData(int num, BYTE* tempChar, int len); 
	// 重置缓存数据
	void ResetTCPData(); 

	//----------------------ARM有关的网络相关函数--------------------
	//刷新状态栏中通道1,温度，电压/电流
	void refreshBar(); 

	/*解析ARM返回的温度设备1、温度设备2、电流、电压数据，根据包头包尾判断
		输入：
			packge：网口新接收的数据包
		返回：未判断到完整的包头包尾时，返回温度为-1000
	*/
	void GetARMData();//GetARMData(CByteArray& packge)
	
	//保存ARM监测的温度数据
	void SaveEnviromentFile(double data[]);

	//-----------------自定义变量-------------------
public: 
	CUDP_Socket* m_UDPSocket; //本地UDP服务
	BOOL UDPStatus; // UDP工作状态
	SOCKET SocketList[4]; // FPGA的TCP端口，也就是CH1~CH4
	SOCKET relaySocket; //继电器网络套接字	
	BOOL NetSwitchList[5]; // 网络开关,其中0位置对应总开关

	/* 单个包：512能谱=516*4字节，（单个包长=516*4*16=33024字节,10ms刷新，10秒测量时长对应总包长=100*10*516*4=）
		16通道=20*4字节（1ms刷新，10秒测量时长对应总包长=1000*10*20*4）
	*/
	const int DataMaxlen;
	BOOL connectStatusList[4]; // 各网络联网状态
	BOOL netRelayStatus; //继电器联网状态
	int MeasureMode; // 测量状态。0：非测量状态，1：手动测量状态，2：自动测量状态。
	int TrigerMode[4]; // 触发模式。0:非测量状态，1:软件触发模式，2：硬件触发模式（带硬件触发反馈）。用于处理数据内容判别（指令反馈/测量数据）。
	BOOL GetDataStatus; // 是否接受到TCP网口的数据
	BOOL m_getTargetChange; // 检测炮号是否变化
	BOOL sendStopFlag; // 用来告知是否发送停止指令的标志，防止重复发送停止指令
	
	// ------------------TCP网络指令反馈的相关变量--------------------------------
	BOOL ifFeedback[4]; //用于判断当前接收数据是否为指令反馈。
	BOOL TCPfeedback[4]; // 发送数据后，网口指令反馈状态.无正确反馈则禁止发送下一条指令。
	BYTE* LastSendMsg[4]; // 上一次发送的指令
	BYTE* RecvMsg[4]; // 网口接收数据
	int recievedFBLength[4]; //已接收网口数据长度，取前N个字节
	int FeedbackLen[4]; //指令反馈字节长度

	// -------------------TCP网络接受数据相关变量----------------------------
	BYTE* DataCH1; // 网口接收的数据，缓存下来，接收完后再存储到文件中。
	BYTE* DataCH2;
	BYTE* DataCH3;
	BYTE* DataCH4;
	int RECVLength[4];//网口已接收数据长度
	fstream fileDetector[4]; //探测器文件保持常开状态，以减小打频繁开关闭所浪费的时间

	//----------------ARM网络监测温度/电压/电流相关变量-------------
	SOCKET armSocket; // ARM网络的TCP端口
	BOOL ARMnetStatus; //ARM联网状态，无法实时监测，只能在联网后置TRUE，断开连接后置FALSE
	BOOL feedbackARM[3]; //获取到数据包状态，前两个是温度包，最后一个电压电流包
	double temperature[6]; //三个通道的温度
	double powerVolt; //电压
	double powerCurrent; //电流
	int refreshTime_ARM; //温度检测模块刷新数据时间,units: s
	int ArmTimer; //计数器，计算流逝的时间,units:s
	CByteArray TotalARMArray; //ARM网口接收的数据
	
	UINT_PTR m_nTimerId[3]; //3个定时器的状态，0为非工作状态，>0为工作状态
	int timer; // 计时器，满测量时长后则发送停止测量
	CString saveAsPath; // 数据存储根路径
	CString saveAsTargetPath; // 数据存储炮号路径
	CStatusBar m_statusBar; // 状态栏
	
	CMenu menu; //菜单栏
	RunningLog m_page1;
	UDP_Log m_page2;
	CRect m_rect;
	CLayout m_layout;
	int m_currentTab; //Tab子窗口序号

	CWinThread *m_pThread_ARM; //线程函数返回指针，
	CWinThread* m_pThread_CH[4]; //线程函数返回指针

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_XRAYS64CHANNEL_DIALOG };
#endif

	protected:
	//退出窗口，关闭窗口
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	// OnClose()后进入该函数
	virtual BOOL DestroyWindow();
	// 关闭窗口
	afx_msg void OnClose();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	//变化后的尺寸
	afx_msg void OnSize(UINT nType, int cx, int cy); 
	//鼠标拖动时的尺寸变化
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect); 
	//连接探测器网络
	afx_msg void OnConnect(); 
	//限制刷新时间输入范围
	afx_msg void OnEnKillfocusRefreshTime(); 
	//限制能谱测量总时长输入范围
	afx_msg void OnEnKillfocusMeasureTime(); 
	//限制阈值输入范围
	afx_msg void OnEnKillfocusThreshold();
	//开始测量（手动测量）
	afx_msg void OnBnClickedStart(); 
	//自动测量
	afx_msg void OnBnClickedAutomeasure(); 
	//定时器
	afx_msg void OnTimer(UINT_PTR nIDEvent); 
	//保存文件，设置文件存储路径
	afx_msg void OnBnClickedSaveas();
	//清空日志按钮
	afx_msg void OnBnClickedClearLog();
	//UDP开关按钮
	afx_msg void OnBnClickedUDPButton();
	//多页对话框选择
	afx_msg void OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult);
	//选择刻度曲线数据文件，并发送指令到FPGA
	afx_msg void OnBnClickedCalibration();
	//选择能谱模式
	afx_msg void OnCbnSelchangeWaveMode(); 
	//网络选择复选框，总开关
	afx_msg void OnBnClickedCheck0(); 
	//网络选择复选框，CH1
	afx_msg void OnBnClickedCheck1(); 
	//网络选择复选框，CH2
	afx_msg void OnBnClickedCheck2(); 
	//网络选择复选框，CH3
	afx_msg void OnBnClickedCheck3(); 
	//网络选择复选框，CH4
	afx_msg void OnBnClickedCheck4(); 
	//菜单栏的电源设置选项
	// afx_msg void OnPowerMenu(); 
	//菜单栏“设置”按钮
	afx_msg void OnNetSettingMenu();
	// 继电器网络连接按钮
	afx_msg void OnBnClickedRelayConnect();
	//切换继电器开关状态
	afx_msg void OnRelayChange(); 

	//-------------------------自定义消息---------------------
	//从ARM来的网口接受到数据，进行相关数据处理
	afx_msg LRESULT OnUpdateARMStatic(WPARAM wParam, LPARAM lParam);
	// 接收到触发信号，刷新文本日志，可以刷新界面日志显示
	afx_msg LRESULT OnUpdateTrigerLog(WPARAM wParam, LPARAM lParam);
	//接收到数据信号，开启定时器1，进行相关的处理
	afx_msg LRESULT OnUpdateTimer1(WPARAM wParam, LPARAM lParam);
	//UDP接收到炮号数据,更新主界面相关动作
	afx_msg LRESULT OnUpdateShot(WPARAM wParam, LPARAM lParam);
	//继电器查询状态刷新
	afx_msg LRESULT OnUpdateRelay(WPARAM wParam, LPARAM lParam);

	// 网络状态LED灯
	LEDButton m_NetStatusLEDList[4];
	// 继电器网络连接LED灯
	LEDButton m_RelayNetStatusLED;
	// 继电器开关状态，端口是开启还是关闭状态（定时查询）
	LEDButton m_RelayStatusLED;
	
	// 触发方式下拉框
	// CComboBox m_TriggerType;
	// 能谱模式选择下拉框
	CComboBox m_WaveMode;
	// 炮号，一种打靶序列号
	CString m_targetID;
	// 主界面Tab对话框
	CTabCtrl m_Tab;
	// 界面输入的能谱总测量时间,ms
	int MeasureTime;
	// 界面输入的能谱刷新时间，ms
	int RefreshTime;
	//触发阈值
	int m_Threshold;
};
