#include "pch.h"
#include "Layout.h"

CLayout::CLayout(void)
{
	m_pParent = NULL;
}


CLayout::~CLayout(void)
{
	UnregisterAllControls();
}

/*----------------------------------------------------------------------------------
 * 功能: 初始化父窗口相关信息，包括初始客户区宽度和高度以及客户区与窗口区高度offset
 * 参数: [in] pParent:父窗口指针
 * 返回值:无 
 -----------------------------------------------------------------------------------*/
void CLayout::Initial(CWnd* pParent)
{
	this->m_pParent = pParent;
	CRect rect;
	pParent->GetClientRect(&rect);
	m_nClientInitHeight = rect.Height();
	m_nClientInitWidth = rect.Width();
}

/*----------------------------------------------------------------------------------
 * 功能: 注册控件至父窗口
 * 参数: [in] nCtrlID:控件ID
         [in] stretchFlag: 缩放标记，参见enum
  * 返回值:无 
 -----------------------------------------------------------------------------------*/
void CLayout::RegisterControl( UINT nCtrlID, int stretchFlag)
{
	ASSERT(m_pParent != NULL);

	CWnd* pCtrl = m_pParent->GetDlgItem(nCtrlID);

	ASSERT(pCtrl != NULL);

	CRect ctrlRect;
	pCtrl->GetWindowRect(&ctrlRect);
	// added by Jackie@2013-03-14
	m_pParent->ScreenToClient(&ctrlRect);
	LOGFONT* pLogFont = NULL;
	CFont* pFont = pCtrl->GetFont();
	if (pFont != NULL)
	{
		pLogFont = new LOGFONT;
		pFont->GetLogFont(pLogFont);
	}
	UINTTOCTRLMap::iterator it = m_ctrlsMap.find(nCtrlID);
	if (it != m_ctrlsMap.end())
	{
		delete (*it).second->m_pCtrlLogFont;
		delete (*it).second;
	}
	m_ctrlsMap[nCtrlID] = new CCtrlInfo(ctrlRect, pLogFont, stretchFlag);
}

/*----------------------------------------------------------------------------------
 * 功能: 从父窗口解除注册控件，解除后该控件不再按主窗口缩放比例移动位置
 * 参数: [in] nCtrlID:被解除注册控件ID
 * 返回值:无 
 -----------------------------------------------------------------------------------*/
void CLayout::UnRegisterControl(UINT nCtrlID)
{
	if (m_ctrlsMap.find(nCtrlID) == m_ctrlsMap.end())
	{
		return;
	}
	CCtrlInfo* pCtrlInfo = m_ctrlsMap[nCtrlID];
	pCtrlInfo->m_font.DeleteObject();
	delete (pCtrlInfo->m_pCtrlLogFont);
	delete pCtrlInfo;
	m_ctrlsMap.erase(nCtrlID);
}

/*----------------------------------------------------------------------------------
 * 功能: 根据主窗口的缩放，调节窗口内各个控件的位置
 * 参数: [in] cx:缩放后主窗口客户区right坐标
         [in] cy:缩放后主窗口客户区bottom坐标
 * 返回值:无 
 -----------------------------------------------------------------------------------*/
void CLayout::OnSize( int cx, int cy )
{
	if (m_ctrlsMap.empty())
	{
		return;
	}

	m_xscale = cx / (float)m_nClientInitWidth;
	m_yscale = cy / (float)m_nClientInitHeight;

	for (UINTTOCTRLMap::iterator it = m_ctrlsMap.begin(); it != m_ctrlsMap.end(); ++it)
	{
		CRect curRect;
		UINT nCtrlID = it->first;
		CWnd* pCtrl = m_pParent->GetDlgItem(nCtrlID);

		CCtrlInfo* pCtrlInfo = it->second;
		ConvertToCurrentRect(pCtrlInfo->m_ctrlRect, pCtrlInfo->m_ctrlCurRect, pCtrlInfo->m_stretchFlag);
		if (((pCtrlInfo->m_stretchFlag & e_stretch_font) != 0) && pCtrlInfo->m_pCtrlLogFont != NULL)
		{
			ConvertToCurrentFont(*(pCtrlInfo->m_pCtrlLogFont), &pCtrlInfo->m_font);
			pCtrl->SetFont(&pCtrlInfo->m_font);
		}
		pCtrl->MoveWindow(&(pCtrlInfo->m_ctrlCurRect));
	}

	m_pParent->Invalidate();
}

/*----------------------------------------------------------------------------------
 * 功能: 根据主窗口缩放比例，将控件的初始窗口坐标缩放为对应当前主窗口的窗口坐标
 * 参数: [in] initRect:控件对应初始主窗口的窗口坐标
		 [out]curRect: 转换后控件对应当前窗口的窗口坐标
		 [in] stretchFlag:缩放标记，参见enum
 * 返回值:无 
 -----------------------------------------------------------------------------------*/
void CLayout::ConvertToCurrentRect(const CRect& initRect, CRect& curRect, int stretchFlag)
{
	curRect.top = (LONG)(initRect.top * m_yscale + 0.5) ;
	curRect.left = (LONG)(initRect.left * m_xscale + 0.5);

	if ((stretchFlag & e_stretch_nofont) != 0)
	{
		curRect.bottom = (LONG)(initRect.bottom * m_yscale + 0.5);
		curRect.right = (LONG)(initRect.right * m_xscale + 0.5);
	}
	else
	{
		if ((stretchFlag & e_stretch_height) != 0)
		{
			curRect.bottom = (LONG)(initRect.bottom * m_yscale + 0.5);
		}
		else
		{
			curRect.bottom = (LONG)(curRect.top + initRect.Height());
		}

		if ((stretchFlag & e_stretch_width) != 0)
		{
			curRect.right = (LONG)(initRect.right * m_xscale + 0.5);
		}
		else
		{
			curRect.right = (LONG)(curRect.left + initRect.Width());
		}
	}
}

/*----------------------------------------------------------------------------------
 * 功能: 根据主窗口缩放比例，缩放控件的字体大小
 * 参数: [in] initLogFont:控件初始字体
		 [out]pCurFont: 转换后控件字体
 * 返回值:无 
 -----------------------------------------------------------------------------------*/
void CLayout::ConvertToCurrentFont(LOGFONT initLogFont, CFont* pCurFont)
{
	LOGFONT curLogFont = initLogFont;

	curLogFont.lfHeight = (LONG)(initLogFont.lfHeight*m_yscale - 0.5);

	pCurFont->DeleteObject();
	pCurFont->CreateFontIndirect(&curLogFont);
}

/*----------------------------------------------------------------------------------
 * 功能: 清空所有已经注册至主窗口的控件
 * 参数: 无
 * 返回值:无 
 -----------------------------------------------------------------------------------*/
void CLayout::UnregisterAllControls()
{
	for (UINTTOCTRLMap::iterator it = m_ctrlsMap.begin(); it != m_ctrlsMap.end(); ++it)
	{
		CCtrlInfo* pCtrlInfo = it->second;
		if (pCtrlInfo != NULL)
		{
			delete pCtrlInfo->m_pCtrlLogFont;
			pCtrlInfo->m_font.DeleteObject();
			delete pCtrlInfo;
		}
	}

	m_ctrlsMap.clear();
}





