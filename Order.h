#pragma once
// #include"Afxcoll.h" //CByteArray
// 控制硬件的指令集
class Order
{
public:
	Order(void);
	~Order(void);
	//CByteArray WaveShape; //CByteArray    8位无符号整数 BYTE类型
	//static char WaveShape[]; // 配置波形基本参数

	static char WaveRefreshTime[]; //能谱刷新时间

	static char TriggerThreshold[]; // 触发阈值

	static char TriggerIntervalTime[]; // 触发时间间隔

	static char WorkMode[]; // 探测器传输模式指令

	static char HardTouchStart[]; // 硬件触发——开始工作

	static char SoftTouchStart[]; // 软件触发——开始工作

	static char Stop[]; //停止工作
};

