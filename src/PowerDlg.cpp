// PowerDlg.cpp: 实现文件
//

#include "pch.h"
#include "Xrays_64Channel.h"
#include "PowerDlg.h"
#include "afxdialogex.h"

#include "json/json.h"
#include "PowerDlg.h"
#include "Order.h"
#include "MyConst.h"
//#include "Xrays_64ChannelDlg.h"


// CPowerDlg 对话框

IMPLEMENT_DYNAMIC(CPowerDlg, CDialogEx)

CPowerDlg::CPowerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_POWER_DIALOG, pParent)
{
	netStatus = FALSE;
	m_RelayPort = 5000;
}

CPowerDlg::~CPowerDlg()
{
	if (netStatus) {
		netStatus = FALSE; // 用来控制关闭线程
		closesocket(relaySocket); // 关闭套接字
	}
}

void CPowerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX); 
	DDX_Control(pDX, IDC_LED_RELAY, m_NetStatusLED); // “建立链接”LED
	DDX_Control(pDX, IDC_RRLAY_STATUS, m_RelayStatusLED);
	DDX_Text(pDX, IDC_RELAY_PORT, m_RelayPort);
}


BEGIN_MESSAGE_MAP(CPowerDlg, CDialogEx)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_RELAY_CONNECT, &CPowerDlg::OnBnClickedConnect)
	ON_BN_CLICKED(IDC_CHANGE_STATUS, &CPowerDlg::OnBnClickedChange)
END_MESSAGE_MAP()

BOOL CPowerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_NetStatusLED.RefreshWindow(FALSE, _T("OFF"));//设置指示灯
	m_RelayStatusLED.RefreshWindow(2, _T("Unknow"));

	CString StrSerIp = _T("192.168.10.22");
	Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
	if (!jsonSetting.isNull()) {
		if (jsonSetting.isMember("IP_Relay"))
		{
			StrSerIp = jsonSetting["IP_Relay"].asCString();
		}
		if (jsonSetting.isMember("Port_Relay"))
		{
			m_RelayPort = jsonSetting["Port_Relay"].asInt();
		}
	}
	SetDlgItemText(IDC_IPADDRESS_RELAY, StrSerIp);
	SetDlgItemInt(IDC_RELAY_PORT, m_RelayPort);

	GetDlgItem(IDC_CHANGE_STATUS)->EnableWindow(FALSE); //禁用
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

// CPowerDlg 消息处理程序
BOOL CPowerDlg::ConnectRelayTCP() {
	// 1、创建套接字
	relaySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (relaySocket == INVALID_SOCKET) {
		CString info = _T("设备网络初始化创建失败！");
		return FALSE;
	}

	// 2、连接服务器
	CString StrSerIp;
	GetDlgItemText(IDC_IPADDRESS_RELAY, StrSerIp);

	char* pStrIP = CstringToWideCharArry(StrSerIp);

	// 写入配置文件
	Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
	jsonSetting["IP_Relay"] = pStrIP;
	jsonSetting["Port_Relay"] = m_RelayPort;
	WriteSetting(_T("Setting.json"), jsonSetting);

	// 3、设置网络参数
	sockaddr_in server_addr;
	inet_pton(AF_INET, pStrIP, (void*)&server_addr.sin_addr.S_un.S_addr);
	server_addr.sin_family = AF_INET;  // 使用IPv4地址
	server_addr.sin_port = htons(m_RelayPort); //网关：5000

	// 4、检测网络是否连接,以及显示设备联网状况
	if (connect(relaySocket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
		return FALSE;
	}
	return TRUE;
}

void CPowerDlg::OnBnClickedConnect()
{
	GetDlgItem(IDC_RELAY_CONNECT)->EnableWindow(FALSE); //禁用,防止用户重复点击
	UpdateData(TRUE); //是控件变量的值与界面编辑值同步

	CString strTemp;
	GetDlgItemText(IDC_RELAY_CONNECT, strTemp);
	if (strTemp == _T("连接")) {
		netStatus = ConnectRelayTCP();
		if (netStatus) {
			// 开启线程接收数据
			AfxBeginThread(&RecvRealy_Thread, this);
			SetDlgItemText(IDC_RELAY_CONNECT, _T("断开"));
			m_NetStatusLED.RefreshWindow(TRUE, _T("已连接"));//打开指示灯
			//开启定时器，第1个参数表示ID号，第二个参数表示刷新时间ms
			SetTimer(1, 1000, NULL);
			GetDlgItem(IDC_CHANGE_STATUS)->EnableWindow(TRUE); //禁用
		}
		else {
			m_NetStatusLED.RefreshWindow(FALSE, _T("连接失败"));//指示灯
		}
	}
	else {
		netStatus = FALSE; // 用来控制关闭线程
		closesocket(relaySocket); // 关闭套接字
		SetDlgItemText(IDC_RELAY_CONNECT, _T("连接"));
		m_NetStatusLED.RefreshWindow(0, _T("已断开"));//指示灯
		m_RelayStatusLED.RefreshWindow(2, _T("Unknow"));
		GetDlgItem(IDC_CHANGE_STATUS)->EnableWindow(FALSE); //禁用
		KillTimer(1);
	}
	GetDlgItem(IDC_RELAY_CONNECT)->EnableWindow(TRUE); //恢复使用
}

void CPowerDlg::OnBnClickedChange() {
	CString strTemp;
	GetDlgItemText(IDC_CHANGE_STATUS, strTemp);
	if (strTemp == _T("打开")) {
		send(relaySocket, (char*)Order::relay_ON, 10, 0);
		SetDlgItemText(IDC_CHANGE_STATUS, _T("关闭"));
	}
	else {
		send(relaySocket, (char*)Order::relay_OFF, 10, 0);
		SetDlgItemText(IDC_CHANGE_STATUS, _T("打开"));
	}
}

//定时器
void CPowerDlg::OnTimer(UINT_PTR nIDEvent) {
	switch (nIDEvent)
	{
	//定时器一直查询继电器工作状态
	case 1:
		send(relaySocket, (char*)Order::relay_GetStatus, 10, 0);
		break;
	default:
		break;
	}
	CDialogEx::OnTimer(nIDEvent);
}

//接收继电器TCP网口数据
UINT RecvRealy_Thread(LPVOID p)
{
	CPowerDlg* dlg = (CPowerDlg*)p;
	while (1)
	{
		// 断开网络后关闭本线程
		if (!dlg->netStatus) return 0;
		const int dataLen = 10; //接收的数据包长度
		BYTE mk[dataLen];
		int nLength;
		nLength = recv(dlg->relaySocket, (char*)mk, dataLen, 0); //阻塞模式

		if (nLength == -1)
		{
			return 0;
		}
		else {
			if (compareBYTE(mk, Order::relay_StatusON, nLength)) {
				dlg->m_RelayStatusLED.RefreshWindow(1, _T("ON"));
				dlg->GetDlgItem(IDC_CHANGE_STATUS)->SetWindowText(_T("关闭"));
			}
			else if (compareBYTE(mk, Order::relay_StatusOFF, nLength)) {
				dlg->m_RelayStatusLED.RefreshWindow(0, _T("OFF"));
				dlg->GetDlgItem(IDC_CHANGE_STATUS)->SetWindowText(_T("打开"));
			}
		}
	}
}