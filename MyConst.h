#pragma once

#include "afx.h"
#include "afxwin.h"
#include <iostream>
#include <fstream>
#include "json/json.h"
using namespace std;

// By maoxiaoqing 1207402640@qq.com 
// 该文件存放常用的一些简单函数，供其他模块调用。


// 判断文件夹路径是否存在
inline BOOL IsPathExit(CString strPath) {
	CFileFind fFile;
	BOOL bRet = fFile.FindFile(strPath);
	if (bRet)
	{
		//std::cout << "CFileFind:" << "文件或者文件夹查找成功" << std::endl;
		//std::cout << std::endl;
		while (bRet)
		{
			bRet = fFile.FindNextFile();
			if (fFile.IsDots())
			{
				continue;
			}

			if (fFile.IsDirectory())
			{
				//std::cout << "CFileFind:" << "文件夹查找成功" << std::endl;
				return TRUE;
			}
		}
	}
	return FALSE;
}

// 判断文件是否存在
inline BOOL IsFileExit(CString wholePath) {
	CFileFind fFile;
	BOOL bRet = fFile.FindFile(wholePath);
	if (bRet)
	{
		//std::cout << "CFileFind:" << "文件或者文件夹查找成功" << std::endl;
		while (bRet)
		{
			bRet = fFile.FindNextFile();
			if (fFile.IsDots())
			{
				continue;
			}

			if (!fFile.IsDirectory())
			{
				//std::cout << "CFileFind:" << "文件查找成功" << std::endl;
				return TRUE;
			}
		}
	}
	return FALSE;
}

// CString类型的unicode字符串转char
inline int UnicodeToChar(CString& strIn, char* pOut, int nLen)
{
	if (NULL == pOut) {
		return 0;
	}
	int len = WideCharToMultiByte(CP_ACP, 0, strIn, -1, NULL, 0, NULL, NULL);
	len = min(len, nLen);

	WideCharToMultiByte(CP_ACP, 0, strIn, -1, pOut, len, NULL, NULL);

	if (len < nLen) {
		pOut[len] = 0;
	}
	return len;
}

// CString类型的unicode字符串转为string类型的utf-8字符串
inline string _UnicodeToUtf8(CString Unicodestr)
{
	wchar_t* unicode = Unicodestr.AllocSysString();
	int len;
	len = WideCharToMultiByte(CP_UTF8, 0, unicode, -1, NULL, 0, NULL, NULL);
	char* szUtf8 = (char*)malloc(len + 1);
	memset(szUtf8, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, unicode, -1, szUtf8, len, NULL, NULL);
	string result = szUtf8;
	free(szUtf8);
	return result;
}

// CString类型的unicode字符串转为char
inline char* CstringToWideCharArry(CString CstrText)
{
	int lth = WideCharToMultiByte(CP_ACP, 0, CstrText, CstrText.GetLength(), NULL, 0, NULL, NULL);
	char* pStr = (char*)malloc((lth + 1) * sizeof(char));
	ASSERT(pStr != NULL);
	memset(pStr, 0, (lth + 1) * sizeof(char)); // 初始化为0
	WideCharToMultiByte(CP_ACP, 0, CstrText.GetBuffer(), CstrText.GetLength(), (LPSTR)pStr, lth, NULL, NULL);
	*(pStr + lth + 1) = '\0';
	return pStr;
} 

//HexChar函数的功能是将16进制字符由ASCII码转为相应大小的16进制数
inline char HexChar(char c)
{
	if ((c >= '0') && (c <= '9'))
		return c - '0';//将?0-9的??数?y字??字??符¤?转?a为a十??六?¨?进?制?格?式??
	else if ((c >= 'A') && (c <= 'F'))
		return c - 'A' + 10;//将?A-F的??字??符¤?转?a为a十??六?¨?进?制?格?式??例¤y如¨?字??符¤?'C'-'A'+10=12=0x0C
	else if ((c >= 'a') && (c <= 'f'))
		return c - 'a' + 10;//将?a-f的??字??符¤?转?a为a十??六?¨?进?制?格?式??
	else
		return 0x10;
}

//Str2Hex函数的功能则是将如“66 03 ...”形式的字符串以空格为间隔转换为对应的16进制数
//并存放在BYTE型(typdef unsigned char BYTE)数组中，
//data数组作为发送缓冲数组写入串口即可。
inline int Str2Hex(CString str, char* data)
{
	int t, t1;
	int rlen = 0, len = str.GetLength();
	if (len == 1)
	{
		char h = str[0];
		t = HexChar(h);
		data[0] = t;
		rlen++;
	}
	//data.SetSize(len/2);
	for (int i = 0; i < len;)
	{
		char l, h = str[i];
		if (h == ' ')
		{
			i++;
			continue;
		}
		i++;
		if (i >= len)
			break;
		l = str[i];
		t = HexChar(h);
		t1 = HexChar(l);
		if ((t == 16) || (t1 == 16))//判D断?为a非¤?法¤?§的??16进?制?数?y
			break;
		else
			t = t * 16 + t1;
		i++;
		data[rlen] = t;
		rlen++;
	}
	return rlen;
}


