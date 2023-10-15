// C:\Users\DELL\Desktop\工作事项\16-伽马中子探测\MFC界面\Xrays_64channel-old\Xrays-64channel\src\NetSettingDlg.cpp: 实现文件
//

#include "pch.h"
#include "resource.h"
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
	for (int num = 0; num < 4; num++) {
		PortCHList[num] = 1000 + num;
	}
}

CNetSetting::~CNetSetting()
{
}

void CNetSetting::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_UDP_PORT, m_PortUDP);
	DDX_Text(pDX, IDC_CH1_PORT, PortCHList[0]);
	DDX_Text(pDX, IDC_CH2_PORT, PortCHList[1]);
	DDX_Text(pDX, IDC_CH3_PORT, PortCHList[2]);
	DDX_Text(pDX, IDC_CH4_PORT, PortCHList[3]);
	DDX_Text(pDX, IDC_ARM_PORT, m_PortARM);
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
	CString StrIP_CH1 = _T("192.168.10.22");
	CString StrIP_CH2 = _T("192.168.10.22");
	CString StrIP_CH3 = _T("192.168.10.22");
	CString StrIP_CH4 = _T("192.168.10.22");
	CString StrIP_ARM = _T("192.168.10.22");
	Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
	if (!jsonSetting.isNull()) {
		StrIP_CH1 = jsonSetting["IP_Detector1"].asCString();
		StrIP_CH2 = jsonSetting["IP_Detector2"].asCString();
		StrIP_CH3 = jsonSetting["IP_Detector3"].asCString();
		StrIP_CH4 = jsonSetting["IP_Detector4"].asCString();
		StrIP_ARM = jsonSetting["IP_ARM"].asCString();

		PortCHList[0] = jsonSetting["Port_Detector1"].asInt();
		PortCHList[1] = jsonSetting["Port_Detector2"].asInt();
		PortCHList[2] = jsonSetting["Port_Detector3"].asInt();
		PortCHList[3] = jsonSetting["Port_Detector4"].asInt();
		m_PortUDP = jsonSetting["Port_UDP"].asInt();
		m_PortARM = jsonSetting["Port_ARM"].asInt();
	}

	SetDlgItemText(IDC_IPADDRESS_CH1, StrIP_CH1);
	SetDlgItemText(IDC_IPADDRESS_CH2, StrIP_CH2);
	SetDlgItemText(IDC_IPADDRESS_CH3, StrIP_CH3);
	SetDlgItemText(IDC_IPADDRESS_CH4, StrIP_CH4);
	SetDlgItemText(IDC_IPADDRESS_ARM, StrIP_ARM);

	SetDlgItemInt(IDC_CH1_PORT, PortCHList[0]);
	SetDlgItemInt(IDC_CH2_PORT, PortCHList[1]);
	SetDlgItemInt(IDC_CH3_PORT, PortCHList[2]);
	SetDlgItemInt(IDC_CH4_PORT, PortCHList[3]);
	SetDlgItemInt(IDC_ARM_PORT, m_PortARM);
	SetDlgItemInt(IDC_UDP_PORT, m_PortUDP);

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
		CString StrIP_CH1, StrIP_CH2, StrIP_CH3, StrIP_CH4;
		CString StrIP_ARM;
		GetDlgItemText(IDC_IPADDRESS_CH1, StrIP_CH1);
		GetDlgItemText(IDC_IPADDRESS_CH2, StrIP_CH2);
		GetDlgItemText(IDC_IPADDRESS_CH3, StrIP_CH3);
		GetDlgItemText(IDC_IPADDRESS_CH4, StrIP_CH4);
		GetDlgItemText(IDC_IPADDRESS_ARM, StrIP_ARM);
		
		// 写入配置文件
		Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
		char* pStrIP1 = CstringToWideCharArry(StrIP_CH1);
		char* pStrIP2 = CstringToWideCharArry(StrIP_CH2);
		char* pStrIP3 = CstringToWideCharArry(StrIP_CH3);
		char* pStrIP4 = CstringToWideCharArry(StrIP_CH4);
		char* pStrIP_ARM = CstringToWideCharArry(StrIP_ARM);
		jsonSetting["IP_Detector1"] = pStrIP1;
		jsonSetting["IP_Detector2"] = pStrIP2;
		jsonSetting["IP_Detector3"] = pStrIP3;
		jsonSetting["IP_Detector4"] = pStrIP4;
		jsonSetting["IP_ARM"] = pStrIP_ARM;

		jsonSetting["Port_Detector1"] = PortCHList[0];
		jsonSetting["Port_Detector2"] = PortCHList[1];
		jsonSetting["Port_Detector3"] = PortCHList[2];
		jsonSetting["Port_Detector4"] = PortCHList[3];

		jsonSetting["Port_UDP"] = m_PortUDP;
		jsonSetting["Port_ARM"] = m_PortARM;

		if (WriteSetting(_T("Setting.json"), jsonSetting) == 0) {
			GetDlgItem(IDC_NETSETTING_APPLY)->EnableWindow(FALSE);
		}
	}
}

