﻿
// Xrays_64ChannelDlg.cpp: 实现文件
//
#include "json/json.h"

#include "pch.h"
#include "framework.h"
#include "Xrays_64Channel.h"
#include "Xrays_64ChannelDlg.h"
#include "afxdialogex.h"
#include "Order.h"
#include "Log.h"
#include "LayoutInit.h"

//定时器时间间隔
const int TIMER_INTERVAL = 100;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CMutex Mutex; //mutex，线程锁

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CXrays_64ChannelDlg 对话框
CXrays_64ChannelDlg::CXrays_64ChannelDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_XRAYS64CHANNEL_DIALOG, pParent)
	, m_UDPSocket(NULL)
	, DataMaxlen(5000)
	, UDPStatus(FALSE)
	, AutoMeasureStatus(FALSE)
	, GetDataStatus(FALSE)
	, m_getTargetChange(FALSE)
	, sendStopFlag(FALSE)
	, timer(0)
	, saveAsPath("")
	, saveAsTargetPath("")
	, m_currentTab(0)
	, m_targetID(_T("00000"))
	, m_UDPPort(12100)
	, MeasureTime(3000)
	, RefreshTime(10)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_NUCLEAR); //设置主界面图标
	DataCH1 = new char[DataMaxlen];
	DataCH2 = new char[DataMaxlen];
	DataCH3 = new char[DataMaxlen];
	DataCH4 = new char[DataMaxlen];
	for(int num=0; num<4; num++){
		RECVLength[num]=0;
		PortList[num] = 5000;
		connectStatusList[num] = FALSE;
		SocketList[num] = NULL;
		NetSwitchList[num] = TRUE; // 默认打开所有网络
		ifFeedback[num] = FALSE;
		TCPfeedback[num] = FALSE;
		LastSendMsg[num] = NULL;
		RecvMsg[num] = NULL;
		recievedFBLength[num] = 0;
		FeedbackLen[num] = 12;
		MeasureMode[num] = 0;
	}
	NetSwitchList[4] = TRUE;
	CLog::WriteMsg(_T("打开软件，软件环境初始化！"));
}

CXrays_64ChannelDlg::~CXrays_64ChannelDlg()
{
	CLog::WriteMsg(_T("正在退出软件，释放相关资源！"));

	for(int num = 0; num<4; num++){
		if(connectStatusList[num]) {
			connectStatusList[num] = FALSE; // 用来控制关闭线程
			closesocket(SocketList[num]); //关闭套接字
		}
	}
	
	if (UDPStatus) CloseUDP();
	if (m_UDPSocket != NULL) delete m_UDPSocket;
	delete DataCH1;
	delete DataCH2;
	delete DataCH3;
	delete DataCH4;

	delete m_page1;
	delete m_page2;
	KillTimer(4);
	CLog::WriteMsg(_T("退出软件，软件关闭成功！"));
}

// 绑定变量与控件
void CXrays_64ChannelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LED, m_NetStatusLEDList[0]);		// “建立链接”LED
	DDX_Control(pDX, IDC_LED2, m_NetStatusLEDList[1]);		// “建立链接”LED
	DDX_Control(pDX, IDC_LED3, m_NetStatusLEDList[2]);		// “建立链接”LED
	DDX_Control(pDX, IDC_LED4, m_NetStatusLEDList[3]);		// “建立链接”LED
	DDX_Control(pDX, IDC_IPADDRESS1, ServerIP);
	DDX_Text(pDX, IDC_PORT1, PortList[0]);
	DDX_Text(pDX, IDC_PORT2, PortList[1]);
	DDX_Text(pDX, IDC_PORT3, PortList[2]);
	DDX_Text(pDX, IDC_PORT4, PortList[3]);
	DDX_Control(pDX, IDC_COMBO1, m_TriggerType);
	DDX_Control(pDX, IDC_WAVE_MODE, m_WaveMode);
	DDX_Text(pDX, IDC_TargetNum, m_targetID);
	DDX_Text(pDX, IDC_UDPPORT, m_UDPPort);
	DDX_Control(pDX, IDC_TAB1, m_Tab);
	DDX_Text(pDX, IDC_MeasureTime, MeasureTime);
	DDX_Text(pDX, IDC_RefreshTimeEdit, RefreshTime);
	DDX_Check(pDX, IDC_CHECK1, NetSwitchList[0]);
	DDX_Check(pDX, IDC_CHECK2, NetSwitchList[1]);
	DDX_Check(pDX, IDC_CHECK3, NetSwitchList[2]);
	DDX_Check(pDX, IDC_CHECK4, NetSwitchList[3]);
	DDX_Check(pDX, IDC_CHECK5, NetSwitchList[4]);
}

BEGIN_MESSAGE_MAP(CXrays_64ChannelDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_BN_CLICKED(IDC_CONNECT1, &CXrays_64ChannelDlg::OnConnect)
	ON_EN_KILLFOCUS(IDC_PORT1, &CXrays_64ChannelDlg::OnEnKillfocusPort1)
	ON_EN_KILLFOCUS(IDC_PORT2, &CXrays_64ChannelDlg::OnEnKillfocusPort2)
	ON_EN_KILLFOCUS(IDC_PORT3, &CXrays_64ChannelDlg::OnEnKillfocusPort3)
	ON_EN_KILLFOCUS(IDC_PORT4, &CXrays_64ChannelDlg::OnEnKillfocusPort4)
	ON_EN_KILLFOCUS(IDC_UDPPORT, &CXrays_64ChannelDlg::OnEnKillfocusUDPPort)
	ON_EN_KILLFOCUS(IDC_RefreshTimeEdit, &CXrays_64ChannelDlg::OnEnKillfocusRefreshTime)
	ON_EN_KILLFOCUS(IDC_MeasureTime, &CXrays_64ChannelDlg::OnEnKillfocusMeasureTime)
	ON_BN_CLICKED(IDC_Start, &CXrays_64ChannelDlg::OnBnClickedStart)
	ON_BN_CLICKED(IDC_AutoMeasure, &CXrays_64ChannelDlg::OnBnClickedAutomeasure)
	ON_BN_CLICKED(IDC_SaveAs, &CXrays_64ChannelDlg::OnBnClickedSaveas)
	ON_BN_CLICKED(IDC_CLEAR_LOG, &CXrays_64ChannelDlg::OnBnClickedClearLog)
	ON_BN_CLICKED(IDC_UDP_BUTTON, &CXrays_64ChannelDlg::OnBnClickedUdpButton)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CXrays_64ChannelDlg::OnTcnSelchangeTab1)
	ON_BN_CLICKED(IDC_CALIBRATION, &CXrays_64ChannelDlg::OnBnClickedCalibration)
	ON_CBN_SELCHANGE(IDC_WAVE_MODE, &CXrays_64ChannelDlg::OnCbnSelchangeWaveMode)
	ON_BN_CLICKED(IDC_CHECK1, &CXrays_64ChannelDlg::OnBnClickedCheck0)
	ON_BN_CLICKED(IDC_CHECK2, &CXrays_64ChannelDlg::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_CHECK3, &CXrays_64ChannelDlg::OnBnClickedCheck2)
	ON_BN_CLICKED(IDC_CHECK4, &CXrays_64ChannelDlg::OnBnClickedCheck3)
	ON_BN_CLICKED(IDC_CHECK5, &CXrays_64ChannelDlg::OnBnClickedCheck4)
