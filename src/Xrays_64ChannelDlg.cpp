#pragma comment(lib,"Version.lib")
// Xrays_64ChannelDlg.cpp: 实现文件
//
#include "json/json.h"
#include "GitVerison.h"

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
	virtual BOOL OnInitDialog();
	CString GetFileVer();
	CString getGitVersion();
	CString m_strVersion; // 版本号
// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
	m_strVersion = _T("123");
	m_strVersion = GetFileVer();
}

//获取软件当前的版本
//该版本号是在资源文件中的../Version/VS_VERSION_INFO/FILEVERSION的值
CString CAboutDlg::GetFileVer()
{
	TCHAR cPath[200];// szVersionBuffer[200];
	DWORD dwHandle,InfoSize;
	CString strVersion;
	::GetModuleFileName(NULL,cPath,200); //首先获得版本信息资源的长度
	InfoSize = GetFileVersionInfoSize(cPath,&dwHandle); //将版本信息资源读入缓冲区
	if(InfoSize==0) return _T("None VerSion Supprot");
	TCHAR*InfoBuf = new TCHAR[InfoSize];
	GetFileVersionInfo(cPath,0,InfoSize,InfoBuf); //获得生成文件使用的代码页及文件版本
	unsigned int  cbTranslate = 0;
	struct LANGANDCODEPAGE {
	WORD wLanguage;
	WORD wCodePage;
	} *lpTranslate;
	VerQueryValue(InfoBuf, TEXT("\\VarFileInfo\\Translation"),(LPVOID*)&lpTranslate,&cbTranslate);
	// Read the file description for each language and code page.
	for( int i=0; i < (cbTranslate/sizeof(struct LANGANDCODEPAGE)); i++ )
	{
	TCHAR   SubBlock[200];
	wsprintf( SubBlock,
				TEXT("\\StringFileInfo\\%04x%04x\\FileVersion"),
				lpTranslate[i].wLanguage,
				lpTranslate[i].wCodePage);
	void *lpBuffer=NULL;
	unsigned int dwBytes=0;
	VerQueryValue(InfoBuf,
	SubBlock,
	&lpBuffer,
	&dwBytes);
	CString strTemp = (TCHAR *)lpBuffer;
	strVersion+=strTemp;
	}
	return strVersion;
}

CString CAboutDlg::getGitVersion() {
	CString commitHash;
	string commitVer = GIT_VER;
	commitHash = commitVer.c_str();
	return commitHash;
}


void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

BOOL CAboutDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
	
	//设置软件标题名称
	CString AppTitle = _T("垂直硬X射线相机阵列");//默认名称
	Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
	if (!jsonSetting.isNull()) {
		if (jsonSetting.isMember("SoftwareTitle"))
		{
			// AppTitle = jsonSetting["SoftwareTitle"].asCString();
			const char* s  = jsonSetting["SoftwareTitle"].asCString();
			int nLenW = ::MultiByteToWideChar(CP_UTF8, 0, s, -1, NULL, 0);
			wchar_t* wszBuffer = new wchar_t[nLenW];
			::MultiByteToWideChar(CP_UTF8, 0, s, -1, wszBuffer, nLenW);

			// 将 Unicode 编码转换为 GB2312 编码（也就是简体中文编码）
			int nLenA = ::WideCharToMultiByte(CP_ACP, 0, wszBuffer, -1, NULL, 0, NULL, NULL);
			char* szBuffer = new char[nLenA];
			::WideCharToMultiByte(CP_ACP, 0, wszBuffer, -1, szBuffer, nLenA, NULL, NULL);

			// 输出结果
			std::string strResult(szBuffer);
			const char * tmp = strResult.c_str();
			AppTitle = tmp;
		}
		else {
			string pStrTitle = _UnicodeToUtf8(AppTitle);
			// char* pStrTitle = CstringToWideCharArry(AppTitle);
			jsonSetting["SoftwareTitle"] = pStrTitle;
		}
	}
	WriteSetting(_T("Setting.json"), jsonSetting);

	GetDlgItem(IDC_STATIC_VERSION)->SetWindowText(AppTitle); 
	
    // 将版本号设置为静态文本控件的文本
    GetDlgItem(IDC_STATIC_VERSION5)->SetWindowText(m_strVersion); 
	CString commitHash;
	string commitVer = GIT_VER;
	commitHash = commitVer.c_str();
	GetDlgItem(IDC_STATIC_VERSION3)->SetWindowText(commitHash);
    return TRUE;
}

