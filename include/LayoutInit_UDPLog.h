#pragma once

#include "Layout.h"

void InitLayoutUDPLog(CLayout& layout, CWnd* parent)
{
	layout.Initial(parent);
	layout.RegisterControl(IDC_UDP_LOG, CLayout::e_stretch_nofont);
}