END_MESSAGE_MAP()


// CXrays_64ChannelDlg 消息处理程序

BOOL CXrays_64ChannelDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。
	
	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	InitLayout(m_layout, this);
	//UpdateData(TRUE); //表示写数据，将窗口控制变量写入内存（更新数据）
	UpdateData(FALSE); //表示读数据，即显示窗口读取内存的数据以供实时显示
	
	//---------------初始化状态栏---------------
	InitBarSettings();

	//----------------------------Tab窗口-------------
	InitTabSettings();

	//-------------------读取配置参数，初始化界面其他控件-----------
	InitOtherSettings();

	CLog::WriteMsg(_T("软件加载完成！"));
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}


void CXrays_64ChannelDlg::InitBarSettings(){
	CRect rectDlg;
	GetClientRect(rectDlg);//获得窗体的大小
	// 添加状态栏
	UINT nID[] = { 1001,1002,1003 };
	//创建状态栏
	m_statusBar.Create(this);
	//添加状态栏面板，参数为ID数组和面板数量
	m_statusBar.SetIndicators(nID, sizeof(nID) / sizeof(UINT));
	//设置面板序号，ID，样式和宽度，SBPS_NORMAL为普通样式，固定宽度，SBPS_STRETCH为弹簧样式，会自动扩展它的空间
	m_statusBar.SetPaneInfo(0, 1001, SBPS_NORMAL, int(0.6 * rectDlg.Width()));
	m_statusBar.SetPaneInfo(1, 1002, SBPS_STRETCH, int(0.2 * rectDlg.Width()));
	m_statusBar.SetPaneInfo(2, 1003, SBPS_NORMAL, int(0.2 * rectDlg.Width()));
	//设置状态栏位置
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
	//设置状态栏面板文本，参数为面板序号和对应文本
	m_statusBar.SetPaneText(0, L"aaa");
	m_statusBar.SetPaneText(1, L"bbb");
	m_statusBar.SetPaneText(2, L"ccc");
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
	//开启定时器，刷新状态栏参数
	SetTimer(4, 1000, NULL);
}


void CXrays_64ChannelDlg::InitTabSettings(){

	//为Tab Control增加两个页面   
	m_Tab.InsertItem(0, _T("系统运行日志"));
	m_Tab.InsertItem(1, _T("UDP运行日志"));

	//创建两个对话框   
	m_page1.Create(IDD_RunningLog, &m_Tab);
	m_page2.Create(IDD_UDP_RecieveLog, &m_Tab);
	
	//设定在Tab内显示的范围,子控件大小
	CRect rc;
	m_Tab.GetClientRect(&rc);
	int itemHigh = 25;
	TabCtrl_SetItemSize(m_Tab, 150, itemHigh); //标题的尺寸
	rc.top += itemHigh;
	rc.bottom -= 5;
	rc.left += 3;
	rc.right -= 10;

	m_page1.MoveWindow(&rc);
	m_page2.MoveWindow(&rc);

	//显示初始页面   
	m_page1.ShowWindow(TRUE);
	m_page2.ShowWindow(FALSE);

	m_Tab.SetCurSel(0);
	GetClientRect(&m_rect);
}


void CXrays_64ChannelDlg::InitOtherSettings(){
	for(int num=0; num<4; num++){
		m_NetStatusLEDList[num].RefreshWindow(FALSE);//设置指示灯
	}
	
	// 设置下拉框默认选项
	m_TriggerType.SetCurSel(0); 
	m_WaveMode.SetCurSel(0);

	CString strPath = GetExeDir();
	if (IsPathExit(strPath)) {
		saveAsPath = strPath;
		CString str = saveAsPath += "\\";
		CString info = _T("实验数据默认保存路径：") + str;
		m_page1.PrintLog(info);
	}
	else {
		CString info = _T("获取实验数据默认保存路径失败");
		m_page1.PrintLog(info);
	}
	
	// ------------------读取配置参数并设置到相应控件上---------------------
	CString StrSerIp = _T("192.168.10.22");
	CString StrSerIp2 = _T("192.168.10.22");
	CString StrSerIp3 = _T("192.168.10.22");
	CString StrSerIp4 = _T("192.168.10.22");
	Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
	if (!jsonSetting.isNull()) {
		StrSerIp = jsonSetting["IP_Detector1"].asCString();
		StrSerIp2 = jsonSetting["IP_Detector2"].asCString();
		StrSerIp3 = jsonSetting["IP_Detector3"].asCString();
		StrSerIp4 = jsonSetting["IP_Detector4"].asCString();

		PortList[0] = jsonSetting["Port_Detector1"].asInt();
		PortList[1] = jsonSetting["Port_Detector2"].asInt();
		PortList[2] = jsonSetting["Port_Detector3"].asInt();
		PortList[3] = jsonSetting["Port_Detector4"].asInt();

		m_UDPPort = jsonSetting["Port_UDP"].asInt();
	}
	SetDlgItemText(IDC_IPADDRESS1, StrSerIp);
	SetDlgItemText(IDC_IPADDRESS2, StrSerIp2);
	SetDlgItemText(IDC_IPADDRESS3, StrSerIp3);
	SetDlgItemText(IDC_IPADDRESS4, StrSerIp4);

	SetDlgItemInt(IDC_PORT1, PortList[0]);
	SetDlgItemInt(IDC_PORT2, PortList[1]);
	SetDlgItemInt(IDC_PORT3, PortList[2]);
	SetDlgItemInt(IDC_PORT4, PortList[3]);

	SetDlgItemInt(IDC_UDPPORT, m_UDPPort);
	
	// ---------------设置部分按钮初始化使能状态-------------
	GetDlgItem(IDC_Start)->EnableWindow(FALSE);
	GetDlgItem(IDC_AutoMeasure)->EnableWindow(FALSE);
	GetDlgItem(IDC_CALIBRATION)->EnableWindow(FALSE);
}


