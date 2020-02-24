
// DISHARD2Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "DISHARD2.h"
#include "DISHARD2Dlg.h"
#include "afxdialogex.h"
#include <SETUPAPI.H>    
//#include <SHLWAPI.H>    
#include <cfgmgr32.h>
//Setupapi.lib

#pragma comment(lib, "setupapi.lib")    
#pragma comment(lib, "rpcrt4.lib")
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
BOOL GetRegistryProperty( HDEVINFO DeviceInfoSet, 
                        PSP_DEVINFO_DATA DeviceInfoData, 
                        ULONG Property, 
                        PVOID Buffer, 
                        PULONG Length );

BOOL EnableDevice(DWORD dwDevID, HDEVINFO hDevInfo);
BOOL StateChange( DWORD dwNewState, DWORD dwDevID, HDEVINFO hDevInfo);
BOOL DisableDevice(DWORD dwDevID, HDEVINFO hDevInfo);
BOOL ControlDevice(DWORD dwDevID, HDEVINFO hDevInfo , BOOL bEnable);

BOOL IsDisableable(DWORD dwDevID, HDEVINFO hDevInfo);
BOOL IsDisabled(DWORD dwDevID, HDEVINFO hDevInfo);
BOOL SetCOMDevState(BOOL bEnable); 
// CAboutDlg dialog used for App About
int findstr(char s1[256],char s2[256]);
BOOL MySystemShutdown2();
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CDISHARD2Dlg dialog



CDISHARD2Dlg::CDISHARD2Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDISHARD2Dlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDISHARD2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDISHARD2Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CDISHARD2Dlg::OnBnClickedOk)
	ON_WM_ACTIVATE()
	ON_BN_CLICKED(IDCANCEL, &CDISHARD2Dlg::OnBnClickedCancel)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CDISHARD2Dlg message handlers

BOOL CDISHARD2Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDISHARD2Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDISHARD2Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDISHARD2Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


BOOL GetRegistryProperty( HDEVINFO DeviceInfoSet, 
                        PSP_DEVINFO_DATA DeviceInfoData, 
                        ULONG Property, 
                        PVOID Buffer, 
                        PULONG Length ) 
{ 
    while ( !SetupDiGetDeviceRegistryProperty( DeviceInfoSet, 
                                               DeviceInfoData, 
                                               Property, 
                                               NULL, 
                                               (BYTE *)*(TCHAR **)Buffer, 
                                               *Length, 
                                               Length)) 
    { 
        // 长度不够则重新分配缓冲区
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) 
        { 
            if (*(LPTSTR *)Buffer) 
                LocalFree(*(LPTSTR *)Buffer); 

            *(LPTSTR *)Buffer = (PCHAR)LocalAlloc(LPTR,*Length); 
        } 
        else 
        {
            return false; 
        }
    } 

    return (BOOL)(*(LPTSTR *)Buffer)[0]; 
} 

BOOL EnableDevice(DWORD dwDevID, HDEVINFO hDevInfo)
{
    return StateChange(DICS_ENABLE,dwDevID,hDevInfo);
}

BOOL StateChange( DWORD dwNewState, DWORD dwDevID, HDEVINFO hDevInfo)
{
    SP_PROPCHANGE_PARAMS PropChangeParams;
    SP_DEVINFO_DATA        DevInfoData = {sizeof(SP_DEVINFO_DATA)};
    SP_DEVINSTALL_PARAMS devParams;

    //查询设备信息
    if (!SetupDiEnumDeviceInfo( hDevInfo, dwDevID, &DevInfoData))
    {
        OutputDebugString("SetupDiEnumDeviceInfo FAILED");
        return FALSE;
    }

    //设置设备属性变化参数
    PropChangeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    PropChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
    PropChangeParams.Scope = DICS_FLAG_GLOBAL; //使修改的属性保存在所有的硬件属性文件
    PropChangeParams.StateChange = dwNewState;
    PropChangeParams.HwProfile = 0;


    //改变设备属性
    if (!SetupDiSetClassInstallParams( hDevInfo,
                                        &DevInfoData,
                                        (SP_CLASSINSTALL_HEADER *)&PropChangeParams,
                                        sizeof(PropChangeParams)))
    {
        OutputDebugString("SetupDiSetClassInstallParams FAILED");
        return FALSE;
    }
    
    
    PropChangeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    PropChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
    PropChangeParams.Scope = DICS_FLAG_CONFIGSPECIFIC;//使修改的属性保存在指定的属性文件
    PropChangeParams.StateChange = dwNewState;
    PropChangeParams.HwProfile = 0;

    //改变设备属性并调用安装服务
    if (!SetupDiSetClassInstallParams( hDevInfo,
                                       &DevInfoData,
                                       (SP_CLASSINSTALL_HEADER *)&PropChangeParams,
                                       sizeof(PropChangeParams)) ||
        !SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, hDevInfo, &DevInfoData))
    {
        OutputDebugString("SetupDiSetClassInstallParams or SetupDiCallClassInstaller FAILED");
        return TRUE;
    }
    else
    { 
        //判断是否需要重新启动
        devParams.cbSize = sizeof(devParams);
        if (!SetupDiGetDeviceInstallParams( hDevInfo, &DevInfoData, &devParams))
        {
            OutputDebugString("SetupDiGetDeviceInstallParams FAILED");
            return FALSE;
        }

        if (devParams.Flags & (DI_NEEDRESTART|DI_NEEDREBOOT))
        {
            OutputDebugString("Need Restart Computer");
            return TRUE;
        }
        return TRUE;
    }
}


