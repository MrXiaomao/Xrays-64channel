#pragma once

#include "Layout.h"

void InitLayoutRunner(CLayout& layout, CWnd* parent)
{
	layout.Initial(parent);
	layout.RegisterControl(IDC_RUNNING_LOG, CLayout::e_stretch_nofont);
}

