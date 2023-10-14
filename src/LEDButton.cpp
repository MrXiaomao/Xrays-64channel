#include "pch.h"
#include "LEDButton.h"

IMPLEMENT_DYNAMIC(LEDButton, CButton)

LEDButton::LEDButton()
{
	m_OFFPen.CreatePen(PS_SOLID, 1, RGB(100, 130, 0));
	m_ONPen.CreatePen(PS_SOLID, 1, RGB(0, 130, 100));
	m_UnknowPen.CreatePen(PS_SOLID, 1, RGB(255, 255, 255));

	m_OFFBrush.CreateSolidBrush(RGB(255, 0, 0));
	m_ONBrush.CreateSolidBrush(RGB(0, 255, 0));
	m_UnknowBrush.CreateSolidBrush(RGB(128, 128, 128));
	m_Status = 2;
}

LEDButton::~LEDButton()
{
	m_ONPen.DeleteObject();
	m_OFFPen.DeleteObject();
	m_UnknowPen.DeleteObject();
	m_OFFBrush.DeleteObject();
	m_ONBrush.DeleteObject();
	m_UnknowBrush.DeleteObject();
}

BEGIN_MESSAGE_MAP(LEDButton, CButton)
END_MESSAGE_MAP()

void LEDButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CString  strText{};
	CPen* oldPen;
	CBrush* oldBrush;

	strText = m_Text;

	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	int nSaveDc = pDC->SaveDC();
	
	switch (m_Status) {
	case 0:
		oldBrush = pDC->SelectObject(&m_OFFBrush);
		oldPen = pDC->SelectObject(&m_OFFPen);
		pDC->SetTextColor(RGB(0, 0, 255));
		break;
	case 1:
		oldBrush = pDC->SelectObject(&m_ONBrush);
		oldPen = pDC->SelectObject(&m_ONPen);
		pDC->SetTextColor(RGB(255, 255, 255));
		break;
	case 2:
		oldBrush = pDC->SelectObject(&m_UnknowBrush);
		oldPen = pDC->SelectObject(&m_UnknowPen);
		pDC->SetTextColor(RGB(0, 0, 255));
		break;
	default:
		break;
	}

	CRect rct = lpDrawItemStruct->rcItem;
	pDC->Ellipse(&rct); // ????
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

void LEDButton::RefreshWindow(int mButtoned, CString txt)
{
	m_Text = txt;
	m_Status = mButtoned;
	RedrawWindow();
}