/****************************Copyright Reference*****************************
 * File Name   : Layout.h
 * File Version: 1.0
 * Author      : Jackie
 * Create Date : 2012.9.27
 * Description : interface for the CLayout class.
 ****************************************************************************/

/*******************************Change Log***********************************
 *
 * Version  |   Date     | Revisior | Log
 *   1.0    | 2012.9.27  |  Jackie  | Inital version  
 *   1.1    | 2013.3.14  |  Jackie  | fix a bug
            |            |          | Use client's coordinate instead of window's coordinate
			|			 |	        | to resize child controls
			|			 |		    | see detail in RegisterControl
			|			 |		    |  m_pParent->ScreenToClient(&ctrlRect);
			|            |          | optimize
			|            |          |  change x_scale and y_scale to member variable
			|            |          |  move codes for computing scale to OnSize()
*****************************************************************************/

#pragma once
#ifndef _LAYOUT_
#define _LAYOUT_

#include "map"
#include "vector"

using namespace std;

class CCtrlInfo
{
public:
	CCtrlInfo(CRect& rect, LOGFONT* logFont, int stretchFlag)
	{
		m_ctrlRect = rect;
		m_pCtrlLogFont = logFont;
		m_stretchFlag = stretchFlag;
	}

private:
	// Hide copy constructor and '='
	CCtrlInfo(const CCtrlInfo&){}
	CCtrlInfo& operator=(const CCtrlInfo&){}

public:
	CRect    m_ctrlRect;
	CRect    m_ctrlCurRect;
	LOGFONT* m_pCtrlLogFont;
	int      m_stretchFlag;
	CFont    m_font;
};

typedef map<UINT, CCtrlInfo*> UINTTOCTRLMap;

class CLayout
{
public:
	CLayout();
	virtual ~CLayout(void);
private:
	// Hide copy constructor and '='
	CLayout(CLayout&) {}
	CLayout& operator=(const CCtrlInfo&) {}
public:
	void Initial(CWnd* pParent);
	void RegisterControl(UINT nCtrlID, int stretchFlag);
	void UnRegisterControl(UINT nCtrlID);
	void OnSize(int cx, int cy);
	CCtrlInfo* GetCtrlInfo(int nID)
	{
		UINTTOCTRLMap::iterator it = m_ctrlsMap.find(nID);
		if (it != m_ctrlsMap.end())
		{
			return it->second;
		}

		return NULL;
	}

	enum {
		e_stretch_none   = 0x00,        // �ؼ����洰�����ţ�ֻ�洰�������ƶ�����
		e_stretch_width  = 0x01,		// �ؼ����洰������
		e_stretch_height = 0x02,		// �ؼ����洰������
		e_stretch_nofont = 0x03,		// �ؼ������洰�����ţ����岻����
		e_stretch_font   = 0x04,		// �ؼ������洰������
		e_stretch_all    = 0x07,		// �ؼ��������Ծ��洰������
	};

private:
	void ConvertToCurrentRect(const CRect& initRect, CRect& curRect, int stretchFlag);
	void ConvertToCurrentFont(LOGFONT initLogFont, CFont* pCurFont);
	void UnregisterAllControls();

private:
	UINTTOCTRLMap m_ctrlsMap;

	CWnd* m_pParent;
	int   m_nClientInitWidth;	// �����ڿͻ������ʼ���
	int   m_nClientInitHeight;	// �����ڿͻ������ʼ�߶�

	float m_xscale;				// x�������ű���
	float m_yscale;				// y�������ű���
};

#endif