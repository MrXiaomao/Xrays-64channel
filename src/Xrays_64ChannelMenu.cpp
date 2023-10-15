#include "pch.h"
#include "Xrays_64ChannelDlg.h"
#include "PowerDlg.h"
#include "NetSettingDlg.h"


void CXrays_64ChannelDlg::OnBnClickedPowerButton()
{
	// TODO: 在此添加控件通知处理程序代码
	CPowerDlg powerdlg; // 创建一个模态对话框 
	powerdlg.DoModal(); // 显示模态对话框 其中参数用swp_SHOWNOMAL,  SW_SHOW, SW_VISION 好像效果是一样的
}

void CXrays_64ChannelDlg::OnPowerMenu()
{
	// TODO: 在此添加命令处理程序代码
	CPowerDlg powerdlg; // 创建一个模态对话框 
	powerdlg.DoModal(); // 显示模态对话框 其中参数用swp_SHOWNOMAL,  SW_SHOW, SW_VISION 好像效果是一样的
}

void CXrays_64ChannelDlg::OnNetSettingMenu()
{
	// TODO: 在此添加命令处理程序代码
	CNetSetting netsetdlg; // 创建一个模态对话框
	netsetdlg.DoModal(); // 显示模态对话框 其中参数用swp_SHOWNOMAL,  SW_SHOW, SW_VISION 好像效果是一样的
}