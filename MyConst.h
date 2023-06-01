#pragma once

#include "afx.h"
#include "afxwin.h"
#include <iostream>
#include <fstream>
#include "json/json.h"
using namespace std;

// By maoxiaoqing 1207402640@qq.com 
// ���ļ���ų��õ�һЩ�򵥺�����������ģ����á�


// �ж��ļ���·���Ƿ����
inline BOOL IsPathExit(CString strPath) {
	CFileFind fFile;
	BOOL bRet = fFile.FindFile(strPath);
	if (bRet)
	{
		//std::cout << "CFileFind:" << "�ļ������ļ��в��ҳɹ�" << std::endl;
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
				//std::cout << "CFileFind:" << "�ļ��в��ҳɹ�" << std::endl;
				return TRUE;
			}
		}
	}
	return FALSE;
}

// �ж��ļ��Ƿ����
inline BOOL IsFileExit(CString wholePath) {
	CFileFind fFile;
	BOOL bRet = fFile.FindFile(wholePath);
	if (bRet)
	{
		//std::cout << "CFileFind:" << "�ļ������ļ��в��ҳɹ�" << std::endl;
		while (bRet)
		{
			bRet = fFile.FindNextFile();
			if (fFile.IsDots())
			{
				continue;
			}

			if (!fFile.IsDirectory())
			{
				//std::cout << "CFileFind:" << "�ļ����ҳɹ�" << std::endl;
				return TRUE;
			}
		}
	}
	return FALSE;
}

// CString���͵�unicode�ַ���תchar
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

// CString���͵�unicode�ַ���תΪstring���͵�utf-8�ַ���
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

// CString���͵�unicode�ַ���תΪchar
inline char* CstringToWideCharArry(CString CstrText)
{
	int lth = WideCharToMultiByte(CP_ACP, 0, CstrText, CstrText.GetLength(), NULL, 0, NULL, NULL);
	char* pStr = (char*)malloc((lth + 1) * sizeof(char));
	ASSERT(pStr != NULL);
	memset(pStr, 0, (lth + 1) * sizeof(char));
	WideCharToMultiByte(CP_ACP, 0, CstrText.GetBuffer(), CstrText.GetLength(), (LPSTR)pStr, lth, NULL, NULL);
	*(pStr + lth + 1) = '\0';
	return pStr;
} 

// ��ȡ�����ļ�
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

	//���ļ��ж�ȡ����֤��ǰ�ļ���demo.json�ļ�
	ifstream in("Setting.json", ios::binary);
	if (!in.is_open())
	{
		cout << "Error opening file\n";
		return root;
	}

	if (reader.parse(in, root))
	{
		//��ȡ���ڵ���Ϣ
		string IP = root["IP_Detector"].asString();
		int port = root["Port_Detector"].asInt();


		//��ȡ�ӽڵ���Ϣ
		string friend_name = root["friends"]["friend_name"].asString();
		int friend_age = root["friends"]["friend_age"].asInt();
		string friend_sex = root["friends"]["friend_sex"].asString();

		//��ȡ������Ϣ
		for (unsigned int i = 0; i < root["hobby"].size(); i++)
		{
			string ach = root["hobby"][i].asString();
		}
	}
	in.close();
	return root;
	*/
}

// д�������ļ���ʵ�������޸������ļ�
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

// ��ȡ��ǰ����.exe�ļ�������·��
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

// �Ի���ѡ���ļ��У������ļ��洢·��
// hwndΪ���ھ����
// defaultPathΪĬ���ļ���·����������Ϊ��ʱ�ĳ�ʼ·��
inline BOOL SetSavePath(HWND hwnd, CString defaultPath, CString outPath)
{
	CString m_saveFilePath;
	TCHAR   szPath[MAX_PATH] = { 0 };

	// �ж�Ĭ��·���Ƿ����
	/*if (IsPathExit(defaultPath)) {
		// Unicode�����¿��ַ�------->խ�ַ���ת��
		int nNum = WideCharToMultiByte(CP_ACP, 0, szPath, -1, 0, 0, NULL, NULL);
		char* pBuffer = new char[nNum + 1];
		WideCharToMultiByte(CP_ACP, 0, szPath, -1, pBuffer, nNum, NULL, NULL);
	}
	*/

	LPITEMIDLIST   pItemIDList;
	BROWSEINFO   info;
	::ZeroMemory(&info, sizeof(info));
	info.hwndOwner = hwnd;
	info.lpszTitle = _T("��ѡ��·��: ");
	info.pszDisplayName = szPath;
	info.ulFlags = BIF_NEWDIALOGSTYLE; // ���ڿ��Ե�����С,�����½��ļ��а�ť
	//info.pidlRoot = CSIDL_DESKTOP; //ָ���ļ��еĸ�Ŀ¼���˴�Ϊ����

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

