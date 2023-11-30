#include"Order.h"
Order::Order(void) {
	
	// 配置波形基本参数
	/*WaveShape.SetSize(24);
	WaveShape.Add(0x12); WaveShape.Add(0x34); WaveShape.Add(0x00); WaveShape.Add(0xA1); //1234 00A1
	WaveShape.Add(0x00); WaveShape.Add(0x05); WaveShape.Add(0x08); WaveShape.Add(0x02); //0005 0802
	WaveShape.Add(0x00); WaveShape.Add(0x00); WaveShape.Add(0x00); WaveShape.Add(0x00); //0000 0000
	WaveShape.Add(0x00); WaveShape.Add(0x01); WaveShape.Add(0x00); WaveShape.Add(0x04); //0001 0004
	WaveShape.Add(0x00); WaveShape.Add(0x00); WaveShape.Add(0x00); WaveShape.Add(0x00); //0000 0000
	WaveShape.Add(0x00); WaveShape.Add(0x00); WaveShape.Add(0xab); WaveShape.Add(0xcd); //0000 abcd
	*/

}

Order::~Order(void)
{}

//char Order::WaveShape[24] = { 
//	0x12,0x34, 0x00,0xA1, 0x00,0x05, 0x08,0x02, 0x00,0x00, 0x00,0x00,
//	0x00,0x01, 0x00,0x04, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0xab,0xcd };

BYTE Order::WaveRefreshTime[12] = {
	0x12, 0x34, 0x00, 0x0F, 0xFA, 0x11, 0x00, 0x00, 0x01, 0xE8, 0xAB, 0xCD };

BYTE Order::TriggerThresholdSpectrum[12] = {
	0x12, 0x34, 0x00, 0x0F, 0xFA, 0x12, 0x00, 0x00, 0x00, 0x0F, 0xAB, 0xCD };

BYTE Order::TriggerThresholdWave[12] = {
	0x12, 0x34, 0x00, 0x0F, 0xFE, 0x11, 0x00, 0x00, 0x00, 0x00, 0xAB, 0xCD};

BYTE Order::TriggerIntervalTime[12] = {
	0x12, 0x34, 0x00, 0x0F, 0xFA, 0x14, 0x00, 0x00, 0x00, 0x06, 0xAB, 0xCD };

BYTE Order::WorkMode0[12] = {
	0x12, 0x34, 0x00, 0x0F, 0xFA, 0x13, 0x00, 0x00, 0x00, 0x00, 0xAB, 0xCD };

BYTE Order::WorkMode3[12] = {
	0x12, 0x34, 0x00, 0x0F, 0xFA, 0x13, 0x00, 0x00, 0x00, 0x03, 0xAB, 0xCD };

BYTE Order::StartSpectrumHardTrigger[12] = {
	0x12, 0x34, 0x00, 0x0F, 0xFF, 0xA0, 0x00, 0x00, 0x00, 0x02, 0xAB, 0xCD };

BYTE Order::HardTriggerBack[12] = {
	0x12, 0x34, 0x00, 0xAA, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0xAB, 0xCD};

BYTE Order::StartSpectrumSoftTrigger[12] = {
	0x12, 0x34, 0x00, 0x0F, 0xFF, 0xA0, 0x00, 0x00, 0x00, 0x01, 0xAB, 0xCD };

BYTE Order::SpectrumStop[12] = {
	0x12, 0x34, 0x00, 0x0F, 0xFF, 0xA0, 0x00, 0x00, 0x00, 0x00, 0xAB, 0xCD };

BYTE Order::StartWaveSoftTrigger[12] = {
	0x12, 0x34, 0x00, 0x0F, 0xFE, 0x10, 0x00, 0x00, 0x00, 0x02, 0xAB, 0xCD };

BYTE Order::StartWaveHardTrigger[12] = {
	0x12, 0x34, 0x00, 0x0F, 0xFE, 0x10, 0x00, 0x00, 0x00, 0x01, 0xAB, 0xCD };

BYTE Order::WaveStop[12] = {
	0x12, 0x34, 0x00, 0x0F, 0xFE, 0x10, 0x00, 0x00, 0x00, 0x00, 0xAB, 0xCD};

BYTE Order::relay_ON[10] = {
	0x48, 0x3A, 0x01, 0x70, 0x02, 0x01, 0x00, 0x00, 0x45, 0x44 };

BYTE Order::relay_OFF[10] = {
	0x48, 0x3A, 0x01, 0x70, 0x02, 0x00, 0x00, 0x00, 0x45, 0x44 };

BYTE Order::relay_GetStatus[10] = {
	0x48, 0x3A, 0x01, 0x72, 0x02, 0x00, 0x00, 0x00, 0x45, 0x44 };

BYTE Order::relay_StatusON[10] = {
	0x48, 0x3A, 0x01, 0x71, 0x02, 0x01, 0x00, 0x00, 0x45, 0x44 };

BYTE Order::relay_StatusOFF[10] = {
	0x48, 0x3A, 0x01, 0x71, 0x02, 0x00, 0x00, 0x00, 0x45, 0x44 };


BYTE Order::ARM_Temperature1[12] = {
	0x12, 0x34, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAB, 0xCD};

BYTE Order::ARM_Temperature2[12] = {
	0x12, 0x34, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAB, 0xCD };

BYTE Order::ARM_VoltCurrent[12] = {
	0x12, 0x34, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAB, 0xCD };