void CXrays_64ChannelDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CXrays_64ChannelDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CXrays_64ChannelDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// 连接网络
void CXrays_64ChannelDlg::OnConnect()
{
	UpdateData(TRUE); //是控件变量的值与界面编辑值同步
	// TODO: 在此添加控件通知处理程序代码
	// 停用连接按钮，防止用户连续点击多次
	GetDlgItem(IDC_CONNECT1)->EnableWindow(FALSE);
	SetTCPInputStatus(FALSE);
	
	CString strTemp;
	GetDlgItemText(IDC_CONNECT1, strTemp);
	
	BOOL AllconnectStatus = TRUE;
	if (strTemp == _T("连接网络")) {
		CLog::WriteMsg(_T("点击“连接网络按钮”，尝试连接TCP网络！"));
		
		// 1、尝试建立网络
		for (int num = 0; num < 4; num++) {
			if(NetSwitchList[num+1]) {
				connectStatusList[num] = ConnectTCP(num);
			}
		}
		
		// 2、判断连接状态
		for (int num = 0; num < 4; num++) {
			if(connectStatusList[num] != NetSwitchList[num+1]) AllconnectStatus = FALSE;
		}

		// 3、连接成功
		if (AllconnectStatus) {
			SetDlgItemText(IDC_CONNECT1, _T("断开网络"));
			
			// 开启线程接收数据
			AfxBeginThread(&Recv_Th1, 0); 
			AfxBeginThread(&Recv_Th2, 0); 
			AfxBeginThread(&Recv_Th3, 0); 
			AfxBeginThread(&Recv_Th4, 0); 

			GetDlgItem(IDC_Start)->EnableWindow(TRUE);
			// 必须TCP和UDP同时工作才能使用自动测量
			if (UDPStatus) {
				GetDlgItem(IDC_AutoMeasure)->EnableWindow(TRUE);
			}
			m_page1.PrintLog(_T("TCP网络全部连接成功！"));
		}
		else {
			// 断开连接成功的网口
			for (int num = 0; num < 4; num++) {
				if(connectStatusList[num]) {
					connectStatusList[num] = FALSE; // 用来控制关闭线程
					closesocket(SocketList[num]); // 关闭套接字
					m_NetStatusLEDList[num].RefreshWindow(FALSE);// 关闭指示灯
				}
			}

			// 恢复各个输入框使能状态
			SetTCPInputStatus(TRUE);
			GetDlgItem(IDC_Start)->EnableWindow(FALSE);
			GetDlgItem(IDC_AutoMeasure)->EnableWindow(FALSE);
			m_page1.PrintLog(_T("TCP网络未能全部连接成功！"));
		}
	}
	else{
		// 1、断开连接成功的套接字
		for(int num=0; num<4; num++){
			if(connectStatusList[num]) {
				connectStatusList[num] = FALSE; // 用来控制关闭线程
				closesocket(SocketList[num]); // 关闭套接字
				m_NetStatusLEDList[num].RefreshWindow(FALSE);// 关闭指示灯
			}
		} 

		SetDlgItemText(IDC_CONNECT1, _T("连接网络"));

		// 恢复各个输入框使能状态
		SetTCPInputStatus(TRUE);
		GetDlgItem(IDC_Start)->EnableWindow(FALSE);
		GetDlgItem(IDC_AutoMeasure)->EnableWindow(FALSE);
		m_page1.PrintLog(_T("TCP网络(CH1,CH2,CH3,CH4)已断开"));
	}
	// 恢复各个输入框使能状态
	GetDlgItem(IDC_CONNECT1)->EnableWindow(TRUE); // 恢复按钮使能
}

