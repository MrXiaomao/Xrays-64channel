#pragma once
#include "Layout.h"

void InitLayout(CLayout& layout, CWnd* parent)
{
	layout.Initial(parent);
	layout.RegisterControl(IDC_TAB1, CLayout::e_stretch_nofont); //Ëõ·Å
	layout.RegisterControl(IDC_CLEAR_LOG, CLayout::e_stretch_none); //²»Ëõ·Å
	//layout.RegisterControl(IDC_BUTTONCTRL, CLayout::e_stretch_all);
}