BOOL DisableDevice(DWORD dwDevID, HDEVINFO hDevInfo)
{
    return StateChange(DICS_DISABLE,dwDevID,hDevInfo);
}

BOOL ControlDevice(DWORD dwDevID, HDEVINFO hDevInfo , BOOL bEnable)
{
    BOOL bCanDisable;
    bCanDisable = (IsDisableable(dwDevID,hDevInfo) && (!IsDisabled(dwDevID,hDevInfo)));

if (bEnable)//Enable NetCard
{
if (!bCanDisable)
{
return EnableDevice(dwDevID,hDevInfo);
}
}
else //Disable NetCard
{
if (bCanDisable )
{
return DisableDevice(dwDevID,hDevInfo);
}
}

    return TRUE;
}

BOOL IsDisableable(DWORD dwDevID, HDEVINFO hDevInfo)
{
    SP_DEVINFO_DATA DevInfoData = {sizeof(SP_DEVINFO_DATA)};
    DWORD    dwDevStatus,dwProblem;
    if(!SetupDiEnumDeviceInfo(hDevInfo,dwDevID,&DevInfoData))
    {
        OutputDebugString("SetupDiEnumDeviceInfo FAILED");
        return FALSE;
    }
    //查询设备状态
    if(CM_Get_DevNode_Status(&dwDevStatus,&dwProblem,DevInfoData.DevInst,0)!=CR_SUCCESS)
    {
        OutputDebugString("CM_GET_DevNode_Status FAILED");
        return FALSE;
    }
    return ((dwDevStatus & DN_DISABLEABLE) && (dwProblem != CM_PROB_HARDWARE_DISABLED));
}

BOOL IsDisabled(DWORD dwDevID, HDEVINFO hDevInfo)
{
    SP_DEVINFO_DATA DevInfoData = {sizeof(SP_DEVINFO_DATA)};
    DWORD    dwDevStatus,dwProblem;
    if(!SetupDiEnumDeviceInfo(hDevInfo,dwDevID,&DevInfoData))
    {
        OutputDebugString("SetupDiEnumDeviceInfo FAILED");
        return FALSE;
    }
    //查询设备状态
    if(CM_Get_DevNode_Status(&dwDevStatus,&dwProblem,DevInfoData.DevInst,0)!=CR_SUCCESS)
    {
        OutputDebugString("CM_GET_DevNode_Status FAILED");
        return FALSE;
    }
    return ((dwDevStatus & DN_HAS_PROBLEM) && (dwProblem == CM_PROB_DISABLED));
}


BOOL SetCOMDevState(BOOL bEnable) 
{
    DWORD Status, Problem; 
    LPTSTR Buffer = NULL; 
    DWORD BufSize = 0; 
BOOL bResult = FALSE;

    // 返回所有设备信息
    HDEVINFO hDevInfo = SetupDiGetClassDevs(NULL,NULL,0,DIGCF_PRESENT|DIGCF_ALLCLASSES);

    if (INVALID_HANDLE_VALUE == hDevInfo )        
        return bResult; 

    SP_DEVINFO_DATA DeviceInfoData = {sizeof(SP_DEVINFO_DATA)};

    //////////////////////////////////////////////////////////////////////////
    // 枚举设备
    //////////////////////////////////////////////////////////////////////////
    for ( DWORD DeviceId=0; 
        SetupDiEnumDeviceInfo( hDevInfo,DeviceId,&DeviceInfoData); 
        DeviceId++) 
    {
// 获得设备的状态
        if (CM_Get_DevNode_Status(&Status, &Problem, DeviceInfoData.DevInst ,0) != CR_SUCCESS) 
            continue; 

        // 获取设备类名
        TCHAR szDevName [MAX_PATH] = _T("") ;
        if (GetRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_CLASS , &Buffer, (PULONG)&BufSize)) 
        {
            lstrcpyn( szDevName, Buffer, MAX_PATH );          
        }
//::MessageBox(NULL,szDevName,"ERROR",MB_OK);
      
		/*if ( lstrcmp( szDevName, _T("Ports") ) == 0 )            
        {             
            TCHAR szName [MAX_PATH] = _T("") ;
            if (GetRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_ENUMERATOR_NAME , &Buffer, (PULONG)&BufSize)) 
            {
                lstrcpyn( szName, Buffer, MAX_PATH );
            }
			*/

            if ( lstrcmp( szDevName, _T("USB") ) == 0 )                
            { 
				//{
				TCHAR szName [MAX_PATH] = _T("") ;
				if (GetRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_DEVICEDESC , &Buffer, (PULONG)&BufSize)) 
                {
                    lstrcpyn( szName, Buffer, MAX_PATH );
//::MessageBox(NULL,szName,"ERROR",MB_OK);
// 获取设备描述
if ( lstrcmp( szName, _T("USB Mass Storage Device") ) == 0 ) 
{				
	//::MessageBox(NULL,szName,"ERROR",MB_OK);
	if (GetRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_DEVICEDESC , &Buffer, (PULONG)&BufSize)) 
{
                    
lstrcpyn( szName, Buffer, MAX_PATH );
bResult = ControlDevice(DeviceId,hDevInfo,bEnable);
/*if ( lstrcmp( szName, _T("USB-SERIAL") ) == 0 )//找到平安卡COM口转USB口读写设备           
{
bResult = ControlDevice(DeviceId,hDevInfo,bEnable);
}*/
} 
                }
            } 
        } 
    }
    SetupDiDestroyDeviceInfoList(hDevInfo);

