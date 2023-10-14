#pragma once

// 指示灯显示 
// VS 2019 MFC 圆形按钮指示灯继承CButton类自绘例程 https://blog.csdn.net/lzc881012/article/details/122969928
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
	CString m_Text; // 显示的文字内容
	int m_Status; // 状态，0：关闭、1：打开、2：未知
};
