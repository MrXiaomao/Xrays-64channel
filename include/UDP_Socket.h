#pragma once
#include <afxsock.h>

class CXrays_64ChannelDlg;

// 用于界面上记录UDP接受日志的Tab子对话框
class CUDP_Socket : public CSocket
{
 public:
     CUDP_Socket(CXrays_64ChannelDlg* dlg);
   virtual ~CUDP_Socket();
   SOCKADDR_IN ClientAddr;
private:
    CXrays_64ChannelDlg* m_pMainDlg;

public:
    virtual void OnReceive(int nErrorCode);
    //virtual void OnSend(int nErrorCode);
protected:
};