BOOL CXrays_64ChannelDlg::ConnectTCP(int num){
	// 1、创建套接字
	SocketList[num] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (SocketList[num] == INVALID_SOCKET) {
		CString info;
		info.Format(_T("设备%d网络初始化创建失败！"), num+1);
		m_page1.PrintLog(info);
		return FALSE;
	}

	// 2、连接服务器
	CString StrSerIp;
	char keyIP[] = "IP_Detector1";
	char keyPort[] = "Port_Detector1";
	char keyRecvLen[] = "RECVLength_CH1";
	
	keyIP[strlen(keyIP)-1] = '1' + num;
	keyPort[strlen(keyPort)-1] = '1' + num;
	keyRecvLen[strlen(keyRecvLen)-1] = '1' + num;
	switch(num)
	{
	case 0:
		GetDlgItemText(IDC_IPADDRESS1, StrSerIp);
		break;
	case 1:
		GetDlgItemText(IDC_IPADDRESS2, StrSerIp);
		break;
	case 2:
		GetDlgItemText(IDC_IPADDRESS3, StrSerIp);
		break;
	case 3:
		GetDlgItemText(IDC_IPADDRESS4, StrSerIp);
		break;		
	default:
		break;
	}
	char* pStrIP = CstringToWideCharArry(StrSerIp);

	// 写入配置文件
	Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
	jsonSetting[keyIP] = pStrIP;
	jsonSetting[keyPort] = PortList[num];
	jsonSetting[keyRecvLen] = 0;
	WriteSetting(_T("Setting.json"), jsonSetting);
	
	// 3、设置网络参数
	sockaddr_in server_addr;
	inet_pton(AF_INET, pStrIP, (void*)&server_addr.sin_addr.S_un.S_addr);
	server_addr.sin_family = AF_INET;  // 使用IPv4地址
	server_addr.sin_port = htons(PortList[num]); //网关：5000
	SetSocketSize(SocketList[num], 1048576*3); //1M=1024k=1048576字节，缓存区大小
	
	// 4、检测网络是否连接,以及显示设备联网状况
	if (connect(SocketList[num], (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
		m_NetStatusLEDList[num].RefreshWindow(FALSE);//打开指示灯
		// 打印日志
		CString info;
		info.Format(_T("CH%d网络连接失败。请重新尝试，若再次连接失败,请做以下尝试:\n\
            1、检查网络参数设置是否正确；\n2、检查设备电源是否打开；\n\
			3、检查电脑的网络适配器属性设置是否正确\n"), num+1);
		m_page1.PrintLog(info);
		return FALSE;
	}
	CString info;
	info.Format(_T("CH%d网络已连接"), num+1);
	m_page1.PrintLog(info);

	m_NetStatusLEDList[num].RefreshWindow(TRUE);//打开指示灯
	return TRUE;
}

//线程1，接收TCP网口数据
UINT Recv_Th1(LPVOID p)
{
	CSingleLock singleLock(&Mutex); //线程锁
	CXrays_64ChannelDlg* dlg = (CXrays_64ChannelDlg*)AfxGetApp()->GetMainWnd();
	while (1)
	{
		// 断开网络后关闭本线程
		if (!dlg->connectStatusList[0]) return 0;
		const int dataLen = 10000; //接收的数据包长度
		char mk[dataLen];
		int nLength;
		nLength = recv(dlg->SocketList[0], mk, dataLen, 0); //阻塞模式
		
		if (nLength == -1) 
		{
			return 0;
		}
		else {
			// 提取指令反馈数据,只取前12字节
			if(dlg->ifFeedback[0]){
				int receLen = 0; //本次接受总反馈指令长度
				int receivedLen = dlg->recievedFBLength[0]; //上一次已接收数据长度
				
				// 计算本次及之前接受到的反馈指令字节数
				if (receivedLen + nLength < dlg->FeedbackLen[0]) {
					receLen = receivedLen + nLength;
				}
				else {
					receLen = dlg->FeedbackLen[0];
				}

				char* tempChar = (char*)malloc(receLen);
				//先取旧数据
				if (dlg->RecvMsg[0] != NULL) {
					for (int i = 0; i < receivedLen; i++) {
						tempChar[i] = *(dlg->RecvMsg[0] + i);
					}
				}

				// 拼接新数据
				if (receivedLen + nLength < dlg->FeedbackLen[0]) {
					// 拼接反馈指令
					for (int i = 0; i < nLength; i++) {
						tempChar[receivedLen + i] = mk[i];
					}
					nLength = 0;
				} 
				else{
					// 先拼反馈指令
					int remainLen = 12 - receivedLen; //剩余拼接长度
					for (int i = 0; i < remainLen; i++) {
						tempChar[receivedLen + i] = mk[i];
					}
					
					// 再处理剩余字符数组
					nLength = nLength - remainLen;
					for (int i = 0; i < nLength; i++) {
						mk[i] = mk[remainLen+i];
					}
				}

				dlg->RecvMsg[0] = tempChar;
				dlg->recievedFBLength[0] = receLen;
				if (receLen == dlg->FeedbackLen[0]) {
					dlg->ifFeedback[0] = FALSE; //接收完12字节，重置标志位
				}
			}

			if (nLength < 1) continue; //提前结束本次循环

			// 普通数据
			CString fileName = dlg->m_targetID + _T("CH1");
			dlg->SaveFile(fileName, mk, nLength);
			dlg->AddTCPData(1, mk, nLength);

			// MeasureMode=2,硬件触发信号反馈
			if(dlg->MeasureMode[0] == 2){
				if(nLength==12 && strncmp(mk, (char *)Order::HardTriggerBack, nLength) == 0){
					dlg->MeasureMode[0] = 0;
					CString info = _T("已收到硬件触发信号,CH1 RECV HEX:") + Char2HexCString((char*)mk, nLength);
					dlg->m_page1.PrintLog(info,FALSE);
				}
				continue;
			}

			// 有效测量数据开始
			singleLock.Lock(); //Mutex
			if (singleLock.IsLocked()){
				dlg->GetDataStatus = TRUE; //线程锁的变量
			}
			singleLock.Unlock(); //Mutex
		}
	}
	return 0;
}

//线程2，接收TCP网口数据
UINT Recv_Th2(LPVOID p)
{
	CSingleLock singleLock(&Mutex); //线程锁
	CXrays_64ChannelDlg* dlg = (CXrays_64ChannelDlg*)AfxGetApp()->GetMainWnd();
	while (1)
	{
		// 断开网络后关闭本线程
		if (!dlg->connectStatusList[1]) return 0;

		const int dataLen = 10000; //接收的数据包长度
		char mk[dataLen];
		int nLength;
		nLength = recv(dlg->SocketList[1], mk, dataLen, 0);
		if (nLength == -1) //超过recvTimeout不再有数据，关闭该线程
		{
			return 0;
		}
		else {
			// 提取指令反馈数据,只取前12字节
			if(dlg->ifFeedback[1]){
				int receLen = 0; //本次接受总反馈指令长度
				int receivedLen = dlg->recievedFBLength[1]; //上一次已接收数据长度
				
				// 计算本次及之前接受到的反馈指令字节数
				if (receivedLen + nLength < dlg->FeedbackLen[1]) {
					receLen = receivedLen + nLength;
				}
				else {
					receLen = dlg->FeedbackLen[1];
				}

				char* tempChar = (char*)malloc(receLen);
				//先取旧数据
				if (dlg->RecvMsg[1] != NULL) {
					for (int i = 0; i < receivedLen; i++) {
						tempChar[i] = *(dlg->RecvMsg[1] + i);
					}
				}

				// 拼接新数据
				if (receivedLen + nLength < dlg->FeedbackLen[1]) {
					// 拼接反馈指令
					for (int i = 0; i < nLength; i++) {
						tempChar[receivedLen + i] = mk[i];
					}
					nLength = 0;
				} 
				else{
					// 先拼反馈指令
					int remainLen = 12 - receivedLen; //剩余拼接长度
					for (int i = 0; i < remainLen; i++) {
						tempChar[receivedLen + i] = mk[i];
					}
					
					// 再处理剩余字符数组
					nLength = nLength - remainLen;
					for (int i = 0; i < nLength; i++) {
						mk[i] = mk[remainLen+i];
					}
				}

				dlg->RecvMsg[1] = tempChar;
				dlg->recievedFBLength[1] = receLen;
				if (receLen == dlg->FeedbackLen[1]) {
					dlg->ifFeedback[1] = FALSE; //接收完12字节，重置标志位
				}
			}

			if (nLength < 1) continue; //提前结束本次循环

			// 普通数据
			CString fileName = dlg->m_targetID + _T("CH2");
			dlg->SaveFile(fileName, mk, nLength);
			dlg->AddTCPData(2, mk, nLength);

			// MeasureMode=2,硬件触发信号反馈
			if(dlg->MeasureMode[1] == 2){
				if(nLength==12 && strncmp(mk, (char *)Order::HardTriggerBack, nLength) == 0){
					dlg->MeasureMode[1] = 0;
					CString info = _T("已收到硬件触发信号,CH2 RECV HEX:") + Char2HexCString((char*)mk, nLength);
					dlg->m_page1.PrintLog(info,FALSE);
				}
				continue;
			}

			// 有效测量数据开始
			singleLock.Lock(); //Mutex
			if (singleLock.IsLocked())
			{
				dlg->GetDataStatus = TRUE;	
			}
			singleLock.Unlock(); //Mutex
		}
	}
	return 0;
}

//线程3，接收TCP网口数据
UINT Recv_Th3(LPVOID p)
{
	CSingleLock singleLock(&Mutex); //线程锁
	CXrays_64ChannelDlg* dlg = (CXrays_64ChannelDlg*)AfxGetApp()->GetMainWnd();
	while (1)
	{
		// 断开网络后关闭本线程
		if (!dlg->connectStatusList[2]) return 0;

		const int dataLen = 10000; //接收的数据包长度
		char mk[dataLen];

		int nLength;
		nLength = recv(dlg->SocketList[2], mk, dataLen, 0);
		if (nLength == -1) //超过recvTimeout不再有数据，关闭该线程
		{
			return 0;
		}
		else {
			// 提取指令反馈数据,只取前12字节
			if(dlg->ifFeedback[2]){
				int receLen = 0; //本次接受总反馈指令长度
				int receivedLen = dlg->recievedFBLength[2]; //上一次已接收数据长度
				
				// 计算本次及之前接受到的反馈指令字节数
				if (receivedLen + nLength < dlg->FeedbackLen[2]) {
					receLen = receivedLen + nLength;
				}
				else {
					receLen = dlg->FeedbackLen[2];
				}

				char* tempChar = (char*)malloc(receLen);
				//先取旧数据
				if (dlg->RecvMsg[2] != NULL) {
					for (int i = 0; i < receivedLen; i++) {
						tempChar[i] = *(dlg->RecvMsg[2] + i);
					}
				}

				// 拼接新数据
				if (receivedLen + nLength < dlg->FeedbackLen[2]) {
					// 拼接反馈指令
					for (int i = 0; i < nLength; i++) {
						tempChar[receivedLen + i] = mk[i];
					}
					nLength = 0;
				} 
				else{
					// 先拼反馈指令
					int remainLen = 12 - receivedLen; //剩余拼接长度
					for (int i = 0; i < remainLen; i++) {
						tempChar[receivedLen + i] = mk[i];
					}
					
					// 再处理剩余字符数组
					nLength = nLength - remainLen;
					for (int i = 0; i < nLength; i++) {
						mk[i] = mk[remainLen+i];
					}
				}

				dlg->RecvMsg[2] = tempChar;
				dlg->recievedFBLength[2] = receLen;
				if (receLen == dlg->FeedbackLen[2]) {
					dlg->ifFeedback[2] = FALSE; //接收完12字节，重置标志位
				}
			}

			if (nLength < 1) continue; //提前结束本次循环

			// 普通数据
			CString fileName = dlg->m_targetID + _T("CH3");
			dlg->SaveFile(fileName, mk, nLength);
			dlg->AddTCPData(3, mk, nLength);

			// MeasureMode=2,硬件触发信号反馈
			if(dlg->MeasureMode[2] == 2){
				if(nLength==12 && strncmp(mk, (char *)Order::HardTriggerBack, nLength) == 0){
					dlg->MeasureMode[2] = 0;
					CString info = _T("已收到硬件触发信号,CH3 RECV HEX:") + Char2HexCString((char*)mk, nLength);
					dlg->m_page1.PrintLog(info,FALSE);
				}
				continue;
			}
			
			// 有效测量数据开始
			singleLock.Lock(); //Mutex
			if (singleLock.IsLocked()){
				dlg->GetDataStatus = TRUE;
			}
			singleLock.Unlock(); //Mutex
		}
	}
	return 0;
}

//线程4，接收TCP网口数据
UINT Recv_Th4(LPVOID p)
{
	CSingleLock singleLock(&Mutex); //线程锁
	CXrays_64ChannelDlg* dlg = (CXrays_64ChannelDlg*)AfxGetApp()->GetMainWnd();
	while (1)
	{
		// 断开网络后关闭本线程
		if (!dlg->connectStatusList[3]) return 0;
		const int dataLen = 10000; //接收的数据包长度
		char mk[dataLen];

		int nLength;
		nLength = recv(dlg->SocketList[3], mk, dataLen, 0); //阻塞模式，如果没有数据，则一直等待
		if (nLength == -1) { 
			return 0;
		}
		else {
			// 提取指令反馈数据,只取前12字节
			if(dlg->ifFeedback[3]){
				int receLen = 0; //本次接受总反馈指令长度
				int receivedLen = dlg->recievedFBLength[3]; //上一次已接收数据长度
				
				// 计算本次及之前接受到的反馈指令字节数
				if (receivedLen + nLength < dlg->FeedbackLen[3]) {
					receLen = receivedLen + nLength;
				}
				else {
					receLen = dlg->FeedbackLen[3];
				}

				char* tempChar = (char*)malloc(receLen);
				//先取旧数据
				if (dlg->RecvMsg[3] != NULL) {
					for (int i = 0; i < receivedLen; i++) {
						tempChar[i] = *(dlg->RecvMsg[3] + i);
					}
				}

				// 拼接新数据
				if (receivedLen + nLength < dlg->FeedbackLen[3]) {
					// 拼接反馈指令
					for (int i = 0; i < nLength; i++) {
						tempChar[receivedLen + i] = mk[i];
					}
					nLength = 0;
				} 
				else{
					// 先拼反馈指令
					int remainLen = 12 - receivedLen; //剩余拼接长度
					for (int i = 0; i < remainLen; i++) {
						tempChar[receivedLen + i] = mk[i];
					}
					
					// 再处理剩余字符数组
					nLength = nLength - remainLen;
					for (int i = 0; i < nLength; i++) {
						mk[i] = mk[remainLen+i];
					}
				}

				dlg->RecvMsg[3] = tempChar;
				dlg->recievedFBLength[3] = receLen;
				if (receLen == dlg->FeedbackLen[3]) {
					dlg->ifFeedback[3] = FALSE; //接收完12字节，重置标志位
				}
			}

			if (nLength < 1) continue; //提前结束本次循环

			// 普通数据
			CString fileName = dlg->m_targetID + _T("CH4");
			dlg->SaveFile(fileName, mk, nLength);
			dlg->AddTCPData(4, mk, nLength);

			// MeasureMode=2,硬件触发信号反馈
			if(dlg->MeasureMode[3] == 2){
				if(nLength==12 && strncmp(mk, (char *)Order::HardTriggerBack, nLength) == 0){
					dlg->MeasureMode[3] = 0;
					CString info = _T("已收到硬件触发信号,CH3 RECV HEX:") + Char2HexCString((char*)mk, nLength);
					dlg->m_page1.PrintLog(info,FALSE);
				}
				continue;
			}
			
			// 有效测量数据开始
			singleLock.Lock(); //Mutex
			if (singleLock.IsLocked()){
				dlg->GetDataStatus = TRUE;
			}
			singleLock.Unlock(); //Mutex
		}
	}
	return 0;
}

//定时器
void CXrays_64ChannelDlg::OnTimer(UINT_PTR nIDEvent) {
	// 计时MeasureTime=3000ms。定时结束后向网口发送停止指令
	switch (nIDEvent)
	{
	case 1:
		//开始测量(手动测量)模式
		if (GetDataStatus) {
			timer++;

			//状态栏显示
			CString strInfo;
			strInfo.Format(_T("Data Length:CH1= %d,CH2=%d,CH3=%d,CH4=%d"),
				RECVLength[0], RECVLength[1], RECVLength[2], RECVLength[3]);
			m_statusBar.SetPaneText(0, strInfo);

			if (timer * TIMER_INTERVAL > MeasureTime)
			{
				if (!sendStopFlag) {
					for(int num=0; num<4; num++){
						if(connectStatusList[num]) NoBackSend(num, Order::Stop, 12, 0, 1);
					}

					sendStopFlag = TRUE;
					// 打印日志
					CString info;
					info.Format(_T("测量时间：%dms, 已发送停止测量指令,请耐心等待数据完全接收！"), MeasureTime);
					m_page1.PrintLog(info);
				}
				Json::Value jsonSetting = ReadSetting(_T("Setting.json"));

				//在这规定时间内四个线程均没有接收到新的数据，即全部stop了
				if (RECVLength[0] == jsonSetting["RECVLength_CH1"].asInt() 
					&& RECVLength[1] == jsonSetting["RECVLength_CH2"].asInt()
				 	&& RECVLength[2] == jsonSetting["RECVLength_CH3"].asInt() 
					&& RECVLength[3] == jsonSetting["RECVLength_CH4"].asInt())
				{
					SetDlgItemText(IDC_Start, _T("开始测量"));
					timer = 0;
					GetDataStatus = FALSE;
					for(int num=0; num<4; num++){
						MeasureMode[num] = 0;
					}
					sendStopFlag = FALSE;

					//往TCP发送的控制板配置参数允许输入
					SetParameterInputStatus(TRUE);

					// 按键互斥锁,测量结束，恢复各按钮使能
					GetDlgItem(IDC_SaveAs)->EnableWindow(TRUE); //设置文件路径
					GetDlgItem(IDC_AutoMeasure)->EnableWindow(TRUE); //自动测量
					GetDlgItem(IDC_CONNECT1)->EnableWindow(TRUE); //连接网络
					GetDlgItem(IDC_UDP_BUTTON)->EnableWindow(TRUE); //连接UDP网络
					GetDlgItem(IDC_SaveAs)->EnableWindow(TRUE); //设置文件路径
					GetDlgItem(IDC_CALIBRATION)->EnableWindow(TRUE); //刻度曲线

					if (RECVLength[0] + RECVLength[1] + RECVLength[2] + RECVLength[3] > 0) {
						// 打印日志
						CString info = _T("实验数据存储路径：") + saveAsTargetPath;
						m_page1.PrintLog(info);
						info.Format(_T("Data Length(字节) : CH1 = % d, CH2 = % d, CH3 = % d, CH4 = % d"),
							RECVLength[0], RECVLength[1], RECVLength[2], RECVLength[3]);
						m_page1.PrintLog(info);
					}
					KillTimer(1);	//测量结束，关闭定时器
				}
			}			
			
			if (timer % 10 == 0)//每间隔10个timer记录一次
			{	// 写入配置文件
				Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
				jsonSetting["RECVLength_CH1"] = RECVLength[0];
				jsonSetting["RECVLength_CH2"] = RECVLength[1];
				jsonSetting["RECVLength_CH3"] = RECVLength[2];
				jsonSetting["RECVLength_CH4"] = RECVLength[3];
				WriteSetting(_T("Setting.json"), jsonSetting);
			}
		}
		break;
	case 2:
		// 自动测量定时器:硬件触发后3秒定时发送关闭指令
		if (GetDataStatus) {
			timer++;

			//状态栏显示
			CString strInfo;
			strInfo.Format(_T("Data Length:CH1= %d,CH2=%d,CH3=%d,CH4=%d"),
				RECVLength[0], RECVLength[1], RECVLength[2], RECVLength[3]);
			m_statusBar.SetPaneText(0, strInfo);

			if (timer * TIMER_INTERVAL >= MeasureTime) {
				if (!sendStopFlag) {
					for(int num=0; num<4; num++){
						if(connectStatusList[num]) NoBackSend(num, Order::Stop, 12, 0, 1);
					}

					sendStopFlag = TRUE;

					// 打印日志
					CString info;
					info.Format(_T("测量时间：%dms, 已发送停止测量指令,请耐心等待数据完全接收！"), MeasureTime);
					m_page1.PrintLog(info);
				}

				//在规定时间内四个线程均没有接收到新的数据，即全部stop了
				Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
				if (RECVLength[0] == jsonSetting["RECVLength_CH1"].asInt()
					&& RECVLength[1] == jsonSetting["RECVLength_CH2"].asInt()
					&& RECVLength[2] == jsonSetting["RECVLength_CH3"].asInt()
					&& RECVLength[3] == jsonSetting["RECVLength_CH4"].asInt())
				{
					// 重置部分数据
					timer = 0;
					for(int num=0; num<4; num++){
						MeasureMode[num] = 0;
					}
					GetDataStatus = FALSE;
					sendStopFlag = FALSE;

					if (RECVLength[0] + RECVLength[1] + RECVLength[2] + RECVLength[3]> 0) {
						// 打印日志
						CString info = _T("实验数据存储路径：") + saveAsTargetPath;
						m_page1.PrintLog(info);
						info.Format(_T("Data Length : CH1 = % d, CH2 = % d, CH3 = % d, CH4 = % d"),
							RECVLength[0], RECVLength[1], RECVLength[2], RECVLength[3]);
						m_page1.PrintLog(info);
					}
				}
			}

			if (timer % 10 == 0)//每间隔10个timer记录一次
			{	// 写入配置文件
				Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
				jsonSetting["RECVLength_CH1"] = RECVLength[0];
				jsonSetting["RECVLength_CH2"] = RECVLength[1];
				jsonSetting["RECVLength_CH3"] = RECVLength[2];
				jsonSetting["RECVLength_CH4"] = RECVLength[3];
				WriteSetting(_T("Setting.json"), jsonSetting);
			}
		}
		break;
	case 3:
		// 炮号检测定时器：检测炮号是否刷新
		if (m_getTargetChange) {
			//刷新炮号相关显示以及日志
			m_getTargetChange = FALSE;
			CString info = _T("炮号已刷新：") + m_targetID;
			m_page1.PrintLog(info);
			UpdateData(FALSE);

			//若处于自动测量状态，创建对应炮号文件夹，并发送开始测量指令
			if (AutoMeasureStatus) {
				//创建对应炮号文件夹
				CString pathPostfix;
				//(.4表示将占用4位，如果数字超过4位将输出所有数字，不会截断)
				pathPostfix.Format(_T("_%.3d_%.4d"), RefreshTime, MeasureTime/1000);
				saveAsTargetPath = saveAsPath + m_targetID + pathPostfix;
				saveAsTargetPath += "\\";
				Mkdir(saveAsTargetPath);

				// 发送停止指令，复位。以保证把上一次测量重置。	
				for(int num=0; num<4; num++){
					MeasureMode[num] = 0;
					if(connectStatusList[num]) BackSend(num, Order::Stop, 12, 0, 1); //这里带指令反馈检测
				}			
				
				// 重置从网口接收的缓存数据
				ResetTCPData();
				//重置接受数据标志位
				GetDataStatus = FALSE;
				//重置接受数据计数器
				timer = 0;
				//重置测量状态
				m_getTargetChange = FALSE;
				sendStopFlag = FALSE;

				//发送开始测量指令，采用硬件触发
				for (int num = 0; num < 4; num++) {
					if(connectStatusList[num]) BackSend(num, Order::StartHardTrigger, 12, 0, 1);
					MeasureMode[num] = 2;
				}
				
				CString info = _T("\"硬件触发\"工作模式");
				m_page1.PrintLog(info);
			}
		}
		break;
	case 4:
		//状态栏，软件界面的一些常规参数刷新
		{
			CTime t= CTime::GetCurrentTime();
			CString strInfo = t.Format(_T("%Y-%m-%d %H:%M:%S"));
			m_statusBar.SetPaneText(2, strInfo);
		}
		break;
	default:
		break;
	}
	CDialogEx::OnTimer(nIDEvent);
}

//开始测量——手动测量，采用软件触发方式工作
void CXrays_64ChannelDlg::OnBnClickedStart()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_Start)->EnableWindow(FALSE);

	CString strTemp;
	GetDlgItemText(IDC_Start, strTemp);
	if (strTemp == _T("开始测量")) {
		timer = 0;
		GetDataStatus = FALSE;
		for(int num = 0; num < 4; num++){
			MeasureMode[num] = 0;
		}
		//向TCP发送配置参数。
		SendParameterToTCP(); 
		//TCP发送的控制板配置参数禁止输入
		SetParameterInputStatus(FALSE);
		//重置网口接收的数据
		ResetTCPData();

		//向TCP发送开始指令
		for (int num = 0; num < 4; num++) {
			if(connectStatusList[num]) BackSend(num, Order::StartSoftTrigger, 12, 0, 1);
			MeasureMode[num] = 1;
		}

		SetDlgItemText(IDC_Start, _T("停止测量"));
		
		//开启定时器，第1个参数表示ID号，第二个参数表示刷新时间ms
		SetTimer(1, TIMER_INTERVAL, NULL); 

		CTime t = CTime::GetCurrentTime();
		// 打印日志
		CString info;
		info = _T("\r\n开始测量（手动测量），软件触发，开始时间：");
		info += t.Format(_T("%Y-%m-%d %H:%M:%S"));
		m_page1.PrintLog(info);

		CString strInfo = t.Format(_T("%Y-%m-%d_%H-%M-%S"));
		saveAsTargetPath = saveAsPath + strInfo;
		saveAsTargetPath += "\\";
		Mkdir(saveAsTargetPath);

		info = _T("测试数据存储路径:") + saveAsTargetPath;
		m_page1.PrintLog(info);

		// 按键互斥锁
		GetDlgItem(IDC_SaveAs)->EnableWindow(FALSE); //设置文件路径
		GetDlgItem(IDC_AutoMeasure)->EnableWindow(FALSE); //自动测量
		GetDlgItem(IDC_CONNECT1)->EnableWindow(FALSE); //连接网络
		GetDlgItem(IDC_UDP_BUTTON)->EnableWindow(FALSE); //连接UDP网络
		GetDlgItem(IDC_SaveAs)->EnableWindow(FALSE); //设置文件路径
		GetDlgItem(IDC_CALIBRATION)->EnableWindow(FALSE); //刻度曲线
	}
	else {
		for(int num=0; num<4; num++){
			MeasureMode[num] = 0;
		}
		//往TCP发送的控制板配置参数允许输入
		SetParameterInputStatus(TRUE);

		//往TCP发送停止指令
		for(int num=0; num<4; num++){
			if(connectStatusList[num]) NoBackSend(num, Order::Stop, 12, 0, 1);
		}

		SetDlgItemText(IDC_Start, _T("开始测量"));
		KillTimer(1);	//关闭定时器

		//打印日志
		CString info;
		info = _T("\r\n已停止（手动）测量\r\n");
		m_page1.PrintLog(info);

		// 按键互斥锁
		GetDlgItem(IDC_SaveAs)->EnableWindow(TRUE); //设置文件路径
		GetDlgItem(IDC_AutoMeasure)->EnableWindow(TRUE); //自动测量
		GetDlgItem(IDC_CONNECT1)->EnableWindow(TRUE); //连接网络
		GetDlgItem(IDC_UDP_BUTTON)->EnableWindow(TRUE); //连接UDP网络
		GetDlgItem(IDC_SaveAs)->EnableWindow(TRUE); //设置文件路径
		GetDlgItem(IDC_CALIBRATION)->EnableWindow(TRUE); //刻度曲线
	}
	
	GetDlgItem(IDC_Start)->EnableWindow(TRUE);
}

