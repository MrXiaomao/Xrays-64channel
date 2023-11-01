#include "MyConst.h"
#include "afx.h"
#include "afxwin.h"
#include "COMDEF.H"
#include <shlobj.h>
#include <afxdlgs.h>

#include "windows.h"
#include <iostream>
#include <fstream>
//#include "json/json.h"
using namespace std;

void SaveFile_BYTE(CString fileName, BYTE* mk, int length)
{
	if (length < 1)
		return;
	CFile mfile;
	mfile.Open(fileName.GetBuffer(100), CFile::modeCreate | CFile::modeNoTruncate |CFile::modeWrite);
	mfile.SeekToEnd();
	mfile.Write(mk, length);
	fileName.ReleaseBuffer();
	mfile.Close();
}

// �ж��ļ���·���Ƿ����
BOOL IsPathExit(CString strPath) {
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
BOOL IsFileExit(CString wholePath) {
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
int UnicodeToChar(CString& strIn, char* pOut, int nLen)
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
string _UnicodeToUtf8(CString Unicodestr)
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
char* CstringToWideCharArry(CString CstrText)
{
	int lth = WideCharToMultiByte(CP_ACP, 0, CstrText, CstrText.GetLength(), NULL, 0, NULL, NULL);
	char* pStr = (char*)malloc((lth + 1) * sizeof(char));
	ASSERT(pStr != NULL);
	memset(pStr, 0, (lth + 1) * sizeof(char)); // ��ʼ��Ϊ0
	WideCharToMultiByte(CP_ACP, 0, CstrText.GetBuffer(), CstrText.GetLength(), (LPSTR)pStr, lth, NULL, NULL);
	*(pStr + lth + 1) = '\0';
	return pStr;
}

//HexChar�����Ĺ����ǽ�16�����ַ���ASCII��תΪ��Ӧ��С��16������
char HexChar(char c)
{
	if ((c >= '0') && (c <= '9'))
		return c - '0';//��0-9�������ַ�ת��Ϊʮ�����Ƹ�ʽ
	else if ((c >= 'A') && (c <= 'F'))
		return c - 'A' + 10;//��A-F���ַ�ת��Ϊʮ�����Ƹ�ʽ����OC C'-'A'+10=12=0x0C
	else if ((c >= 'a') && (c <= 'f'))
		return c - 'a' + 10;//��a-f���ַ�ת��Ϊʮ�����Ƹ�ʽ
	else
		return 0x10;
}

BOOL GetOnePackage(CByteArray &TotalArray, CByteArray &OnePackArray, BYTE* head, BYTE* tail, int checkLen, int PackLength) 
{
	if (TotalArray.GetSize() >= PackLength)
	{
		// һ�������ֽ���
		//int PackLength = 11;
		//��ͷ��β�ж�
		// BYTE head[2] = { 0xAA, 0xBB };
		// BYTE tail[2] = { 0xCC, 0xDD };
		//----------------------------------Ѱ�Ұ�ͷ��β---------------------------------//
		int HeadIndex = -1; // ����ֵ��0-258֮��
		int TailIndex = -1;

		// DataHead
		for (int i = 0; i < TotalArray.GetSize() - 1; i++)
		{
            bool isHead = true;
            for(int j=0; j<checkLen; j++){
                if ((TotalArray[i+j] & 0xFF) != head[j]){
                    isHead = false;
                    break;
                }
            }
            if(isHead) {
                HeadIndex = i;
                break;
            }
		}

		// DataTail
        for (int i = 0; i < TotalArray.GetSize() - 1; i++)
		{
            bool isTail = true;
            for(int j = 0; j<checkLen; j++){
                if ((TotalArray[i+j] & 0xFF) != tail[j]){
                    isTail = false;
                    break;
                }
            }
            if(isTail) {
                TailIndex = i;
                break;
            }
		}

		//-----------------------���ݰ��쳣����------------------------//
		// ���û�м�⵽��ͷ���߰�β�򷵻ء���ִ�к������
		if ((HeadIndex == -1) || (TailIndex == -1))  return FALSE;

		if (HeadIndex > TailIndex) // �����ͷ���ڰ�β�������ͷ֮ǰ������
		{
			TotalArray.RemoveAt(0, HeadIndex);
			return FALSE;
		}

		//��ȡһ�����ݰ�
		// CByteArray OnePackArray;
		if ((TailIndex - HeadIndex) == (PackLength - checkLen)) {
			TotalArray.RemoveAt(0, HeadIndex);//�Ƴ���ͷ��ǰ�Ĳ���
			OnePackArray.Copy(TotalArray);
			TotalArray.RemoveAt(0, PackLength);
			return TRUE;
		}
		else {
			TotalArray.RemoveAt(0, TailIndex + checkLen);
			return FALSE;
		}
    }
    else{
        return FALSE;
    }
}

//Str2Hex�����Ĺ������ǽ��硰66 03 ...����ʽ���ַ����Կո�Ϊ���ת��Ϊ��Ӧ��16������
//�������BYTE��(typdef unsigned char BYTE)�����У�
//data������Ϊ���ͻ�������д�봮�ڼ��ɡ�
int Str2Hex(CString cRcv, BYTE* data)
{
	std::string str2;
	str2 = _bstr_t(cRcv).operator const char* ();
	int t, t1;
	int rlen = 0, len = cRcv.GetLength();
	if (len == 1)
	{
		char h = str2[0];
		t = HexChar(h);
		data[0] = t;
		rlen++;
	}
	//data.SetSize(len/2);
	for (int i = 0; i < len;)
	{
		char l, h = str2[i];
		if (h == ' ')
		{
			i++;
			continue;
		}
		i++;
		if (i >= len)
			break;
		l = str2[i];
		t = HexChar(h);
		t1 = HexChar(l);
		if ((t == 16) || (t1 == 16))//�ж��Ƿ�Ϊ�Ƿ���ʮ������
			break;
		else
			t = t * 16 + t1;
		i++;
		data[rlen] = t;
		rlen++;
	}
	return rlen;
}

//ʮ����תʮ������,ʮ���Ƶ���ת��Ϊ���ֽڳ��ȵ�ʮ������
BOOL DecToHex(int decIn, BYTE* pOut) {
	if (decIn <= 0xFFFFFF)
	{
		pOut[0] = (BYTE)((decIn & 0xFF000000) >> 24);
		pOut[1] = (BYTE)((decIn & 0x00FF0000) >> 16);
		pOut[2] = (BYTE)((decIn & 0x0000FF00) >> 8);
		pOut[3] = (BYTE)(decIn & 0x000000FF);
	}
	return TRUE;
}

//BYTE*ת16�����ַ���
CString Char2HexCString(BYTE* cData, int len)
{
	CString sHex;
	CString sTemp;
	for (int i = 0; i < len; i++)
	{
		sTemp.Format(_T("%02X"), cData[i]);
		sHex += sTemp.Right(2);
		sHex += " ";
	}
	return sHex;
}

BOOL compareBYTE(BYTE* a, BYTE* b, int len)
{
	for(int n=0; n<len; n++)
	{
		if(a[n] != b[n])
		{
			return FALSE;
		}
	}
	return TRUE;
}

// ��ȡ�����ļ�
Json::Value ReadSetting(CString fileName)
{
	char* file = CstringToWideCharArry(fileName);
	Json::Value root;
	std::ifstream ifs;
	ifs.open(file, ifstream::out | ifstream::app);
	if (!ifs.is_open()) {
		CString msg;
		msg.Format(_T("%s�ļ���ʧ��,���鵱ǰ·���´��ڸ��ļ�"), fileName);
		MessageBox(NULL, msg, _T("��Ϣ��ʾ��"), MB_OKCANCEL | MB_ICONERROR);
	}
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
int WriteSetting(CString fileName, Json::Value jsonData)
{
	int writeStatus = 0;
	char* file = CstringToWideCharArry(fileName);
	ofstream os;
	os.open(file, std::ios::out);
	Json::StreamWriterBuilder sw;
	const std::unique_ptr<Json::StreamWriter> writer(sw.newStreamWriter());
	if (os.is_open())
	{
		writeStatus = writer->write(jsonData, &os);
	}
	os.close();
	return writeStatus;
}

// ��ȡ�����̶�����
vector<CString> ReadEnCalibration(CString fileWholePath) {

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
			if (txtLine % 3 == 2) //����Ϊһ�飬�ڶ���ΪҪ���͵�������
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

// ��ȡ��ǰ����.exe�ļ�������·��
CString GetExeDir() {
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

//�����ļ���
BOOL Mkdir(CString myPath) {
	if (!PathIsDirectory(myPath))
	{
		CreateDirectory(myPath, 0);//�������򴴽�
		return TRUE;
	}
	return FALSE;
}

// �Ի���ѡ���ļ��У������ļ��洢·��
// hwndΪ���ھ����
// defaultPathΪĬ���ļ���·����������Ϊ��ʱ�ĳ�ʼ·��
BOOL BrowserMyPath(HWND hwnd, CString defaultPath, CString& outPath)
{
	CString m_saveFilePath;
	TCHAR   szPath[MAX_PATH] = { 0 };

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

// �ļ�ѡ��Ի���ѡ��ָ��txt�ļ�
BOOL ChooseFile(CString& outPath) {
	// �򿪶Ի���ѡ��txt�ļ�
	CString fileName = _T("");

	CFileDialog dlgFile(TRUE, NULL, NULL, OFN_HIDEREADONLY,
		_T("�ļ� (*.txt)|*.txt||"), NULL);

	if (dlgFile.DoModal() == IDOK)
	{
		fileName = dlgFile.GetPathName();
	}
	else {
		return FALSE;
	}
	outPath = fileName;
	return TRUE;
}