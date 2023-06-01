
// Xrays_64ChannelDlg.cpp: 实现文件
//
//#pragma comment(lib,"json.lib")
#include "json/json.h"

#include "pch.h"
#include "framework.h"
#include "Xrays_64Channel.h"
#include "Xrays_64ChannelDlg.h"
#include "afxdialogex.h"
#include "Order.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

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
	, connectStatus(FALSE)
	, MeasureStatus(FALSE)
	, AutoMeasureStatus(FALSE)
	, GetDataStatus(FALSE)
	, m_getTargetChange(FALSE)
	, timer(0)
	, m_currentTab(0)
	, saveAsPath("")
	, sPort(6000)
	, m_targetID(_T("00000"))
	, m_UDPPort(12100)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_NUCLEAR); //设置主界面图标
}

CXrays_64ChannelDlg::~CXrays_64ChannelDlg()
{
	//KillTimer(3); //炮号刷新的计时器
	if (!m_UDPSocket) delete m_UDPSocket;
}

// 绑定变量与控件
void CXrays_64ChannelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LED, m_NetStatusLED);		// “建立链接”LED
													//DDX_Control(pDX, IDC_TargetNum, m_TargetID);
	DDX_Control(pDX, IDC_IPADDRESS1, ServerIP);
	DDX_Text(pDX, IDC_PORT1, sPort);
	DDX_Control(pDX, IDC_COMBO1, m_TriggerType);
	DDX_Text(pDX, IDC_TargetNum, m_targetID);
	DDX_Text(pDX, IDC_UDPPORT, m_UDPPort);
	DDX_Control(pDX, IDC_TAB1, m_Tab);
}