//自动测量按钮
void CXrays_64ChannelDlg::OnBnClickedAutomeasure()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);

	GetDlgItem(IDC_AutoMeasure)->EnableWindow(FALSE);

	CString strTemp;
	GetDlgItemText(IDC_AutoMeasure, strTemp);
	if (strTemp == _T("自动测量")) {
		// 打印日志
		CString info;
		info = _T("\r\n开始自动测量。。。");
		m_page1.PrintLog(info);

		AutoMeasureStatus = TRUE;
		// 重新初始化部分数据
		timer = 0;
		for(int num=0; num<4; num++){
			MeasureMode[num] = 0;
		}
		GetDataStatus = FALSE;
		m_getTargetChange = FALSE;
		for(int num=0; num<4; num++){
			ifFeedback[num] = FALSE;
			TCPfeedback[num] = FALSE;
			LastSendMsg[num] = NULL;
			RecvMsg[num] = NULL;
			recievedFBLength[num] = 0;
			FeedbackLen[num] = 12;
			MeasureMode[num] = 0;
		}

		SendParameterToTCP();
		SetParameterInputStatus(FALSE);
		SetDlgItemText(IDC_AutoMeasure, _T("停止测量"));

		//开启定时器，第1个参数表示ID号，第二个参数表示刷新时间ms
		SetTimer(2, TIMER_INTERVAL, NULL);  

		// 锁死其他相关按键
		GetDlgItem(IDC_Start)->EnableWindow(FALSE); //手动测量
		GetDlgItem(IDC_CONNECT1)->EnableWindow(FALSE); //连接TCP网络
		GetDlgItem(IDC_UDP_BUTTON)->EnableWindow(FALSE); //连接UDP网络
		GetDlgItem(IDC_SaveAs)->EnableWindow(FALSE); //设置文件路径
		GetDlgItem(IDC_CALIBRATION)->EnableWindow(FALSE); //刻度曲线
	}
	else {
		AutoMeasureStatus = FALSE;
		SetParameterInputStatus(TRUE);
		
		// 发送停止指令，带指令反馈，确保上一次测量结束。
		for(int num=0; num<4; num++){
			if(connectStatusList[num]) BackSend(num, Order::Stop, 12, 0, 1);
		}
		SetDlgItemText(IDC_AutoMeasure, _T("自动测量"));

		//关闭定时器
		KillTimer(2);	

		// 打开其他相关按键使能
		GetDlgItem(IDC_Start)->EnableWindow(TRUE); //手动测量
		GetDlgItem(IDC_CONNECT1)->EnableWindow(TRUE); //连接网络
		GetDlgItem(IDC_UDP_BUTTON)->EnableWindow(TRUE); //连接UDP网络
		GetDlgItem(IDC_SaveAs)->EnableWindow(TRUE); //设置文件路径
		GetDlgItem(IDC_CALIBRATION)->EnableWindow(TRUE); //刻度曲线

		// 打印日志
		CString info;
		info = _T("\r\n已停止自动测量。。。\r\n");
		m_page1.PrintLog(info);
	}

	GetDlgItem(IDC_AutoMeasure)->EnableWindow(TRUE);
}