//十进制转十六进制,十进制的数转化为四字节长度的十六进制
inline BOOL DecToHex(int decIn, char* pOut) {
	if (decIn <= 0xFFFFFF)
	{
		pOut[0] = (decIn & 0xFF000000) >> 24;
		pOut[1] = (decIn & 0x00FF0000) >> 16;
		pOut[2] = (decIn & 0x0000FF00) >> 8;
		pOut[3] = (decIn & 0x000000FF);
		pOut[4] = '\0';
	}
	return TRUE;
}

// 读取配置文件
inline Json::Value ReadSetting(CString fileName)
{
	char* file = CstringToWideCharArry(fileName);
	Json::Value root;
	std::ifstream ifs;
	ifs.open(file);

	Json::CharReaderBuilder builder;
	builder["collectComments"] = true;
	JSONCPP_STRING errs;
	if (!parseFromStream(builder, ifs, &root, &errs)) {
		return root;
	}
	return root;
	/*
	Json::Reader reader;
	Json::Value root;

	//从文件中读取，保证当前文件有demo.json文件
	ifstream in("Setting.json", ios::binary);
	if (!in.is_open())
	{
		cout << "Error opening file\n";
		return root;
	}

	if (reader.parse(in, root))
	{
		//读取根节点信息
		string IP = root["IP_Detector"].asString();
		int port = root["Port_Detector"].asInt();


		//读取子节点信息
		string friend_name = root["friends"]["friend_name"].asString();
		int friend_age = root["friends"]["friend_age"].asInt();
		string friend_sex = root["friends"]["friend_sex"].asString();

		//读取数组信息
		for (unsigned int i = 0; i < root["hobby"].size(); i++)
		{
			string ach = root["hobby"][i].asString();
		}
	}
	in.close();
	return root;
	*/
}

// 写入配置文件，实际上是修改配置文件
inline void WriteSetting(CString fileName, Json::Value jsonData)
{
	char* file = CstringToWideCharArry(fileName);
	ofstream os;
	os.open(file, std::ios::out);
	Json::StreamWriterBuilder sw;
	const std::unique_ptr<Json::StreamWriter> writer(sw.newStreamWriter());
	if (os.is_open())
	{
		writer->write(jsonData, &os);
	}
	os.close();
}

// 读取能量刻度数据
inline vector<CString> ReadEnCalibration(CString fileWholePath) {

	vector<CString> v_msg;

	CStdioFile myFile;
	CFileException fileException;
	if (myFile.Open(fileWholePath, CFile::typeText | CFile::modeReadWrite), &fileException)
	{
		myFile.SeekToBegin();
		CString myStr;
		int txtLine = 0;
		while (myFile.ReadString(myStr)) {
			txtLine++;
			if (txtLine % 3 == 2) //三行为一组，第二行为要发送的数据行
			{
				v_msg.push_back(myStr);
			}
		}
	}
	else
	{
		TRACE("Can't open file %s,error=%u\n", fileWholePath, fileException.m_cause);
	}
	myFile.Close();
	return v_msg;
}

// 获取当前程序（.exe文件）所在路径
inline CString GetExeDir() {
	HMODULE module = GetModuleHandle(0);
	TCHAR pFileName[MAX_PATH] = { 0 };
	GetModuleFileName(module, pFileName, MAX_PATH);
	CString csFullName(pFileName);
	int nPos = csFullName.ReverseFind('\\');
	if (nPos < 0)
		return CString("");
	else
		return csFullName.Left(nPos);
}

//创建文件夹
inline BOOL Mkdir(CString myPath) {
	if (!PathIsDirectory(myPath))
	{
		CreateDirectory(myPath, 0);//不存在则创建
		return TRUE;
	}
	return FALSE;
}

// 对话框选择文件夹，设置文件存储路径
// hwnd为窗口句柄，
// defaultPath为默认文件夹路径，用来作为打开时的初始路径
inline BOOL SetSavePath(HWND hwnd, CString defaultPath, CString& outPath)
{
	CString m_saveFilePath;
	TCHAR   szPath[MAX_PATH] = { 0 };

	// 判断默认路径是否存在
	/*if (IsPathExit(defaultPath)) {
		// Unicode环境下宽字符------->窄字符的转换
		int nNum = WideCharToMultiByte(CP_ACP, 0, szPath, -1, 0, 0, NULL, NULL);
		char* pBuffer = new char[nNum + 1];
		WideCharToMultiByte(CP_ACP, 0, szPath, -1, pBuffer, nNum, NULL, NULL);
	}
	*/

	LPITEMIDLIST   pItemIDList;
	BROWSEINFO   info;
	::ZeroMemory(&info, sizeof(info));
	info.hwndOwner = hwnd;
	info.lpszTitle = _T("请选择路径: ");
	info.pszDisplayName = szPath;
	info.ulFlags = BIF_NEWDIALOGSTYLE; // 窗口可以调整大小,并有新建文件夹按钮
	//info.pidlRoot = CSIDL_DESKTOP; //指定文件夹的根目录，此处为桌面

	if (pItemIDList = ::SHBrowseForFolder(&info))
	{
		if (SHGetPathFromIDList(pItemIDList, szPath))
		{
			m_saveFilePath = szPath;
			m_saveFilePath += "\\";
			outPath = m_saveFilePath;
			return TRUE;
		}
	}
	return FALSE;
}

