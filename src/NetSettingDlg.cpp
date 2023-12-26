// C:\Users\DELL\Desktop\工作事项\16-伽马中子探测\MFC界面\Xrays_64channel-old\Xrays-64channel\src\NetSettingDlg.cpp: 实现文件
//

#include "pch.h"
#include "Xrays_64Channel.h"
#include "json/json.h"
#include "NetSettingDlg.h"
#include "afxdialogex.h"

#include "MyConst.h"

// CNetSetting 对话框

IMPLEMENT_DYNAMIC(CNetSetting, CDialogEx)

CNetSetting::CNetSetting(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_NETSETTING_DLG, pParent)
{
	isDataChange = FALSE;
	m_PortUDP = 5000;
	m_PortARM = 6000;
	m_PortRelay = 1030;
	PortCHList = 1000;
	StrIP_CH = _T("192.168.10.22");

	StrIP_ARM = _T("192.168.10.22");
	StrIP_Relay = _T("192.168.10.22");
}

CNetSetting::~CNetSetting()
{
	// 保存界面部分参数设置
	Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
	char* pStrIP = CstringToWideCharArry(StrIP_CH);
	jsonSetting["IP_Detector"] = pStrIP;
 	pStrIP = CstringToWideCharArry(StrIP_ARM);
 	jsonSetting["IP_ARM"] = pStrIP;
	pStrIP = CstringToWideCharArry(StrIP_Relay);
 	jsonSetting["IP_Relay"] = pStrIP;

	jsonSetting["Port_Detector"] = PortCHList;
	jsonSetting["Port_UDP"] = m_PortUDP;
	jsonSetting["Port_ARM"] = m_PortARM;
	jsonSetting["Port_Relay"] = m_PortRelay;
	WriteSetting(_T("Setting.json"), jsonSetting);
}

void CNetSetting::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_IPADDRESS_CH1, StrIP_CH);
	DDX_Text(pDX, IDC_IPADDRESS_ARM, StrIP_ARM);
	DDX_Text(pDX, IDC_IPADDRESS_RELAY, StrIP_Relay);
	DDX_Text(pDX, IDC_UDP_PORT, m_PortUDP);
	DDX_Text(pDX, IDC_CH1_PORT, PortCHList);
	DDX_Text(pDX, IDC_ARM_PORT, m_PortARM);
	DDX_Text(pDX, IDC_RELAY_PORT, m_PortRelay);
	DDV_MinMaxInt(pDX, m_PortUDP, 0, 65535);
	DDV_MinMaxInt(pDX, PortCHList, 0, 65535);
	DDV_MinMaxInt(pDX, m_PortARM, 0, 65535);
}

BEGIN_MESSAGE_MAP(CNetSetting, CDialogEx)
	ON_BN_CLICKED(IDC_NETSETTING_APPLY, &CNetSetting::SaveNetSetting)
	ON_BN_CLICKED(IDC_NETSETTING_CANCEL, &CNetSetting::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK_NETSETTING, &CNetSetting::OnBnClickedOk)
END_MESSAGE_MAP()

BOOL CNetSetting::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	// ------------------读取配置参数并设置到相应控件上---------------------
	Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
	if (!jsonSetting.isNull()) {
		if(jsonSetting.isMember("IP_Detector")) {
			StrIP_CH = jsonSetting["IP_Detector"].asCString();
		}
		if (jsonSetting.isMember("IP_ARM")) {
			StrIP_ARM = jsonSetting["IP_ARM"].asCString();
		}
		if (jsonSetting.isMember("IP_Relay")) {
			StrIP_Relay = jsonSetting["IP_Relay"].asCString();
		}
		
		if(jsonSetting.isMember("Port_Detector")) {
			PortCHList = jsonSetting["Port_Detector"].asInt();
		}
		if (jsonSetting.isMember("Port_UDP")) {
			m_PortUDP = jsonSetting["Port_UDP"].asInt();
		}
		if (jsonSetting.isMember("Port_ARM")) {
			m_PortARM = jsonSetting["Port_ARM"].asInt();
		}
		if (jsonSetting.isMember("Port_Relay")) {
			m_PortRelay = jsonSetting["Port_Relay"].asInt();
		}
	}
	UpdateData(FALSE);
	GetDlgItem(IDC_NETSETTING_APPLY)->EnableWindow(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

// CNetSetting 消息处理程序
void CNetSetting::SaveNetSetting()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE); //将控件中的value给该控件的变量赋值。（本质）
	Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
	if (!jsonSetting.isNull()) {
		CString StrIP_CH;
		CString StrIP_ARM;
		CString StrIP_Relay;
		GetDlgItemText(IDC_IPADDRESS_CH1, StrIP_CH);
		GetDlgItemText(IDC_IPADDRESS_ARM, StrIP_ARM);
		GetDlgItemText(IDC_IPADDRESS_RELAY, StrIP_Relay);
		
		// 写入配置文件
		Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
		char* pStrIP1 = CstringToWideCharArry(StrIP_CH);
		char* pStrIP_ARM = CstringToWideCharArry(StrIP_ARM);
		char* pStrIP_Relay = CstringToWideCharArry(StrIP_Relay);
		jsonSetting["IP_Detector"] = pStrIP1;
		jsonSetting["IP_ARM"] = pStrIP_ARM;
		jsonSetting["IP_Relay"] = pStrIP_Relay;

		jsonSetting["Port_Detector"] = PortCHList;
		jsonSetting["Port_UDP"] = m_PortUDP;
		jsonSetting["Port_ARM"] = m_PortARM;
		jsonSetting["Port_Relay"] = m_PortRelay;

		if (WriteSetting(_T("Setting.json"), jsonSetting) == 0) {
			GetDlgItem(IDC_NETSETTING_APPLY)->EnableWindow(FALSE);
		}
	}
}

void CNetSetting::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}


void CNetSetting::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	if (isDataChange) SaveNetSetting();
	CDialogEx::OnOK();
}


BOOL CNetSetting::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// TODO: 在此添加专用代码和/或调用基类

	int wmEVENT = HIWORD(wParam);
	if (wmEVENT == EN_CHANGE)
	{
		//HWND hwndCtl = (HWND) lParam;
		int wmID = LOWORD(wParam);
		if (wmID == IDC_IPADDRESS_CH1){
			isDataChange = TRUE;
		}
		if (wmID == IDC_IPADDRESS_ARM) {
			isDataChange = TRUE;
		}
		if (wmID == IDC_CH1_PORT) {
			isDataChange = TRUE;
		}
		if (wmID == IDC_ARM_PORT || wmID == IDC_UDP_PORT) {
			isDataChange = TRUE;
		}
		if (wmID == IDC_IPADDRESS_RELAY || wmID == IDC_RELAY_PORT) {
			isDataChange = TRUE;
		}
		// 若控件发生了编辑动作，则激活“应用”按钮
		if(isDataChange) GetDlgItem(IDC_NETSETTING_APPLY)->EnableWindow(TRUE);
	}
	return CDialogEx::OnCommand(wParam, lParam);
}