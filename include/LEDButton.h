#pragma once

// ָʾ����ʾ 
// VS 2019 MFC Բ�ΰ�ťָʾ�Ƽ̳�CButton���Ի����� https://blog.csdn.net/lzc881012/article/details/122969928
class LEDButton : public CButton
{
	DECLARE_DYNAMIC(LEDButton)

public:
	LEDButton();
	virtual ~LEDButton();

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
	virtual void PreSubclassWindow();
	void	RefreshWindow(int mButtoned, CString txt);

private:
	CPen   m_ONPen;
	CPen   m_OFFPen;
	CPen   m_UnknowPen;
	CBrush m_ONBrush;
	CBrush m_OFFBrush;
	CBrush m_UnknowBrush;
	CString m_Text; // ��ʾ����������
	int m_Status; // ״̬��0���رա�1���򿪡�2��δ֪
};