// UDP网络连接与断开
void CXrays_64ChannelDlg::OnBnClickedUdpButton()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strTemp;
	GetDlgItemText(IDC_UDP_BUTTON, strTemp);
	if (strTemp == _T("开启UDP网络")) {
		//打开定时器3，用来刷新炮号
		SetTimer(3, TIMER_INTERVAL, NULL);
		OpenUDP();
		SetDlgItemText(IDC_UDP_BUTTON, _T("断开UDP网络"));
	}
	else {
		CloseUDP();
		SetDlgItemText(IDC_UDP_BUTTON, _T("开启UDP网络"));
	}
}

// 获取刻度曲线数据文件的全路径
void CXrays_64ChannelDlg::OnBnClickedCalibration()
{
	CString fileName(_T(""));
	if(!ChooseFile(fileName)) return;

	SendCalibration(fileName);
}

//发送刻度曲线数据
//fileName为发送文件路径（绝对路径）
void CXrays_64ChannelDlg::SendCalibration(CString fileName)
{
	CString info;
	info = _T("刻度曲线指令发送中。。。");
	m_page1.PrintLog(info);

	BOOL sendStatus = TRUE; // 刻度曲线发送是否成功

	vector<CString> messageList = ReadEnCalibration(fileName);
	BYTE cmd[2000] = { 0 }; //144条指令*12字节=1728字节
	BYTE cmd2[2000] = { 0 };
	BYTE cmd3[2000] = { 0 };
	BYTE cmd4[2000] = { 0 };
	int iSize = 0;
	
	iSize = Str2Hex(messageList[0], cmd);
	Str2Hex(messageList[1], cmd2);
	Str2Hex(messageList[2], cmd3);
	Str2Hex(messageList[3], cmd4);
	// 将各个指令分开发送
	for (int i = 0; i < iSize / 12; i++) {
		BYTE temp[13] = { 0 };
		BYTE temp2[13] = { 0 };
		BYTE temp3[13] = { 0 };
		BYTE temp4[13] = { 0 };
		for (int j = 0; j < 12; j++) {
			temp[j] = cmd[i * 12 + j];
			temp2[j] = cmd2[i * 12 + j];
			temp3[j] = cmd3[i * 12 + j];
			temp4[j] = cmd4[i * 12 + j];
		}

		// 若当前是联网状态，则发送数据
		if (connectStatusList[0]) sendStatus = sendStatus & BackSend(0, temp, 12, 0, 2, 10, FALSE);
		if (connectStatusList[1]) sendStatus = sendStatus & BackSend(1, temp2, 12, 0, 2, 10, FALSE);
		if (connectStatusList[2]) sendStatus = sendStatus & BackSend(2, temp3, 12, 0, 2, 10, FALSE);
		if (connectStatusList[3]) sendStatus = sendStatus & BackSend(3, temp4, 12, 0, 2, 10, FALSE);
	}
	if(sendStatus){
		CString info;
		info = _T("刻度曲线指令发送成功");
		m_page1.PrintLog(info);
	}
	else{
		CString info;
		info = _T("刻度曲线指令发送失败");
		m_page1.PrintLog(info);
	}
}

//选择能谱模式
void CXrays_64ChannelDlg::OnCbnSelchangeWaveMode()
{
	// TODO: 在此添加控件通知处理程序代码
}
