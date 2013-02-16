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

void ShutdownNow(void);

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
    MyUPS(const char * upsName, const char * hostName, int portNo = 3493);
    ~MyUPS();

    void PollMaster(void);

    wxString GetUpsname(void) { return upsname; };
    wxString GetHostname(void) { return hostname; };
    int GetPort(void) { return port; };
    UPSCONN_t * GetConnection(void) { return connection; };
    
    enum upsStatus GetStatus(void) { return status; };
    void SetStatus(enum upsStatus Status) { status = Status; };
    unsigned int GetBattLevel(void) { return battLevel; };

private:
    UPSCONN_t * connection;
    wxString upsname;
    wxString hostname;
    int    port;
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
    ~ControlTimer() {};
    void Notify();

private:
    MyApp *m_myApp;
};


class MyTaskBarIcon : public wxTaskBarIcon
{
public:
#if defined(__WXOSX__) && wxOSX_USE_COCOA
    MyTaskBarIcon(wxTaskBarIconType iconType = wxTBI_DEFAULT_TYPE)
    :   wxTaskBarIcon(iconType)
#else
    MyTaskBarIcon()
#endif
    {};

    wxMenu * menu;

    void OnLeftButtonDClick(wxTaskBarIconEvent&);
    void OnMenuConnect(wxCommandEvent&);
    void OnMenuExit(wxCommandEvent&);
    void OnMenuSettings(wxCommandEvent&);
    void OnMenuHelp(wxCommandEvent&);
    void OnMenuAbout(wxCommandEvent&);
    virtual wxMenu *CreatePopupMenu();

    void IconUpdate(enum upsStatus, unsigned int battlvl, 
                                    wxString upsName, wxString hostName);
    void DoShutdown() { ShutdownNow(); };
    DECLARE_EVENT_TABLE()
};


// Define a new application
class MyApp : public wxApp
{
public:
    virtual bool OnInit();
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

    DECLARE_EVENT_TABLE()
};
