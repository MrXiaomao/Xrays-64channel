#pragma once
#include "Layout.h"

inline void InitLayout(CLayout& layout, CWnd* parent)
{
	layout.Initial(parent);
	layout.RegisterControl(IDC_TAB1, CLayout::e_stretch_nofont); // ???????????????
	layout.RegisterControl(IDC_CLEAR_LOG, CLayout::e_stretch_none); // ???????????????????
	//layout.RegisterControl(IDC_BUTTONCTRL, CLayout::e_stretch_all);
}

inline void InitLayoutRunner(CLayout& layout, CWnd* parent)
{
	layout.Initial(parent);
	layout.RegisterControl(IDC_RUNNING_LOG, CLayout::e_stretch_nofont);
}

inline void InitLayoutUDPLog(CLayout& layout, CWnd* parent)
{
	layout.Initial(parent);
	layout.RegisterControl(IDC_UDP_LOG, CLayout::e_stretch_nofont);
}