void CNetSetting::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	//检测前后变量是否发生改变
	/*BOOL isChange = FALSE;
	UpdateData(FALSE);
	Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
	if (!jsonSetting.isNull()) {
		CString StrIP_CH1, StrIP_CH2, StrIP_CH3, StrIP_CH4;
		CString StrIP_ARM;
		GetDlgItemText(IDC_IPADDRESS_CH1, StrIP_CH1);
		GetDlgItemText(IDC_IPADDRESS_CH2, StrIP_CH2);
		GetDlgItemText(IDC_IPADDRESS_CH3, StrIP_CH3);
		GetDlgItemText(IDC_IPADDRESS_CH4, StrIP_CH4);
		GetDlgItemText(IDC_IPADDRESS_ARM, StrIP_ARM);

		if (StrIP_CH1 != jsonSetting["IP_Detector1"].asCString()) { 
			isChange = TRUE; 
		}
		else if(StrIP_CH2 != jsonSetting["IP_Detector2"].asCString());
		StrIP_CH3 = jsonSetting["IP_Detector3"].asCString();
		StrIP_CH4 = jsonSetting["IP_Detector4"].asCString();
		StrIP_ARM = jsonSetting["IP_ARM"].asCString();

		PortCHList[0] = jsonSetting["Port_Detector1"].asInt();
		PortCHList[1] = jsonSetting["Port_Detector2"].asInt();
		PortCHList[2] = jsonSetting["Port_Detector3"].asInt();
		PortCHList[3] = jsonSetting["Port_Detector4"].asInt();
		m_PortUDP = jsonSetting["Port_UDP"].asInt();
		m_PortARM = jsonSetting["Port_ARM"].asInt();
	}*/
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
		if (wmID == IDC_IPADDRESS_CH1 || wmID == IDC_IPADDRESS_CH2 || wmID == IDC_IPADDRESS_CH3 || wmID == IDC_IPADDRESS_CH4){
			isDataChange = TRUE;
		}
		if (wmID == IDC_IPADDRESS_ARM) {
			isDataChange = TRUE;
		}
		if (wmID == IDC_CH1_PORT || wmID == IDC_CH2_PORT || wmID == IDC_CH3_PORT || wmID == IDC_CH4_PORT) {
			isDataChange = TRUE;
		}
		if (wmID == IDC_ARM_PORT || wmID == IDC_UDP_PORT) {
			isDataChange = TRUE;
		}
		// 若控件发生了编辑动作，则激活“应用”按钮
		if(isDataChange) GetDlgItem(IDC_NETSETTING_APPLY)->EnableWindow(TRUE);
	}
	return CDialogEx::OnCommand(wParam, lParam);
}