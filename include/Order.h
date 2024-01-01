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
	// 触发阈值
	static BYTE TriggerThreshold[]; 
	// 探测器数据传输模式指令 积分模式
	static BYTE WorkMode0[]; 
	// 探测器数据传输模式指令 波形模式
	static BYTE WorkMode5[]; 
	// 硬件触发——开始工作 积分模式
	static BYTE StartHardTrigger_Integ[]; 
	// 软件触发——开始工作 积分模式
	static BYTE StartSoftTrigger_Integ[]; 
	//停止工作 积分模式
	static BYTE Stop_Integ[]; 

	// 硬件触发——开始工作 波形模式
	static BYTE StartHardTrigger_Wave[]; 
	// 软件触发——开始工作 波形模式
	static BYTE StartSoftTrigger_Wave[]; 
	//停止工作 波形模式
	static BYTE Stop_Wave[]; 

public:
	//------------------------------继电器相关指令--------------
	static BYTE relay_ON[]; //继电器第1路打开指令
	static BYTE relay_OFF[]; //继电器第1路关闭指令
	static BYTE relay_GetStatus[]; //读取继电器状态
	static BYTE relay_StatusON[]; //反馈指令，继电器打开状态
	static BYTE relay_StatusOFF[]; //反馈指令，继电器关闭状态

public:
	//---------------------------ARM---------------------------
	static BYTE ARM_Temperature1[]; // 温度查询指令1
	static BYTE ARM_Temperature2[]; // 温度查询指令2
	static BYTE ARM_VoltCurrent[]; // 电压电流查询指令
};

