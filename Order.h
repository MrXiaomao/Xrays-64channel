#pragma once
// #include"Afxcoll.h" //CByteArray
// 控制硬件的指令集
class Order
{
public:
	Order(void);
	~Order(void);
	//CByteArray WaveShape; //CByteArray    8位无符号整数 BYTE类型
	static char WaveShape[]; // 配置波形基本参数
	
	static char WaveRefreshTime[]; //能谱刷新时间

	static char Start[]; //开始工作

	static char Stop[]; //停止工作
};

