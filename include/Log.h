#pragma once

using namespace std;

class CLog
{
public:
	CLog(void);
	~CLog(void);

	//-日志文件-
	//---如果没有指定，则为EXE所在路径下的LOG\\EXEName.log	
	static short SetLogFile(LPCTSTR strPath=NULL);

	//-前缀-
	//---如果多个进程往同一个文件输出日志，可以为每个进程设置一个前缀---
	//---前缀出现在日期时间之前---
	static short SetPrefix(CString strPrefix);

	//获取调试信息（函数名称和行号）
	static short GetDugInfo(LPCTSTR pDugInfo=NULL);

	//新建文件
	static void CreateNewFile();

	//-日志信息-
	//---写入日志信息---
	static short  WriteMsg(CString strFormat = NULL);

	static short WriteDugMsg(CString strFormat);

protected:
	static CString s_errorInfo;
	static CString s_strLogFile;
	static CString s_strLogPrefix;
	static HANDLE  s_hWriteEvent;
};

