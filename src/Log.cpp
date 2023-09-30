
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

//����·�������Ʒ�Ϊ����ģʽ
//��������Ĳ��Ǿ���·������EXE���ڵ�Ŀ¼�£��½�һ��LOG�ļ��У�log�ļ�������־�ļ�����ΪSMMS2014-10-28.log
//����������Ǿ���·�������Ծ���·��Ϊ׼��
short CLog::SetLogFile(LPCTSTR strPath)
{
	//�������·��Ϊ�գ�������Ϊxxx.log,Ŀ¼ΪEXEͬĿ¼
	if (strPath==NULL)
	{
		// �õ���ǰ���ļ���
		CString strFileName;
		GetModuleFileName(AfxGetInstanceHandle(),strFileName.GetBuffer(_MAX_PATH),_MAX_PATH);
		strFileName.ReleaseBuffer();

		// �õ���ǰĿ¼
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

//��յ�ǰ���ļ����½�
void CLog::CreateNewFile()
{
	//-�򿪹ر��ļ�-
	if(s_strLogFile.IsEmpty())
		SetLogFile(NULL);
	CStdioFile file;
	if(!file.Open(s_strLogFile, CFile::modeCreate))
	{
		CString str;
		str.Format(_T("���ļ�ʧ�ܣ�·��Ϊ��%s"),s_strLogFile);
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
	//sprintf_s(file,__FILE__); //�ļ���  
	//sprintf_s(func,__FUNCTION__);//������    
	//line = __LINE__;				//�к�

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
	//�߳���
	if(s_hWriteEvent==NULL)
	{
		s_hWriteEvent = OpenEvent(0, FALSE,LOG_EVENT);
		if(s_hWriteEvent==NULL)
			s_hWriteEvent = CreateEvent(NULL, FALSE, TRUE, LOG_EVENT); 
	}
	WaitForSingleObject(s_hWriteEvent, INFINITE);

	//-�򿪹ر��ļ�- 
	if (s_strLogFile.IsEmpty()) { SetLogFile(NULL); }

	CString strPart_Time;
	{
		CTime ct = CTime::GetCurrentTime();
		strPart_Time = ct.Format(_T("\n[%Y-%m-%d %H:%M:%S]# "));
	}
	CString  str;
	string outStr;
	fstream datafile(s_strLogFile, ios::out | ios::app | ios::binary);   // ׷��
	if (datafile.is_open()) {
		str = strPart_Time + s_errorInfo;
		s_errorInfo = "";
		datafile << _UnicodeToUtf8(str);
		str = _T(" [") + s_strLogPrefix + _T("] ") + strFormat;
		datafile << _UnicodeToUtf8(str);
		datafile.close();
	}
	SetEvent(s_hWriteEvent);
	return 1;
}