// CXrays_64ChannelDlg 对话框
CXrays_64ChannelDlg::CXrays_64ChannelDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_XRAYS64CHANNEL_DIALOG, pParent)
	, m_UDPSocket(NULL)
	, UDPStatus(FALSE)
	, relaySocket(NULL)
	, DataMaxlen(10)
	, netRelayStatus(FALSE)
	, MeasureMode(0)
	, GetDataStatus(FALSE)
	, m_getTargetChange(FALSE)
	, sendStopFlag(FALSE)
	, armSocket(NULL)
	, ARMnetStatus(FALSE)
	, powerVolt(0.0)
	, powerCurrent(0.0)
	, refreshTime_ARM(30)
	, ArmTimer(0)
	, timer(0)
	, saveAsPath("")
	, saveAsTargetPath("")
	, m_currentTab(0)
	, m_pThread_ARM(NULL)
	, m_targetID(_T("00000"))
	, MeasureTime(3000)
	, m_Threshold(15)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_NUCLEAR); //设置主界面图标
	DataCH1 = new BYTE[DataMaxlen];

	RECVLength =0;
	connectStatus = FALSE;
	SocketList = NULL;
	ifFeedback = FALSE;
	TCPfeedback = FALSE;
	LastSendMsg = NULL;
	RecvMsg = NULL;
	recievedFBLength = 0;
	FeedbackLen = 12;
	TrigerMode = 0;
	m_pThread_CH = NULL;
	for (int i = 0; i < 3; i++) {
		feedbackARM[i] = FALSE;
	}
	for (int i = 0; i < 3; i++) {
		m_nTimerId[i] = 0;
	}
	for (int i = 0; i < 6; i++) {
		temperature[i] = -999.0;
	}

	CLog::WriteMsg(_T("打开软件，软件环境初始化！"));
}

CXrays_64ChannelDlg::~CXrays_64ChannelDlg()
{
	// 保存界面部分参数设置
	Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
	jsonSetting["Measure_Time"] = MeasureTime;
	jsonSetting["Threshold"] = m_Threshold;
	WriteSetting(_T("Setting.json"), jsonSetting);

	CLog::WriteMsg(_T("正在退出软件，释放相关资源！"));

	for (int i = 0; i < 6; i++) {
		temperature[i] = 0;
	}

	for (int i = 0; i < 3; i++) {
		feedbackARM[i] = FALSE;
	}
	
	
	if(connectStatus) {
		connectStatus = FALSE; // 停止线程执行，用来控制关闭线程
		// 关闭打开的文件
		if(fileDetector.is_open()){
			fileDetector.close();
		}
		closesocket(SocketList); //关闭套接字
	}
	
	if(ARMnetStatus) {
		ARMnetStatus = FALSE; //停止线程执行
		//WaitForSingleObject(m_pThread_ARM->m_hThread, INFINITE);   //一直等待线程退出
		//CLog::WriteMsg(_T("ARM线程退出成功！"));
		closesocket(armSocket);
	}
	if(netRelayStatus) {
		netRelayStatus = FALSE; //停止线程执行
		closesocket(relaySocket);
	}
	//if (UDPStatus) CloseUDP();
	if (m_UDPSocket != NULL) delete m_UDPSocket;
	delete DataCH1;

	for(int i=0;i<3;i++){
		if(m_nTimerId[i] > 0) KillTimer(m_nTimerId[i]);
	}

	delete m_page1;
	delete m_page2;
	CLog::WriteMsg(_T("退出软件，软件关闭成功！"));
}

// 绑定变量与控件
void CXrays_64ChannelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LED, m_NetStatusLEDList);
	DDX_Control(pDX, IDC_LED_RELAYNET, m_RelayNetStatusLED);
	DDX_Control(pDX, IDC_LED_POWER, m_RelayStatusLED);
	DDX_Control(pDX, IDC_WORK_MODE, m_WaveMode);
	DDX_Text(pDX, IDC_TargetNum, m_targetID);
	DDX_Control(pDX, IDC_TAB1, m_Tab);
	DDX_Text(pDX, IDC_MeasureTime, MeasureTime);
	DDX_Text(pDX, IDC_CH1Threshold, m_Threshold);
}

BEGIN_MESSAGE_MAP(CXrays_64ChannelDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_BN_CLICKED(IDC_CONNECT1, &CXrays_64ChannelDlg::OnConnect)
	ON_EN_KILLFOCUS(IDC_CH1Threshold, &CXrays_64ChannelDlg::OnEnKillfocusThreshold)
	ON_EN_KILLFOCUS(IDC_MeasureTime, &CXrays_64ChannelDlg::OnEnKillfocusMeasureTime)
	ON_BN_CLICKED(IDC_Start, &CXrays_64ChannelDlg::OnBnClickedStart)
	ON_BN_CLICKED(IDC_AutoMeasure, &CXrays_64ChannelDlg::OnBnClickedAutomeasure)
	ON_BN_CLICKED(IDC_SaveAs, &CXrays_64ChannelDlg::OnBnClickedSaveas)
	ON_BN_CLICKED(IDC_CLEAR_LOG, &CXrays_64ChannelDlg::OnBnClickedClearLog)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CXrays_64ChannelDlg::OnTcnSelchangeTab1)
	ON_COMMAND(ID_NETSETTING_MENU, &CXrays_64ChannelDlg::OnNetSettingMenu)
	ON_COMMAND(ID_HELPVIEW, &CXrays_64ChannelDlg::OnHelpview)
	ON_COMMAND(ID_VERSION, &CXrays_64ChannelDlg::OnAbout)
	ON_BN_CLICKED(IDC_POWER_NET, &CXrays_64ChannelDlg::OnBnClickedRelayConnect)
	ON_BN_CLICKED(IDC_POWER_ONOFF, &CXrays_64ChannelDlg::OnRelayChange)
	ON_MESSAGE(WM_UPDATE_ARM, &CXrays_64ChannelDlg::OnUpdateARMStatic) //子线程发送消息通知主线程处理  
	ON_MESSAGE(WM_UPDATE_CH_DATA, &CXrays_64ChannelDlg::OnUpdateTimer1)
	ON_MESSAGE(WM_UPDATE_SHOT, &CXrays_64ChannelDlg::OnUpdateShot)
	ON_MESSAGE(WM_UPDATE_RELAY, &CXrays_64ChannelDlg::OnUpdateRelay)
	ON_CBN_SELCHANGE(IDC_WORK_MODE, &CXrays_64ChannelDlg::OnCbnSelchangeWaveMode)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CXrays_64ChannelDlg 消息处理程序

