#pragma once
#ifndef _MY_CONSTANT_
#define _MY_CONSTANT_

#include "afx.h"
#include "afxwin.h"
#include "windows.h"
//#include <iostream>
#include <fstream>
#include "json/json.h"
using namespace std;

// By maoxiaoqing 1207402640@qq.com 
// 该文件存放常用的一些简单函数，供其他模块调用。

// 保存十六进制BYTE数据，以二进制方式存储
// 实时保存数据，直接操作I/O口，区别于缓存式输出文件
BOOL SaveFile_BYTE_IO(CString fileName, BYTE* mk, int length);

// 保存十六进制BYTE数据，以二进制方式存储
// 实时保存数据，直接操作I/O口，区别于缓存式输出文件
BOOL SaveFile_BYTE_Cache(fstream &file, BYTE* mk, int length);

// 判断文件夹路径是否存在
BOOL IsPathExit(CString strPath);

// 判断文件是否存在
BOOL IsFileExit(CString wholePath);

// CString类型的unicode字符串转char
int UnicodeToChar(CString& strIn, char* pOut, int nLen);

// CString类型的unicode字符串转为string类型的utf-8字符串
string _UnicodeToUtf8(CString Unicodestr);

// CString类型的unicode字符串转为char
char* CstringToWideCharArry(CString CstrText);

//HexChar函数的功能是将16进制字符由ASCII码转为相应大小的16进制数
char HexChar(char c);

/*
CByteArray TotalArray:输入的数据包
BYTE* head：包头 		// BYTE head[2] = { 0xAA, 0xBB };
BYTE* tail：包尾        // BYTE tail[2] = { 0xCC, 0xDD };
int CheckLen：包头包尾各自的长度，包头包尾长度必须一样
int PackLength：数据包长度（包含包头包尾）
*/
BOOL GetOnePackage(CByteArray &TotalArray, CByteArray &OnePackArray, BYTE* head, BYTE* tail, int checkLen, int PackLength);

//Str2Hex函数的功能则是将如“66 03 ...”形式的字符串以空格为间隔转换为对应的16进制数
//并存放在BYTE型(typdef unsigned char BYTE)数组中，
//data数组作为发送缓冲数组写入串口即可。
int Str2Hex(CString cRcv, BYTE* data);

//十进制转十六进制,十进制的数转化为四字节长度的十六进制
BOOL DecToHex(int decIn, BYTE* pOut);

//BYTE*转16进制字符串
CString Char2HexCString(BYTE* cData, int len);

//比较两个十六进制BYTE数组
BOOL compareBYTE(BYTE* a, BYTE* b, int len);

// 读取配置文件
Json::Value ReadSetting(CString fileName);

// 写入配置文件，实际上是修改配置文件
// 返回：0，写入成功
int WriteSetting(CString fileName, Json::Value jsonData);

// 读取能量刻度数据
vector<CString> ReadEnCalibration(CString fileWholePath);

// 获取当前程序（.exe文件）所在路径
CString GetExeDir();

//创建文件夹
BOOL Mkdir(CString myPath);

// 对话框选择文件夹，设置文件存储路径
// hwnd为窗口句柄，
// defaultPath为默认文件夹路径，用来作为打开时的初始路径
BOOL BrowserMyPath(HWND hwnd, CString defaultPath, CString& outPath);

// 文件选择对话框，选择指定txt文件
BOOL ChooseFile(CString& outPath);

#endif