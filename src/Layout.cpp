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
 * ����: ��ʼ�������������Ϣ��������ʼ�ͻ�����Ⱥ͸߶��Լ��ͻ����봰�����߶�offset
 * ����: [in] pParent:������ָ��
 * ����ֵ:�� 
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
 * ����: ע��ؼ���������
 * ����: [in] nCtrlID:�ؼ�ID
         [in] stretchFlag: ���ű�ǣ��μ�enum
  * ����ֵ:�� 
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
 * ����: �Ӹ����ڽ��ע��ؼ��������ÿؼ����ٰ����������ű����ƶ�λ��
 * ����: [in] nCtrlID:�����ע��ؼ�ID
 * ����ֵ:�� 
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
 * ����: ���������ڵ����ţ����ڴ����ڸ����ؼ���λ��
 * ����: [in] cx:���ź������ڿͻ���right����
         [in] cy:���ź������ڿͻ���bottom����
 * ����ֵ:�� 
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
 * ����: �������������ű��������ؼ��ĳ�ʼ������������Ϊ��Ӧ��ǰ�����ڵĴ�������
 * ����: [in] initRect:�ؼ���Ӧ��ʼ�����ڵĴ�������
		 [out]curRect: ת����ؼ���Ӧ��ǰ���ڵĴ�������
		 [in] stretchFlag:���ű�ǣ��μ�enum
 * ����ֵ:�� 
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
 * ����: �������������ű��������ſؼ��������С
 * ����: [in] initLogFont:�ؼ���ʼ����
		 [out]pCurFont: ת����ؼ�����
 * ����ֵ:�� 
 -----------------------------------------------------------------------------------*/
void CLayout::ConvertToCurrentFont(LOGFONT initLogFont, CFont* pCurFont)
{
	LOGFONT curLogFont = initLogFont;

	curLogFont.lfHeight = (LONG)(initLogFont.lfHeight*m_yscale - 0.5);

	pCurFont->DeleteObject();
	pCurFont->CreateFontIndirect(&curLogFont);
}

/*----------------------------------------------------------------------------------
 * ����: ��������Ѿ�ע���������ڵĿؼ�
 * ����: ��
 * ����ֵ:�� 
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





