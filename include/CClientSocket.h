#pragma once
#include <afxsock.h>

class CXrays_64ChannelDlg;

class CClientSocket : public CSocket
{
 public:
     CClientSocket(CXrays_64ChannelDlg* dlg);
   virtual ~CClientSocket();
   SOCKADDR_IN ClientAddr;
private:
    CXrays_64ChannelDlg* m_pMainDlg;

public:
    virtual void OnReceive(int nErrorCode);
    //virtual void OnSend(int nErrorCode);
protected:
};