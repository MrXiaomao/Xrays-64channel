#include "pch.h"
#include "Xrays_64ChannelDlg.h"
#include "PowerDlg.h"
#include "NetSettingDlg.h"


void CXrays_64ChannelDlg::OnBnClickedPowerButton()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CPowerDlg powerdlg; // ����һ��ģ̬�Ի��� 
	powerdlg.DoModal(); // ��ʾģ̬�Ի��� ���в�����swp_SHOWNOMAL,  SW_SHOW, SW_VISION ����Ч����һ����
}

void CXrays_64ChannelDlg::OnPowerMenu()
{
	// TODO: �ڴ���������������
	CPowerDlg powerdlg; // ����һ��ģ̬�Ի��� 
	powerdlg.DoModal(); // ��ʾģ̬�Ի��� ���в�����swp_SHOWNOMAL,  SW_SHOW, SW_VISION ����Ч����һ����
}

void CXrays_64ChannelDlg::OnNetSettingMenu()
{
	// TODO: �ڴ���������������
	CNetSetting netsetdlg; // ����һ��ģ̬�Ի���
	netsetdlg.DoModal(); // ��ʾģ̬�Ի��� ���в�����swp_SHOWNOMAL,  SW_SHOW, SW_VISION ����Ч����һ����
}