BEGIN_MESSAGE_MAP(CXrays_64ChannelDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_CONNECT1, &CXrays_64ChannelDlg::OnConnect)
	ON_EN_KILLFOCUS(IDC_PORT1, &CXrays_64ChannelDlg::OnEnKillfocusPort1)
	ON_BN_CLICKED(IDC_Start, &CXrays_64ChannelDlg::OnBnClickedStart)
	ON_BN_CLICKED(IDC_AutoMeasure, &CXrays_64ChannelDlg::OnBnClickedAutomeasure)
	ON_EN_KILLFOCUS(IDC_UDPPORT, &CXrays_64ChannelDlg::OnEnKillfocusUDPPort)
	ON_BN_CLICKED(IDC_SaveAs, &CXrays_64ChannelDlg::OnBnClickedSaveas)
	ON_BN_CLICKED(IDC_CLEAR_LOG, &CXrays_64ChannelDlg::OnBnClickedClearLog)
	ON_BN_CLICKED(IDC_UDP_BUTTON, &CXrays_64ChannelDlg::OnBnClickedUdpButton)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CXrays_64ChannelDlg::OnTcnSelchangeTab1)
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	UpdateData(TRUE); //表示写数据，将窗口控制变量写入内存（更新数据）
	//UpdateData(FALSE); //表示读数据，即显示窗口读取内存的数据以供实时显示
	m_NetStatusLED.RefreshWindow(FALSE);//设置指示灯
	m_TriggerType.SetCurSel(0); // 设置下拉框默认选项
	
	//----------------------------窗口切换-------------
	m_page1 = new RunningLog;
	m_page2 = new UDP_RecieveLog;
	//为Tab Control增加两个页面   
	m_Tab.InsertItem(0, _T("系统运行日志"));
	m_Tab.InsertItem(1, _T("UDP运行日志"));

	//创建两个对话框   
	m_page1->Create(IDD_RunningLog, &m_Tab);
	m_page2->Create(IDD_UDP_RecieveLog, &m_Tab);
	
	//设定在Tab内显示的范围
	//子控件大小
	CRect rc;
	m_Tab.GetClientRect(rc);
	rc.top += 20;
	m_page1->MoveWindow(&rc);
	m_page2->MoveWindow(&rc);

	//显示初始页面   
	m_page1->ShowWindow(SW_SHOW);
	m_page2->ShowWindow(SW_HIDE);

	saveAsPath = GetExeDir(); 
	CString str = saveAsPath += "\\";
	CString info = _T("实验数据默认保存路径：") + str;
	m_page1->PrintLog(info);

	// ------------------读取配置参数并设置到相应控件上---------------------
	CString StrSerIp = _T("192.168.10.22");
	Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
	if (!jsonSetting.isNull()) {
		StrSerIp = jsonSetting["IP_Detector"].asCString();
		sPort = jsonSetting["Port_Detector"].asInt();
		m_UDPPort = jsonSetting["Port_UDP"].asInt();
	}
	SetDlgItemText(IDC_IPADDRESS1, StrSerIp);
	SetDlgItemInt(IDC_PORT1, sPort);
	SetDlgItemInt(IDC_UDPPORT, m_UDPPort);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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
	// IDC_CONNECT1为连接控件的按钮，设置该按钮不响应操作，防止用户连续点击多次
	GetDlgItem(IDC_CONNECT1)->EnableWindow(FALSE);
	GetDlgItem(IDC_IPADDRESS1)->EnableWindow(FALSE);
	GetDlgItem(IDC_PORT1)->EnableWindow(FALSE);

	CString strTemp;
	GetDlgItemText(IDC_CONNECT1, strTemp);
	if (strTemp == _T("连接网络")){
		// 1、创建套接字
		mySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (mySocket == NULL) {
			MessageBox(_T("ClientSocket创建失败！"), _T("信息提示："), MB_OKCANCEL | MB_ICONERROR);
			GetDlgItem(IDC_CONNECT1)->EnableWindow(TRUE);
			return;
		}
		
		// 2、连接服务器
		CString StrSerIp;
		GetDlgItemText(IDC_IPADDRESS1, StrSerIp);
		char* pStrIP = CstringToWideCharArry(StrSerIp);

		// 写入配置文件
		Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
		jsonSetting["IP_Detector"] = pStrIP;
		jsonSetting["Port_Detector"] = sPort;
		WriteSetting(_T("Setting.json"),jsonSetting);

		sockaddr_in server_addr;
		//server_addr.sin_addr.s_addr = inet_pton(pStrIP);// 网络IP "192.168.0.105"
		inet_pton(AF_INET, pStrIP, (void*)&server_addr.sin_addr.S_un.S_addr);
		server_addr.sin_family = AF_INET;  // 使用IPv4地址
		server_addr.sin_port = htons(sPort); //网关：5000
		
		// 3、检测网络是否连接,以及显示设备联网状况
		if (connect(mySocket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR){
			m_NetStatusLED.RefreshWindow(FALSE);//打开指示灯
			connectStatus = FALSE;
			MessageBox(_T("设备连接失败。请重新尝试，若再次连接失败,请做以下尝试:\n\
             1、检查网络参数设置是否正确；\n2、检查设备电源是否打开；\n\
			 3、检查工控机的网络适配器属性设置是否正确\n")); 
			// 打印日志
			CString info;
			info.Format(_T("，端口号：%d"), sPort);
			info = _T("TCP网络连接失败，请检查网络，当前网址IP：") + StrSerIp + info;
			m_page1->PrintLog(info);
			// 恢复各个按钮使能状态
			GetDlgItem(IDC_CONNECT1)->EnableWindow(TRUE);
			GetDlgItem(IDC_IPADDRESS1)->EnableWindow(TRUE); //恢复IP地址编辑状态
			GetDlgItem(IDC_PORT1)->EnableWindow(TRUE); //恢复端口编辑
			return;
		}
		CString info;
		info.Format(_T("，端口号：%d"), sPort);
		info = _T("TCP网络已连接，网址IP：") + StrSerIp + info;
		m_page1->PrintLog(info);

		connectStatus = TRUE;
		m_NetStatusLED.RefreshWindow(TRUE);//打开指示灯
		// 4、发送数据，设置硬件参数
		//char msg1[6] = { 0x80,0x01,0x00,0x00,0x00,0xF1 };// 开启第一组偏压监测
		
		send(mySocket, Order::WaveRefreshTime, 24, 0);
		//send(mySocket, msg1, 6, 0);
		AfxBeginThread(&Recv_Th1, 0); //开启线程接收数据
		SetDlgItemText(IDC_CONNECT1, _T("断开网络"));
	}
	else{
		// 1、发送数据，在断开网络前进行相应关闭动作
		//unsigned char msg1[6] = { 0x80,0x01,0x00,0x00,0x00,0x00 };// 开启第一组偏压监测
		//send(mySocket, msg1, 6, 0);
		closesocket(mySocket); //关闭套接字。
		connectStatus = FALSE;
		m_NetStatusLED.RefreshWindow(FALSE);//打开指示灯
		SetDlgItemText(IDC_CONNECT1, _T("连接网络"));
		GetDlgItem(IDC_IPADDRESS1)->EnableWindow(TRUE); //恢复IP地址编辑状态
		GetDlgItem(IDC_PORT1)->EnableWindow(TRUE); //恢复端口编辑
		m_page1->PrintLog(_T("TCP网络已断开"));
	}
	GetDlgItem(IDC_CONNECT1)->EnableWindow(TRUE); // 恢复按钮使能
}

