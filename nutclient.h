/////////////////////////////////////////////////////////////////////////////
// Name:        tbtest.h
// Purpose:     wxTaskBarIcon sample
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// RCS-ID:      $Id$
// Copyright:   (c)
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include <upsclient.h>
#define PORT_DEFAULT 3493
void ShutdownNow(int flag);

class MyApp;

enum upsStatus
{
    STATUS_NOCNXT = 0,
    STATUS_ONLINE,
    STATUS_ONBATT,
    STATUS_LOWBAT,
    STATUS_SHUTDN,
};

const char *upsStatusStrs[] =
{
    "No Connection",
    "On Line",
    "On Battery",
    "Low Batter",
    "Shutdown Now"
};


//-----------------------------------------------------------------------------
// MyUPS - UPC class
//-----------------------------------------------------------------------------
class MyUPS
{
public:
    MyUPS();
    ~MyUPS();

    void Connect(wxString upsName, wxString hostName, int portNo);
    void Disconnect(void);
    void PollMaster(void);

    wxString GetUpsname(void) { return upsname; };
    void SetUpsname(wxString upsName) { upsname = upsName; };
    wxString GetHostname(void) { return hostname; };
    void SetHostname(wxString hostName) { hostname = hostName; };
    int GetPort(void) { return portno; };
    void SetPort(int portNo) { portno = portNo; };
    //UPSCONN_t * GetConnection(void) { return connection; };
    bool IsConnected(void) { return connected; };
    
    enum upsStatus GetStatus(void) { return status; };
    void SetStatus(enum upsStatus Status) { status = Status; };
    unsigned int GetBattLevel(void) { return battLevel; };

private:
    bool connected;
    UPSCONN_t * connection;
    wxString upsname;
    wxString hostname;
    int    portno;
    enum upsStatus status;
    unsigned int battLevel;
};

//-----------------------------------------------------------------------------
// Control Timer
//-----------------------------------------------------------------------------
class ControlTimer : public wxTimer
{
public:
    ControlTimer(MyApp * myapp);
    ~ControlTimer() { wxTimer::Stop(); };
    void Notify();

private:
    MyApp *m_myApp;
};


class MyTaskBarIcon : public wxTaskBarIcon
{
public:
#if defined(__WXOSX__) && wxOSX_USE_COCOA
    MyTaskBarIcon(wxTaskBarIconType iconType = wxTBI_DEFAULT_TYPE)
    : wxTaskBarIcon(iconType),
#else
    MyTaskBarIcon() :
#endif
       iconGreenLED(GreenLEDOn_xpm),
       iconYellowLED(YellowLEDOn_xpm),
       iconRedLED(RedLEDOn_xpm),
       iconOffLED(LEDOff_xpm)
    {
    };

    wxMenu * menu;
    wxIcon iconGreenLED;
    wxIcon iconYellowLED;
    wxIcon iconRedLED;
    wxIcon iconOffLED;

    void OnLeftButtonDClick(wxTaskBarIconEvent&);
    void OnMenuConnect(wxCommandEvent&);
    void OnMenuExit(wxCommandEvent&);
    void OnMenuSettings(wxCommandEvent&);
    void OnMenuHelp(wxCommandEvent&);
    void OnMenuAbout(wxCommandEvent&);
    virtual wxMenu *CreatePopupMenu();

    void IconUpdate(enum upsStatus, unsigned int battlvl, 
                                    wxString upsName, wxString hostName);
    void DoShutdown(int flag = 0) { ShutdownNow(flag); };/* 0==shutdown, 1==sleep */
    DECLARE_EVENT_TABLE()
};


// Define a new application
class MyApp : public wxApp
{
public:
    virtual bool OnInit();
    virtual int OnExit();
    ControlTimer * m_timer;
};

class MyDialog: public wxDialog
{
public:
    MyDialog(const wxString& title);
    virtual ~MyDialog();

protected:
    void OnAbout(wxCommandEvent& event);
    void OnOK(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnCloseWindow(wxCloseEvent& event);
    void OnIconize(wxIconizeEvent&);

#if defined(__WXOSX__) && wxOSX_USE_COCOA
    MyTaskBarIcon   *m_dockIcon;
#endif

    wxConfigBase * m_config;

   wxTextCtrl * hostnameText; 
   wxTextCtrl * upsnameText;
   wxSpinCtrl * portSpin;

    DECLARE_EVENT_TABLE()
};
