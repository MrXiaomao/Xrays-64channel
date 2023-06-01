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
	void	RefreshWindow(BOOL mButtoned);
	BOOL    m_ButtonSignal;
private:
	CBrush m_normalBrush;
	CPen   m_pen;
	CPen   m_CheckedPen;
	CBrush m_activeBrush;
};
