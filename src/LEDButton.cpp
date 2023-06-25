#include "pch.h"
#include "LEDButton.h"

IMPLEMENT_DYNAMIC(LEDButton, CButton)

LEDButton::LEDButton()
{
	m_pen.CreatePen(PS_SOLID, 1, RGB(100, 130, 0));
	m_CheckedPen.CreatePen(PS_SOLID, 1, RGB(0, 130, 100));
	m_normalBrush.CreateSolidBrush(RGB(255, 50, 0));
	m_activeBrush.CreateSolidBrush(RGB(50, 255, 0));
	m_ButtonSignal = FALSE;
}

LEDButton::~LEDButton()
{
	m_pen.DeleteObject();
	m_normalBrush.DeleteObject();
	m_activeBrush.DeleteObject();
	m_CheckedPen.DeleteObject();
}

BEGIN_MESSAGE_MAP(LEDButton, CButton)
END_MESSAGE_MAP()

void LEDButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CString  strText{};
	CPen* oldPen;
	CBrush* oldBrush;
	// 设置不同状态字体
	if (m_ButtonSignal)
	{
		strText = _T("ON");
	}
	if (!m_ButtonSignal)
	{
		strText = _T("OFF");
	}

	// 设置不同状态颜色
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	int nSaveDc = pDC->SaveDC();
	if (!m_ButtonSignal)
	{
		oldBrush = pDC->SelectObject(&m_normalBrush);
		oldPen = pDC->SelectObject(&m_pen);
		pDC->SetTextColor(RGB(0, 0, 255));
	}
	else if (m_ButtonSignal)
	{
		oldBrush = pDC->SelectObject(&m_activeBrush);
		oldPen = pDC->SelectObject(&m_CheckedPen);
		pDC->SetTextColor(RGB(255, 255, 255));
	}
	CRect rct = lpDrawItemStruct->rcItem;
	pDC->Ellipse(&rct); // 绘制椭圆
	pDC->SetBkMode(TRANSPARENT);

	CRect textRect;
	textRect.CopyRect(&rct);
	CSize sz = pDC->GetTextExtent(strText);
	textRect.top += (textRect.Height() - sz.cy) / 8;
	pDC->DrawText(strText, textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	pDC->RestoreDC(nSaveDc);
	pDC->SelectObject(&oldBrush);
	pDC->SelectObject(&oldPen);
}

void LEDButton::PreSubclassWindow()
{
	ModifyStyle(0, BS_OWNERDRAW);
	CRgn rgn;
	CRect rct;
	GetClientRect(&rct);
	rgn.CreateEllipticRgnIndirect(&rct);
	::SetWindowRgn(GetSafeHwnd(), (HRGN)rgn, FALSE);
	CButton::PreSubclassWindow();
}

void LEDButton::RefreshWindow(BOOL mButtoned)
{
	m_ButtonSignal = mButtoned;
	RedrawWindow();
}