//线程1，接收TCP网口数据
UINT Recv_Th1(LPVOID p)
{
	CXrays_64ChannelDlg* dlg = (CXrays_64ChannelDlg*)AfxGetApp()->GetMainWnd();
	while (1)
	{
		// 断开网络后关闭本线程
		if (!dlg->connectStatus) return 0;

		if (dlg->MeasureStatus || dlg->AutoMeasureStatus)
		{
			const int dataLen = 10000; //接收的数据包长度
			char mk[dataLen]; 

			//连接网络后时常判断联网状态
			int nLength;
			//int recvTimeout = 4 * 1000;  //4s
			//测量状态是等待超过recvTimeout就不再等待
			//setsockopt(dlg->mySocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&recvTimeout, sizeof(int));
			nLength = recv(dlg->mySocket, mk, dataLen, 0);
			if (nLength == -1) //超过recvTimeout不再有数据，关闭该线程
			{
				return 0;
			}
			else {
				dlg->GetDataStatus = TRUE;
				dlg->SaveFile(dlg->m_targetID, mk, nLength);
			}
		}
	}
	return 0;
}

//定时器
void CXrays_64ChannelDlg::OnTimer(UINT_PTR nIDEvent) {
	// 计时3000ms。定时结束后向网口发送停止指令
	switch (nIDEvent)
	{
	case 1:
		if (GetDataStatus) {
			timer++;
			if (timer * 100 > 3000) {
				send(mySocket, Order::Stop, 24, 0);
				SetDlgItemText(IDC_Start, _T("开始测量"));
				timer = 0;
				GetDataStatus = FALSE;
				MeasureStatus = FALSE;

				CString info = _T("炮号：") + m_targetID + _T("测量结束!测试数据存储路径：")
					+ saveAsPath;
				m_page1->PrintLog(info);
			}
		}
		break;
	case 2:
		// 硬件触发后3秒定时发送关闭指令
		if (GetDataStatus && MeasureStatus) {
			timer++;
			if (timer * 100 > 3000) {
				if(mySocket != NULL) send(mySocket, Order::Stop, 24, 0);
				timer = 0;
				GetDataStatus = FALSE;
				MeasureStatus = FALSE;

				CString info = _T("炮号：") + m_targetID + _T("测量结束！测试数据存储路径：")
								+ saveAsPath;
				m_page1->PrintLog(info);
			}
		}
		break;
	case 3:
		//定时检测炮号是否刷新,若刷新，则发送开始指令
		if (m_getTargetChange) {
			if(mySocket != NULL) send(mySocket, Order::Start, 24, 0);
			
			MeasureStatus = TRUE;
			m_getTargetChange = FALSE;

			CString info = _T("炮号已刷新：") + m_targetID;
			m_page1->PrintLog(info);

			//刷新炮号
			UpdateData(FALSE);
		}
		break;
	default:
		break;
	}
	CDialogEx::OnTimer(nIDEvent);
}

