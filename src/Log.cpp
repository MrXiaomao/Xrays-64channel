
#include "pch.h"
#include "Log.h"
#include "MyConst.h"

#include <fstream>
using namespace std;

#define LOG_EVENT _T("Global\\ChyLogWrite")  
CString CLog::s_strLogFile = _T("");  
CString CLog::s_strLogPrefix = _T("");
CString CLog::s_errorInfo=_T("");
HANDLE CLog::s_hWriteEvent = NULL;  


CLog::CLog(void)
{

}


CLog::~CLog(void)
{
}

//设置路径和名称分为两种模式
//如果给出的不是绝对路径：在EXE所在的目录下，新建一个LOG文件夹，log文件夹下日志文件名称为SMMS2014-10-28.log
//如果给出的是绝对路径：则以绝对路径为准。
short CLog::SetLogFile(LPCTSTR strPath)
{
	//如果给的路径为空，则设置为xxx.log,目录为EXE同目录
	if (strPath==NULL)
	{
		// 得到当前的文件名
		CString strFileName;
		GetModuleFileName(AfxGetInstanceHandle(),strFileName.GetBuffer(_MAX_PATH),_MAX_PATH);
		strFileName.ReleaseBuffer();

		// 得到当前目录
		strFileName.Replace(_T(".exe"),_T(".log"));
		s_strLogFile=strFileName;
	}
	else
	{
		s_strLogFile.Format(_T("%s.log"),strPath);
	} 
	return 1;
}

short CLog::SetPrefix(CString strPrefix)
{
	s_strLogPrefix = strPrefix;
	return 1;
}

//清空当前的文件，新建
void CLog::CreateNewFile()
{
	//-打开关闭文件-
	if(s_strLogFile.IsEmpty())
		SetLogFile(NULL);
	CStdioFile file;
	if(!file.Open(s_strLogFile, CFile::modeCreate))
	{
		CString str;
		str.Format(_T("打开文件失败！路径为：%s"),s_strLogFile);
		AfxMessageBox(str);
		exit(0);
	}
	file.Close();
}

short CLog::GetDugInfo(LPCTSTR pDugInfo/* =NULL */)
{
	//char file[1000];  
	//char func[1000];  
	//int line;   
	//sprintf_s(file,__FILE__); //文件名  
	//sprintf_s(func,__FUNCTION__);//函数名    
	//line = __LINE__;				//行号

	s_errorInfo.Format(_T("%s"),pDugInfo);
	//CString str1, str2;
	//str1.Format(_T("%s"), __FILE__);
	//str2.Format(_T("%s"), __FUNCTION__);
	//s_errorInfo = str1 + str2;

	return 1;
}

short CLog::WriteDugMsg(CString strFormat)
{
	GetDugInfo();
	WriteMsg(strFormat);
	return 1;
}

short CLog::WriteMsg(CString strFormat)
{
	//线程锁
	if(s_hWriteEvent==NULL)
	{
		s_hWriteEvent = OpenEvent(0, FALSE,LOG_EVENT);
		if(s_hWriteEvent==NULL)
			s_hWriteEvent = CreateEvent(NULL, FALSE, TRUE, LOG_EVENT); 
	}
	WaitForSingleObject(s_hWriteEvent, INFINITE);

	//-打开关闭文件- 
	if (s_strLogFile.IsEmpty()) { SetLogFile(NULL); }

	CString strPart_Time;
	{
		CTime ct = CTime::GetCurrentTime();
		strPart_Time = ct.Format(_T("\n[%Y-%m-%d %H:%M:%S]# "));
	}
	CString  str;
	string outStr;
	fstream datafile(s_strLogFile, ios::out | ios::app | ios::binary);   // 追加
	if (datafile.is_open()) {
		str = strPart_Time + s_errorInfo;
		s_errorInfo = "";
		datafile << _UnicodeToUtf8(str);
		str = _T("[") + s_strLogPrefix + _T("] ") + strFormat;
		datafile << _UnicodeToUtf8(str);
		datafile.close();
	}
	SetEvent(s_hWriteEvent);
	return 1;
}


