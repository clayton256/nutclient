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

class MyApp;


//-----------------------------------------------------------------------------
// MyUPS - UPC class
//-----------------------------------------------------------------------------
class MyUPS
{
public:
    MyUPS(const char * upsName, const char * hostName, int portNo = 3493);
    ~MyUPS();

    char * GetUpsname(void)  { return upsname; };
    char * GetHostname(void) { return hostname; };
    int GetPort(void) { return port; };
    UPSCONN_t * GetConnection(void) { return connection; };

private:
    UPSCONN_t * connection;
    char * upsname;
    char * hostname;
    int    port;
};

//-----------------------------------------------------------------------------
// Control Timer
//-----------------------------------------------------------------------------
class ControlTimer : public wxTimer
{
public:
    ControlTimer(MyApp *);
    ~ControlTimer() {};
    void Notify();

private:
    MyApp * m_appHandle;
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

    DECLARE_EVENT_TABLE()
};


// Define a new application
class MyApp : public wxApp
{
public:
    virtual bool OnInit();
    ControlTimer * m_timer;
    MyUPS * m_ups;
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

    MyTaskBarIcon   *m_taskBarIcon;
#if defined(__WXOSX__) && wxOSX_USE_COCOA
    MyTaskBarIcon   *m_dockIcon;
#endif

    DECLARE_EVENT_TABLE()
};
