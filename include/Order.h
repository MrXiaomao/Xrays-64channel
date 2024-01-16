#pragma once

#include "afxwin.h"
// #include"Afxcoll.h" //CByteArray
// 控制硬件的指令集
class Order
{
public:
	Order(void);
	~Order(void);
	//CByteArray WaveShape; //CByteArray    8位无符号整数 BYTE类型
	//static char WaveShape[]; // 配置波形基本参数

	static BYTE WaveRefreshTime[]; //能谱刷新时间

	static BYTE TriggerThreshold[]; // 触发阈值

	static BYTE TriggerIntervalTime[]; // 触发时间间隔

	static BYTE WorkMode0[]; // 探测器传输模式指令 512道能谱

	static BYTE WorkMode3[]; // 探测器传输模式指令 16道能谱

	static BYTE StartHardTrigger[]; // 硬件触发——开始工作

	static BYTE HardTriggerBack[]; // 硬件触发信号反馈指令,当给FPGA一个触发信号时

	static BYTE StartSoftTrigger[]; // 软件触发——开始工作

	static BYTE Stop[]; //停止工作

public:
	//------------------------------继电器相关指令--------------
	static BYTE relay_ON[]; //继电器第1路打开指令
	static BYTE relay_OFF[]; //继电器第1路关闭指令
	static BYTE relay_GetStatus[]; //读取继电器状态
	static BYTE relay_StatusON[]; //反馈指令，继电器打开状态
	static BYTE relay_StatusOFF[]; //反馈指令，继电器关闭状态

public:
	//---------------------------ARM---------------------------
	static BYTE ARM_GetStatus[]; // 查询指令
};

