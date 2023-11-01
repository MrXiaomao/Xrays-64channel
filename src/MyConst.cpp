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

// 判断文件夹路径是否存在
BOOL IsPathExit(CString strPath) {
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
BOOL IsFileExit(CString wholePath) {
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

// CString类型的unicode字符串转为string类型的utf-8字符串
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

// CString类型的unicode字符串转为char
char* CstringToWideCharArry(CString CstrText)
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
char HexChar(char c)
{
	if ((c >= '0') && (c <= '9'))
		return c - '0';//将0-9的数字字符转化为十六进制格式
	else if ((c >= 'A') && (c <= 'F'))
		return c - 'A' + 10;//将A-F的字符转化为十六进制格式例如OC C'-'A'+10=12=0x0C
	else if ((c >= 'a') && (c <= 'f'))
		return c - 'a' + 10;//将a-f的字符转化为十六进制格式
	else
		return 0x10;
}

BOOL GetOnePackage(CByteArray &TotalArray, CByteArray &OnePackArray, BYTE* head, BYTE* tail, int checkLen, int PackLength) 
{
	if (TotalArray.GetSize() >= PackLength)
	{
		// 一个包的字节数
		//int PackLength = 11;
		//包头包尾判断
		// BYTE head[2] = { 0xAA, 0xBB };
		// BYTE tail[2] = { 0xCC, 0xDD };
		//----------------------------------寻找包头包尾---------------------------------//
		int HeadIndex = -1; // 赋初值在0-258之外
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

		//-----------------------数据包异常处理------------------------//
		// 如果没有检测到包头或者包尾则返回。不执行后面语句
		if ((HeadIndex == -1) || (TailIndex == -1))  return FALSE;

		if (HeadIndex > TailIndex) // 如果包头大于包尾则清除包头之前的数据
		{
			TotalArray.RemoveAt(0, HeadIndex);
			return FALSE;
		}

		//提取一个数据包
		// CByteArray OnePackArray;
		if ((TailIndex - HeadIndex) == (PackLength - checkLen)) {
			TotalArray.RemoveAt(0, HeadIndex);//移除包头以前的部分
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

//Str2Hex函数的功能则是将如“66 03 ...”形式的字符串以空格为间隔转换为对应的16进制数
//并存放在BYTE型(typdef unsigned char BYTE)数组中，
//data数组作为发送缓冲数组写入串口即可。
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
		if ((t == 16) || (t1 == 16))//判断是否为非法的十六进制
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

//BYTE*转16进制字符串
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

// 读取配置文件
Json::Value ReadSetting(CString fileName)
{
	char* file = CstringToWideCharArry(fileName);
	Json::Value root;
	std::ifstream ifs;
	ifs.open(file, ifstream::out | ifstream::app);
	if (!ifs.is_open()) {
		CString msg;
		msg.Format(_T("%s文件打开失败,请检查当前路径下存在该文件"), fileName);
		MessageBox(NULL, msg, _T("信息提示："), MB_OKCANCEL | MB_ICONERROR);
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

// 读取能量刻度数据
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

//创建文件夹
BOOL Mkdir(CString myPath) {
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
BOOL BrowserMyPath(HWND hwnd, CString defaultPath, CString& outPath)
{
	CString m_saveFilePath;
	TCHAR   szPath[MAX_PATH] = { 0 };

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

// 文件选择对话框，选择指定txt文件
BOOL ChooseFile(CString& outPath) {
	// 打开对话框，选择txt文件
	CString fileName = _T("");

	CFileDialog dlgFile(TRUE, NULL, NULL, OFN_HIDEREADONLY,
		_T("文件 (*.txt)|*.txt||"), NULL);

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