//开始测量——手动测量
void CXrays_64ChannelDlg::OnBnClickedStart()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_Start)->EnableWindow(FALSE);

	CString strTemp;
	GetDlgItemText(IDC_Start, strTemp);
	if (strTemp == _T("开始测量")) {
		MeasureStatus = TRUE;
		send(mySocket, Order::Start, 24, 0);
		SetDlgItemText(IDC_Start, _T("停止测量"));
		SetTimer(1, 100, NULL); //开启定时器，第1个参数表示ID号，第二个参数表示刷新时间ms
		// 打印日志
		CString info;
		info = _T("开始测量（手动测量），等待触发信号");
		m_page1->PrintLog(info);
	}
	else {
		MeasureStatus = FALSE;
		send(mySocket, Order::Stop, 24, 0);
		SetDlgItemText(IDC_Start, _T("开始测量"));
		KillTimer(1);	//关闭定时器
		// 打印日志
		CString info;
		info = _T("已停止测量");
		m_page1->PrintLog(info);
	}
	
	GetDlgItem(IDC_Start)->EnableWindow(TRUE);
}

//自动测量按钮
void CXrays_64ChannelDlg::OnBnClickedAutomeasure()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!m_UDPSocket || connectStatus) {
		MessageBox(_T("请检查：\n1、是否开启UDP网络;\n2、是否连接TCP网络。"));
		return;
	}
	GetDlgItem(IDC_AutoMeasure)->EnableWindow(FALSE);

	CString strTemp;
	GetDlgItemText(IDC_AutoMeasure, strTemp);
	if (strTemp == _T("自动测量")) {
		AutoMeasureStatus = TRUE;
		SetDlgItemText(IDC_AutoMeasure, _T("停止测量"));

		//开启定时器，第1个参数表示ID号，第二个参数表示刷新时间ms
		SetTimer(2, 100, NULL);  //100

		// 锁死其他相关按键
		GetDlgItem(IDC_Start)->EnableWindow(FALSE); //手动测量
		GetDlgItem(IDC_CONNECT1)->EnableWindow(FALSE); //连接网络
	}
	else {
		AutoMeasureStatus = FALSE;
		if (mySocket != NULL) send(mySocket, Order::Stop, 24, 0);
		SetDlgItemText(IDC_AutoMeasure, _T("自动测量"));

		//关闭定时器
		KillTimer(2);	

		// 打开其他相关按键使能
		GetDlgItem(IDC_Start)->EnableWindow(TRUE); //手动测量
		GetDlgItem(IDC_CONNECT1)->EnableWindow(TRUE); //连接网络
	}

	GetDlgItem(IDC_AutoMeasure)->EnableWindow(TRUE);
}

void CXrays_64ChannelDlg::OnBnClickedClearLog()
{
	// TODO: 在此添加控件通知处理程序代码
	CRect rc;
	m_Tab.GetClientRect(rc);
	rc.top += 20;
	switch (m_currentTab)
	{
	case 0:
		m_page1->m_Information = _T("");
		m_page1->UpdateData(FALSE);
		m_page1->MoveWindow(&rc);
		break;
	case 1:
		m_page2->m_Information = _T("");
		m_page2->UpdateData(FALSE);
		m_page2->MoveWindow(&rc);
		break;
	}
}

void CXrays_64ChannelDlg::OnBnClickedUdpButton()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strTemp;
	GetDlgItemText(IDC_UDP_BUTTON, strTemp);
	if (strTemp == _T("开启UDP网络")) {
		//打开定时器3，用来刷新炮号
		SetTimer(3, 100, NULL);  //100
		OpenUDP();
		SetDlgItemText(IDC_UDP_BUTTON, _T("关闭UDP网络"));
	}
	else {
		CloseUDP();
		SetDlgItemText(IDC_UDP_BUTTON, _T("开启UDP网络"));
	}
}


void CXrays_64ChannelDlg::OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	m_currentTab = m_Tab.GetCurSel();
	switch (m_currentTab)
	{
	case 0:
		m_page1->ShowWindow(SW_SHOW);
		m_page2->ShowWindow(SW_HIDE);
		m_page1->UpdateData(FALSE);
		break;
	case 1:
		m_page1->ShowWindow(SW_HIDE);
		m_page2->ShowWindow(SW_SHOW);
		m_page2->UpdateData(FALSE);
		break;
	}
	*pResult = 0;
}