return bResult;
}















void CDISHARD2Dlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	char UserName[MAX_PATH]; 
CString StrUserName; 
DWORD Size=MAX_PATH; 
//SetCOMDevState(FALSE);
//PostMessage(WM_COMMAND, MAKEWPARAM(IDCANCEL, BN_CLICKED), NULL);
//::GetUserName(UserName,&Size); 
//SetCOMDevState(TRUE);
//hThread=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadFunc,NULL,0,&ThreadID);
HWND hWnd;
	hWnd=::FindWindow(NULL,_T("Lock"));
			if(hWnd!= NULL)
{
 /*::MessageBox(0, TEXT("Sucess"),
                    TEXT("Title"), MB_OK|MB_ICONASTERISK );
 */
 ::SendMessage(hWnd,WM_SHOWWINDOW,SW_HIDE,SW_PARENTCLOSING);
 //::SendMessage(hWnd,WM_CLOSE,0,0);
//b=0;
		}
			
/*if(findstr(UserName,_T("jay.lai"))==-1)
{
	 MySystemShutdown2();
}*/
//SetCOMDevState(TRUE);
/*do{
SetCOMDevState(FALSE);
	::GetUserName(UserName,&Size); 
StrUserName.Format( "%s ",UserName); 
//MessageBox( "用户名称为： "+StrUserName, "信息提示 ",MB_OK); 
if(findstr(UserName,_T("jay.lai"))!=-1)
{
	SetCOMDevState(TRUE);
}
if(findstr(UserName,_T("jay.lai"))==-1)
{
AfxMessageBox(StrUserName);	
	SetCOMDevState(FALSE);
}

}while(1);*/
	//CDialogEx::OnOK();
}



int findstr(char s1[256],char s2[256])
 { int len1,len2,a,b,c,d;
    len1=strlen(s1);
    len2=strlen(s2);
    if (len1-len2<0)
    {return -1;}
   else
    {
    for(a=0;a<=(len1-len2);a++)
    {  b=a;
    for(c=0;c<len2;c++)
    {  if (s1[b]!=s2[c])
       { break;
       }
       b=b+1;
       }

      if (c==len2)
      {break;}
      }
       if(a>(len1-len2))
      {return -1;}
       else
        {return a;}
       }

}



BOOL MySystemShutdown2() 
	{
HANDLE hToken; 

TOKEN_PRIVILEGES tkp;

 

 // Get a token for this process.

 if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))

 return( FALSE ); 

 

// Get the LUID for the shutdown privilege.

 LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);

 tkp.PrivilegeCount = 1;

 

 // one privilege to set 

tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 

 

// Get the shutdown privilege for this process.

 AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0); if (GetLastError() != ERROR_SUCCESS) 

   return FALSE; 

 

// Shut down the system and force all applications to close. 

if (!ExitWindowsEx(EWX_SHUTDOWN|EWX_FORCE,0))

 return FALSE; 

return TRUE; 












	}


void CDISHARD2Dlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CDialogEx::OnActivate(nState, pWndOther, bMinimized);

/*do{
SetCOMDevState(FALSE);
	
}while(1);	
	
	*/
	
	hThread=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadFunc,NULL,0,&ThreadID);	
	// TODO: Add your message handler code here
}


void ThreadFunc()
{

do
{
SetCOMDevState(FALSE);
Sleep(5500); 
}while(1);





}


void CDISHARD2Dlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	DWORD code; 
GetExitCodeThread(hThread,&code); 
if(code == STILL_ACTIVE) 
     {   
TerminateThread(hThread,0); 
                       } 	
	
//SetCOMDevState(TRUE);	
	CDialogEx::OnCancel();
}


void CDISHARD2Dlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);
PostMessage(WM_COMMAND, MAKEWPARAM(IDOK, BN_CLICKED), NULL);  

	
	
	// TODO: Add your message handler code here
}