BOOL CXrays_64ChannelDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。
	menu.LoadMenu(IDR_MENU1);  //IDR_MENU1为菜单栏ID号  
	SetMenu(&menu);

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
	
	//设置软件标题名称
	CString AppTitle = _T("垂直硬X射线相机阵列");//默认名称
	Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
	if (!jsonSetting.isNull()) {
		if (jsonSetting.isMember("SoftwareTitle"))
		{
			// AppTitle = jsonSetting["SoftwareTitle"].asCString();
			const char* s  = jsonSetting["SoftwareTitle"].asCString();
			int nLenW = ::MultiByteToWideChar(CP_UTF8, 0, s, -1, NULL, 0);
			wchar_t* wszBuffer = new wchar_t[nLenW];
			::MultiByteToWideChar(CP_UTF8, 0, s, -1, wszBuffer, nLenW);

			// 将 Unicode 编码转换为 GB2312 编码（也就是简体中文编码）
			int nLenA = ::WideCharToMultiByte(CP_ACP, 0, wszBuffer, -1, NULL, 0, NULL, NULL);
			char* szBuffer = new char[nLenA];
			::WideCharToMultiByte(CP_ACP, 0, wszBuffer, -1, szBuffer, nLenA, NULL, NULL);

			// 输出结果
			std::string strResult(szBuffer);
			const char * tmp = strResult.c_str();
			AppTitle = tmp;
		}
		else {
			string pStrTitle = _UnicodeToUtf8(AppTitle);
			// char* pStrTitle = CstringToWideCharArry(AppTitle);
			jsonSetting["SoftwareTitle"] = pStrTitle;
		}
	}
	WriteSetting(_T("Setting.json"), jsonSetting);
	SetWindowText(AppTitle);

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	InitLayout(m_layout, this);
	
	//---------------初始化状态栏---------------
	InitBarSettings();

	//----------------------------Tab窗口-------------
	InitTabSettings();

	//-------------------读取配置参数，初始化界面其他控件-----------
	InitOtherSettings();

	//UpdateData(TRUE); //表示写数据，将窗口控制变量写入内存（更新数据）
	UpdateData(FALSE); //表示读数据，即显示窗口读取内存的数据以供实时显示

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
	m_statusBar.SetPaneInfo(0, 1001, SBPS_NORMAL, int(0.45 * rectDlg.Width()));
	m_statusBar.SetPaneInfo(1, 1002, SBPS_STRETCH, int(0.40 * rectDlg.Width()));
	m_statusBar.SetPaneInfo(2, 1003, SBPS_NORMAL, int(0.15 * rectDlg.Width()));
	//设置状态栏位置
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
	//设置状态栏面板文本，参数为面板序号和对应文本
	m_statusBar.SetPaneText(0, _T("探测器数据"));
	m_statusBar.SetPaneText(1, _T("Volt:V,I:A"));
	m_statusBar.SetPaneText(2, _T("日期"));
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
	//开启定时器，刷新状态栏参数
	SetTimer(3, 1000, NULL);
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
	m_NetStatusLEDList.RefreshWindow(FALSE, _T("OFF"));//设置指示灯
	m_RelayNetStatusLED.RefreshWindow(FALSE, _T("OFF"));//设置指示灯
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
	Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
	if (!jsonSetting.isNull()) {
		if (jsonSetting.isMember("Threshold"))
		{
			m_Threshold = jsonSetting["Threshold"].asInt();
		}
		if (jsonSetting.isMember("Measure_Time"))
		{
			MeasureTime = jsonSetting["Measure_Time"].asInt();
		}
		if (jsonSetting.isMember("refreshTime_ARM")) {
			refreshTime_ARM = jsonSetting["refreshTime_ARM"].asInt();
		}

		//设置下拉框默认选项
		m_WaveMode.SetCurSel(0);
		int waveMode = 0;
		if (jsonSetting.isMember("WorkMode"))
		{
			waveMode = jsonSetting["WorkMode"].asInt();
			if(waveMode<2){
				m_WaveMode.SetCurSel(waveMode);
			}
		}
	}
	
	// ---------------设置部分按钮初始化使能状态-------------
	GetDlgItem(IDC_Start)->EnableWindow(FALSE);
	GetDlgItem(IDC_AutoMeasure)->EnableWindow(FALSE);
	GetDlgItem(IDC_POWER_ONOFF)->EnableWindow(FALSE);
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


void CXrays_64ChannelDlg::OnHelpview()
{
	// TODO: 在此添加命令处理程序代码
	CString filePath = _T(".\\help.CHM");
	if(IsFileExit(filePath)){
		ShellExecute(NULL, _T("open"), filePath, NULL, NULL, SW_SHOWMAXIMIZED);
	}
}

void CXrays_64ChannelDlg::OnAbout()
{
	 CAboutDlg dlgAbout;
	 dlgAbout.DoModal();
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

//发送刻度曲线数据
//fileName为发送文件路径（绝对路径）
void CXrays_64ChannelDlg::SendCalibration(CString fileName)
{
	CString info;
	info = _T("刻度曲线指令发送中。。。");
	m_page1.PrintLog(info);

	vector<CString> messageList = ReadEnCalibration(fileName);
	BYTE cmd[2000] = { 0 }; //144条指令*12字节=1728字节
	int iSize = 0;
	
	iSize = Str2Hex(messageList[0], cmd);
	// 将各个指令分开发送
	for (int i = 0; i < iSize / 12; i++) {
		BYTE temp[13] = { 0 };
		for (int j = 0; j < 12; j++) {
			temp[j] = cmd[i * 12 + j];
		}

		// 若当前是联网状态，则发送数据
		if (connectStatus) NoBackSend(temp, 12, 0, 2);
	}
}

// 连接网络
void CXrays_64ChannelDlg::OnConnect()
{
	UpdateData(TRUE); //界面——>控件变量
	// 停用连接按钮，防止用户连续点击多次
	GetDlgItem(IDC_CONNECT1)->EnableWindow(FALSE);
	SetTCPInputStatus(FALSE);
	
	CString strTemp;
	GetDlgItemText(IDC_CONNECT1, strTemp);
	
	BOOL AllconnectStatus = TRUE;
	if (strTemp == _T("数据网络连接")) {
		CLog::WriteMsg(_T("点击“数据网络连接按钮”，尝试连接UDP/TCP网络！"));
		
		// 打开UDP网络
		OpenUDP();
		// 打开温度/电压/电流监测
		BOOL ARMConnectFlag = TempVoltMonitorON();
		// 1、尝试建立网络
		connectStatus = ConnectTCP();

		
		// 2、判断连接状态
		if(!connectStatus) AllconnectStatus = FALSE;


		// 3、连接成功
		if (AllconnectStatus) {
			SetDlgItemText(IDC_CONNECT1, _T("断开网络"));
			
			// 开启线程接收数据
			if(connectStatus) m_pThread_CH = AfxBeginThread(&Recv_Th1, this);

			GetDlgItem(IDC_Start)->EnableWindow(TRUE);
			// 必须TCP和UDP同时工作才能使用自动测量
			if (UDPStatus) {
				GetDlgItem(IDC_AutoMeasure)->EnableWindow(TRUE);
			}
			m_page1.PrintLog(_T("探测器网络连接成功！"));
		}
		else {
			// 断开连接成功的网口
			if(connectStatus) {
				connectStatus = FALSE; // 用来控制关闭线程
				closesocket(SocketList); // 关闭套接字
				m_NetStatusLEDList.RefreshWindow(FALSE, _T("OFF"));// 关闭指示灯
			}
			CloseUDP();
			// 关闭温度/电压/电流监测
			TempVoltMonitorOFF();
			// 恢复各个输入框使能状态
			SetTCPInputStatus(TRUE);
			GetDlgItem(IDC_Start)->EnableWindow(FALSE);
			GetDlgItem(IDC_AutoMeasure)->EnableWindow(FALSE);
			m_page1.PrintLog(_T("探测器网络未能连接成功！"));
		}
	}
	else{
		CloseUDP();
		// 关闭温度/电压/电流监测
		TempVoltMonitorOFF();
		// 1、断开连接成功的套接字
		if(connectStatus) {
			connectStatus = FALSE; // 用来控制关闭线程
			closesocket(SocketList); // 关闭套接字
			m_NetStatusLEDList.RefreshWindow(FALSE, _T("OFF"));// 关闭指示灯
		}

		SetDlgItemText(IDC_CONNECT1, _T("数据网络连接"));

		// 恢复各个输入框使能状态
		SetTCPInputStatus(TRUE);
		GetDlgItem(IDC_Start)->EnableWindow(FALSE);
		GetDlgItem(IDC_AutoMeasure)->EnableWindow(FALSE);
		m_page1.PrintLog(_T("TCP网络(CH1,CH2,CH3)已断开"));
	}
	// 恢复各个输入框使能状态
	GetDlgItem(IDC_CONNECT1)->EnableWindow(TRUE); // 恢复按钮使能
}

BOOL CXrays_64ChannelDlg::ConnectTCP(){
	// 1、创建套接字
	SocketList = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (SocketList == INVALID_SOCKET) {
		CString info=_T("设备网络初始化创建失败！");
		m_page1.PrintLog(info);
		return FALSE;
	}

	// 2、连接服务器
	CString StrSerIp;
	int myPort;
	char keyIP[] = "IP_Detector";
	char keyPort[] = "Port_Detector";
	char keyRecvLen[] = "RECVLength_Detector";
	
	Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
	if (!jsonSetting.isNull()) 
	{
		if(jsonSetting.isMember(keyIP))
		{
			StrSerIp = jsonSetting[keyIP].asCString();
		}
		if(jsonSetting.isMember(keyPort))
		{
			myPort = jsonSetting[keyPort].asInt();
		}
	}

	char* pStrIP = CstringToWideCharArry(StrSerIp);

	// 写入配置文件
	jsonSetting[keyRecvLen] = 0;
	WriteSetting(_T("Setting.json"), jsonSetting);
	
	// 3、设置网络参数
	sockaddr_in server_addr;
	inet_pton(AF_INET, pStrIP, (void*)&server_addr.sin_addr.S_un.S_addr);
	server_addr.sin_family = AF_INET;  // 使用IPv4地址
	server_addr.sin_port = htons(myPort); //网关：5000
	SetSocketSize(SocketList, 1048576*4); //1M=1024k=1048576字节，缓存区大小
	
	// 4、检测网络是否连接,以及显示设备联网状况
	if (connect(SocketList, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
		m_NetStatusLEDList.RefreshWindow(FALSE, _T("OFF"));//设置指示灯
		// 打印日志
		CString info = _T("网络连接失败。请重新尝试，若再次连接失败,请做以下尝试:\
            1、检查网络参数设置是否正确；2、检查设备电源是否打开；\
			3、检查电脑的网络适配器属性设置是否正确");
		m_page1.PrintLog(info);
		return FALSE;
	}
	CString info = _T("网络已连接");
	m_page1.PrintLog(info);

	m_NetStatusLEDList.RefreshWindow(TRUE, _T("ON"));//打开指示灯
	return TRUE;
}

//各个探测器接受数据线程的公有部分
UINT Recv_Thread(LPVOID p)
{
	CSingleLock singleLock(&Mutex); //线程锁
	CXrays_64ChannelDlg* dlg = (CXrays_64ChannelDlg*)p;
	while (1)
	{
		// 断开网络后关闭本线程
		if (!dlg->connectStatus) return 0;

		const int dataLen = 10000; //接收的数据包长度
		BYTE mk[dataLen];

		int nLength;
		nLength = recv(dlg->SocketList, (char*)mk, dataLen, 0);
		if (nLength == -1) //超过recvTimeout不再有数据，关闭该线程
		{
			return 0;
		}
		else {
			// 提取指令反馈数据,只取前12字节
			if (dlg->ifFeedback) {
				int receLen = 0; //本次接受总反馈指令长度
				int receivedLen = dlg->recievedFBLength; //上一次已接收数据长度

				// 计算本次及之前接受到的反馈指令字节数
				if (receivedLen + nLength < dlg->FeedbackLen) {
					receLen = receivedLen + nLength;
				}
				else {
					receLen = dlg->FeedbackLen;
				}

				BYTE* tempChar = (BYTE*)malloc(receLen);
				//先取旧数据
				if (dlg->RecvMsg != NULL) {
					for (int i = 0; i < receivedLen; i++) {
						tempChar[i] = *(dlg->RecvMsg + i);
					}
				}

				// 拼接新数据
				if (receivedLen + nLength < dlg->FeedbackLen) {
					// 拼接反馈指令
					for (int i = 0; i < nLength; i++) {
						tempChar[receivedLen + i] = mk[i];
					}
					nLength = 0;
				}
				else {
					// 先拼反馈指令
					int remainLen = 12 - receivedLen; //剩余拼接长度
					for (int i = 0; i < remainLen; i++) {
						tempChar[receivedLen + i] = mk[i];
					}

					// 再处理剩余字符数组
					nLength = nLength - remainLen;
					for (int i = 0; i < nLength; i++) {
						mk[i] = mk[remainLen + i];
					}
				}

				singleLock.Lock(); //线程锁
				if (singleLock.IsLocked()) {
					dlg->RecvMsg = tempChar;
					dlg->recievedFBLength = receLen;
				}
				singleLock.Unlock();

				if (receLen == dlg->FeedbackLen) {
					singleLock.Lock(); //线程锁
					if (singleLock.IsLocked()) {
						dlg->ifFeedback = FALSE; //接收完12字节，重置标志位
					}
					singleLock.Unlock();
				}
			}

			if (nLength < 1) continue; //提前结束本次循环

			// 普通数据，写入文件
			BOOL writeFlag = FALSE;
			writeFlag  = SaveFile_BYTE_Cache(dlg->fileDetector, mk, nLength);
			if (!writeFlag) {
				// 添加日志到文件
				CLog::SetPrefix(_T("ERROR"));
				CString info;
				info.Format(_T("保存数据失败,丢失数据长度:%d,丢失内容："), nLength);
				info += Char2HexCString(mk, nLength);
				CLog::WriteMsg(info);
			}
			dlg->AddTCPData(mk, nLength);

			// 有效测量数据开始
			singleLock.Lock();
			if (singleLock.IsLocked()) {
				dlg->GetDataStatus = TRUE; //线程锁的变量
			}
			singleLock.Unlock();

			//发送消息通知主界面,进入定时测量状态
			if (dlg->m_nTimerId[0] == 0) {
				::PostMessage(dlg->m_hWnd, WM_UPDATE_CH_DATA, 0, 0);
			}
		}
	}
	return 0;
}

//线程1，接收TCP网口数据
UINT Recv_Th1(LPVOID p)
{
	Recv_Thread(p);
	return 0;
}

//定时器
void CXrays_64ChannelDlg::OnTimer(UINT_PTR nIDEvent) {
	// 计时MeasureTime=3000ms。定时结束后向网口发送停止指令
	CSingleLock singleLock(&Mutex); //线程锁
	switch (nIDEvent)
	{
	case 1:
		{
			timer++;

			//状态栏显示
			CString strInfo;
			strInfo.Format(_T("Receieve data Length:%d"), RECVLength);
			m_statusBar.SetPaneText(0, strInfo);

			if (timer * TIMER_INTERVAL >= MeasureTime){
				if (!sendStopFlag) {
					if (connectStatus) {
						if (m_WaveMode.GetCurSel() == 0) {
							NoBackSend(Order::Stop_Integ, 12, 0, 2);
						}
						else if (m_WaveMode.GetCurSel() == 1) {
							NoBackSend(Order::Stop_Wave, 12, 0, 2);
						}
					}

					sendStopFlag = TRUE;
					// 打印日志
					CString info;
					info.Format(_T("测量时间：%dms, 已发送停止测量指令,请耐心等待数据完全接收！"), MeasureTime);
					m_page1.PrintLog(info);
				}
				
				//在这规定时间内四个线程均没有接收到新的数据，即全部stop了
				Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
				if (RECVLength == jsonSetting["RECVLength_Detector"].asInt())
				{
					// 重置部分数据
					timer = 0;

					singleLock.Lock(); //Mutex线程锁
					if (singleLock.IsLocked()){
						TrigerMode = 0;
						GetDataStatus = FALSE;
					}
					singleLock.Unlock(); //Mutex

					sendStopFlag = FALSE;
					
					// 测量结束，恢复各按钮使能
					if(MeasureMode == 1){
						SetDlgItemText(IDC_Start, _T("手动测量"));
						GetDlgItem(IDC_Start)->EnableWindow(TRUE); //手动测量
						GetDlgItem(IDC_AutoMeasure)->EnableWindow(TRUE); //自动测量
					}
					else if(MeasureMode == 0){ // 表明按下了停止测量按钮
						SetDlgItemText(IDC_AutoMeasure, _T("自动测量"));
						GetDlgItem(IDC_Start)->EnableWindow(TRUE); //手动测量
						GetDlgItem(IDC_AutoMeasure)->EnableWindow(TRUE); //自动测量
					}
					
					//按下停止手动测量或者自动测量。还有一种情况是仍然在自动测量，但是随着炮号刷新一直测量
					if ((MeasureMode == 1) || (MeasureMode == 0)) {
						MeasureMode = 0;
						//往TCP发送的控制板配置参数允许输入
						SetParameterInputStatus(TRUE);
						//打开其他相关按键使能
						GetDlgItem(IDC_SaveAs)->EnableWindow(TRUE); //设置文件路径
						GetDlgItem(IDC_CONNECT1)->EnableWindow(TRUE); //连接网络
					}

					if (RECVLength> 0) {
						// 打印日志
						CString info = _T("实验数据存储路径：") + saveAsTargetPath;
						m_page1.PrintLog(info);
						info.Format(_T("Receieve Data(byte): %d"), RECVLength);
						m_page1.PrintLog(info);
					}
					// 关闭文件指针
					if(connectStatus && fileDetector.is_open()){
						fileDetector.close();
					}
					//重置定时器开关状态
					m_nTimerId[0] = 0;
					m_page1.PrintLog(_T("数据接收完毕，测量结束"));
					KillTimer(1);
				}
			}	
			
			if (timer % 10 == 0)//每间隔10个timer记录一次
			{	// 写入配置文件
				Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
				jsonSetting["RECVLength_Detector"] = RECVLength;
				WriteSetting(_T("Setting.json"), jsonSetting);
			}
		}
		break;
	case 2:
		// 炮号检测定时器：检测炮号是否刷新
		{
			m_getTargetChange = FALSE;
			//若处于自动测量状态，创建对应炮号文件夹，并发送开始测量指令
			if (MeasureMode == 2) {
				//创建对应炮号文件夹
				CString pathPostfix;
				//(.4表示将占用4位，如果数字超过4位将输出所有数字，不会截断)
				pathPostfix.Format(_T("_%.4d"), MeasureTime/1000);
				saveAsTargetPath = saveAsPath + m_targetID + pathPostfix;
				saveAsTargetPath += "\\";
				Mkdir(saveAsTargetPath);
				
				// 先确保上一次炮号的文件指针关闭成功
				if(connectStatus && fileDetector.is_open()){
					fileDetector.close();
				}

				// 打开文件指针
				if(connectStatus) {
					CString strCH;
					strCH.Format(_T("CH%d.dat"), 1);
					CString fileName = saveAsTargetPath + m_targetID + strCH;
					fileDetector.open(fileName, ios::out | ios::app | ios::binary); // 追加  | ios::binary
				}

				// 重置
				singleLock.Lock(); //Mutex
				if (singleLock.IsLocked()){
					TrigerMode = 0;
				}
				singleLock.Unlock(); //Mutex
				// 重置从网口接收的缓存数据
				ResetTCPData();
				
				//重置接受数据标志位					
				singleLock.Lock(); //Mutex线程锁
				if (singleLock.IsLocked()){
					GetDataStatus = FALSE;
				}
				singleLock.Unlock(); //Mutex

				//重置接受数据计数器
				timer = 0;
				//重置测量状态
				m_getTargetChange = FALSE;
				sendStopFlag = FALSE;
				
				//发送开始测量指令，采用硬件触发
				if(connectStatus) {
					if (m_WaveMode.GetCurSel() == 0){
						NoBackSend(Order::StartHardTrigger_Integ, 12, 0);
					}
					if (m_WaveMode.GetCurSel() == 1){
						NoBackSend(Order::StartHardTrigger_Wave, 12, 0);
					}
					
				}
				
				singleLock.Lock(); //Mutex
				if (singleLock.IsLocked()){
					TrigerMode = 2;
				}
				singleLock.Unlock(); //Mutex
				
				m_page1.PrintLog(_T("\"硬件触发\"工作模式，等待触发信号"));
			}
			
			m_nTimerId[1] = 0;
			KillTimer(2);
		}
		break;
	case 3:
		//状态栏，软件界面的一些常规参数刷新
		{
			ArmTimer++;
			CTime t= CTime::GetCurrentTime();
			CString strInfo = t.Format(_T("%Y-%m-%d %H:%M:%S"));
			m_statusBar.SetPaneText(2, strInfo);
			
			if (ARMnetStatus && (ArmTimer >refreshTime_ARM))
			{
				ArmTimer = 0;
				send(armSocket, (char*)Order::ARM_Temperature1, 12, 0);
			}
			// 继电器状态查询
			if(netRelayStatus)
			{
				send(relaySocket, (char*)Order::relay_GetStatus, 10, 0);
			}
			else{
				m_RelayStatusLED.RefreshWindow(2, _T("Unknow"));
			}
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
	
	CSingleLock singleLock(&Mutex); //线程锁

	CString strTemp;
	GetDlgItemText(IDC_Start, strTemp);
	if (strTemp == _T("手动测量")) {
		timer = 0;
		MeasureMode = 1;
		m_nTimerId[0] = 0;
		sendStopFlag = FALSE;

		singleLock.Lock(); // 线程锁
		if (singleLock.IsLocked()){
			TrigerMode = 0;
			GetDataStatus = FALSE;
		}
		singleLock.Unlock();
		
		//向TCP发送配置参数。
		SendParameterToTCP(); 
		//TCP发送的控制板配置参数禁止输入
		SetParameterInputStatus(FALSE);
		//重置网口接收的数据
		ResetTCPData();
		
		//状态栏更新显示
		CString strBarInfo;
		strBarInfo.Format(_T("Receieve data Length:%d"), RECVLength);
		m_statusBar.SetPaneText(0, strBarInfo);

		// 生成文件夹名称
		CTime t = CTime::GetCurrentTime();
		// 打印日志
		CString info;
		info = _T("\r\n开始测量（手动测量），软件触发，开始时间：");
		info += t.Format(_T("%Y-%m-%d %H:%M:%S"));
		m_page1.PrintLog(info);
		// 创建文件夹
		CString strInfo = t.Format(_T("%Y-%m-%d_%H-%M-%S"));
		saveAsTargetPath = saveAsPath + strInfo;
		saveAsTargetPath += "\\";
		Mkdir(saveAsTargetPath);

		info = _T("测试数据存储路径:") + saveAsTargetPath;
		m_page1.PrintLog(info);
		
		//打开文件，准备存储
		if(connectStatus) {
			CString strCH;
			strCH.Format(_T("CH%d.dat"), 1);
			CString fileName = saveAsTargetPath + m_targetID + strCH;
			fileDetector.open(fileName, ios::out | ios::app | ios::binary); // 追加  | ios::binary
		}

		//向TCP发送开始指令
		if(connectStatus) {
			if (m_WaveMode.GetCurSel() == 0){
				NoBackSend(Order::StartSoftTrigger_Integ, 12, 0, 1);
			}
			if (m_WaveMode.GetCurSel() == 1){
				NoBackSend(Order::StartSoftTrigger_Wave, 12, 0, 1);
			}
		}
		singleLock.Lock();
		if (singleLock.IsLocked()){
			TrigerMode = 1;
		}
		singleLock.Unlock();

		SetDlgItemText(IDC_Start, _T("停止测量"));
		
		// 按键互斥锁
		GetDlgItem(IDC_SaveAs)->EnableWindow(FALSE); //设置文件路径
		GetDlgItem(IDC_AutoMeasure)->EnableWindow(FALSE); //自动测量
		GetDlgItem(IDC_CONNECT1)->EnableWindow(FALSE); //连接网络
		GetDlgItem(IDC_SaveAs)->EnableWindow(FALSE); //设置文件路径
		
		//恢复按键
		GetDlgItem(IDC_Start)->EnableWindow(TRUE);
	}
	else {
		singleLock.Lock(); //Mutex
		if (singleLock.IsLocked()){
			TrigerMode = 0;
		}
		singleLock.Unlock(); //Mutex

		//往TCP发送停止指令
		if(connectStatus) {
			if (m_WaveMode.GetCurSel() == 0){
				NoBackSend(Order::Stop_Integ, 12, 0, 2);
			}
			else if (m_WaveMode.GetCurSel() == 1){
				NoBackSend(Order::Stop_Wave, 12, 0, 2);
			}
		}

		sendStopFlag = TRUE;
		if (GetDataStatus) {
			//打印日志
			m_page1.PrintLog(_T("\r\n已停止（手动）测量，请等待剩余数据接收完毕\r\n"));

			// 这里不能直接KillTimer，因为在发送停止指令后还会有剩余数据。
			// 通过强制让计数器满值，来使其进入收尾状态，接收停止指令后的剩余数据。
			timer = (int)floor(MeasureTime/TIMER_INTERVAL);
		}
		else{
		// 未接收到数据，直接结束测量，恢复系统状态
			// 恢复控件使能
			GetDlgItem(IDC_SaveAs)->EnableWindow(TRUE); //设置文件路径
			GetDlgItem(IDC_Start)->EnableWindow(TRUE); //手动测量
			GetDlgItem(IDC_AutoMeasure)->EnableWindow(TRUE); //自动测量
			GetDlgItem(IDC_CONNECT1)->EnableWindow(TRUE); //连接网络
			GetDlgItem(IDC_SaveAs)->EnableWindow(TRUE); //设置文件路径
			SetParameterInputStatus(TRUE);
			SetDlgItemText(IDC_Start, _T("手动测量"));
			
			// 完成测量，关闭文件
			if(connectStatus && fileDetector.is_open()){
				fileDetector.close();
			}

			// 打印日志
			CString info = _T("实验数据存储路径：") + saveAsTargetPath;
			m_page1.PrintLog(info);
			info.Format(_T("Receieve Data(byte): %d"), RECVLength);
			m_page1.PrintLog(info);
			m_page1.PrintLog(_T("\r\n已停止（手动）测量\r\n"));
		}
		

	}
}

//自动测量按钮
void CXrays_64ChannelDlg::OnBnClickedAutomeasure()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_AutoMeasure)->EnableWindow(FALSE);
	UpdateData(TRUE);

	CSingleLock singleLock(&Mutex); //线程锁

	CString strTemp;
	GetDlgItemText(IDC_AutoMeasure, strTemp);
	if (strTemp == _T("自动测量")) {
		// 打印日志
		m_page1.PrintLog(_T("\r\n开始自动测量。。。"));

		// 重新初始化部分数据
		MeasureMode = 2;
		timer = 0;
		m_nTimerId[0] = 0;
		sendStopFlag = TRUE;

		singleLock.Lock(); // 线程锁
		if (singleLock.IsLocked()){
			TrigerMode = 0;
			ifFeedback = FALSE; //接收完12字节，重置标志位
			GetDataStatus = FALSE;
		}
		singleLock.Unlock();

		m_getTargetChange = FALSE;
		TCPfeedback = FALSE;
		LastSendMsg = NULL;
		RecvMsg = NULL;
		recievedFBLength = 0;
		FeedbackLen = 12;


		SendParameterToTCP();
		SetParameterInputStatus(FALSE);
		SetDlgItemText(IDC_AutoMeasure, _T("停止测量"));

		// 锁死其他相关按键
		GetDlgItem(IDC_Start)->EnableWindow(FALSE); //手动测量
		GetDlgItem(IDC_CONNECT1)->EnableWindow(FALSE); //连接网络
		GetDlgItem(IDC_SaveAs)->EnableWindow(FALSE); //设置文件路径

		// 恢复按键
		GetDlgItem(IDC_AutoMeasure)->EnableWindow(TRUE);
	}
	else {
		MeasureMode = 0;
		singleLock.Lock();
		if (singleLock.IsLocked()){
			TrigerMode = 0;
		}
		singleLock.Unlock(); 

		// 若处于接收数据状态,则利用定时器进行自动关闭复位
		if (GetDataStatus) {
			// 发送停止指令，结束上一次测量结束。
			if (connectStatus) {
				if (m_WaveMode.GetCurSel() == 0){
					NoBackSend(Order::Stop_Integ, 12, 0, 2);
				}
				else if (m_WaveMode.GetCurSel() == 1){
					NoBackSend(Order::Stop_Wave, 12, 0, 2);
				}
			}

			sendStopFlag = TRUE;

			// 打印日志
			m_page1.PrintLog(_T("\r\n已停止自动测量，请等待剩余数据接收\r\n"));

			// 关闭定时器，这里不能直接KillTimer，因为在发送停止指令后还会有剩余数据。
			// 通过强制让计数器满值，来使其进入收尾状态，接收停止指令后的剩余数据。
			timer = (int)floor(MeasureTime/TIMER_INTERVAL);
		}
		else{
			// 发送停止指令,结束上一次测量结束。
			if (connectStatus) {
				if (m_WaveMode.GetCurSel() == 0){
					NoBackSend(Order::Stop_Integ, 12, 0, 2);
				}
				else if (m_WaveMode.GetCurSel() == 1){
					NoBackSend(Order::Stop_Wave, 12, 0, 2);
				}
			}

			sendStopFlag = TRUE;

			SetDlgItemText(IDC_AutoMeasure, _T("自动测量"));
			//往TCP发送的控制板配置参数允许输入
			SetParameterInputStatus(TRUE);
			//打开其他相关按键使能
			GetDlgItem(IDC_SaveAs)->EnableWindow(TRUE); //设置文件路径
			GetDlgItem(IDC_CONNECT1)->EnableWindow(TRUE); //连接网络
			GetDlgItem(IDC_AutoMeasure)->EnableWindow(TRUE); //自动测量
			GetDlgItem(IDC_Start)->EnableWindow(TRUE); //手动测量
			// 打印日志
			m_page1.PrintLog(_T("\r\n已停止自动测量\r\n"));
		}
	}
}
