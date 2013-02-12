/////////////////////////////////////////////////////////////////////////////
// Name:        tbtest.cpp
// Purpose:     wxTaskBarIcon demo
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// RCS-ID:      $Id$
// Copyright:   (c)
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

// the application icon (under Windows and OS/2 it is in resources)
#ifndef wxHAS_IMAGES_IN_RESOURCES
    #include "images/LEDOff.xpm"
    #include "images/RedLEDOn.xpm"
    #include "images/GreenLEDOn.xpm"
    #include "nutclient.xpm"
#endif


#include <wx/taskbar.h>
#include <wx/spinctrl.h>
#include <wx/notifmsg.h>
#include "nutclient.h"
//#ifdef __cplusplus
//extern "C" {
//#endif
#include <upsclient.h>
//#ifdef __cplusplus
//} /*extern "C"*/
//#endif


// ----------------------------------------------------------------------------
// global variables
// ----------------------------------------------------------------------------

static MyDialog *gs_dialog = NULL;

// ============================================================================
// implementation
// ============================================================================



ControlTimer::ControlTimer(MyApp * myApp) : wxTimer()
{
    m_appHandle = myApp;
    wxTimer::Start(5000);
} // ControlTimer c-tor


void ControlTimer::Notify()
{
    int ret;
    unsigned int numq, numa;
    const char *query[4];
    char ** answer;

    wxLogVerbose("ControlTimer:Notify");
    if(NULL == m_appHandle) return;
    if(NULL == m_appHandle->m_ups) return;

    char * upsName = m_appHandle->m_ups->GetUpsname();
    UPSCONN_t * conn = m_appHandle->m_ups->GetConnection();

    query[0] = "VAR";
    query[1] = upsName;
    query[2] = "ups.status";
    numq = 3;

    ret = upscli_get(conn, numq, query, &numa, &answer);

    if (ret < 0) 
    {
        /* detect old upsd */
        if (upscli_upserror(conn) == UPSCLI_ERR_UNKCOMMAND) 
        {
            wxLogError(wxString::Format(wxT("UPS [%s]: Too old to monitor"), upsName));
            return;
        }

        wxLogError(wxT("UPS [%s]: Unknown error"), upsName);
        /* some other error */
        return;
    }

    if (numa < numq) {
        wxLogError(wxString::Format(wxT("Error: insufficient data ")
            "(got %d args, need at least %d)", numa, numq));
        return ;
    }


    return;
} //ControlTimer::Notify




// ----------------------------------------------------------------------------
// MyApp
// ----------------------------------------------------------------------------

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
    if ( !wxApp::OnInit() )
        return false;

    if ( !wxTaskBarIcon::IsAvailable() )
    {
        wxMessageBox
        (
            "There appears to be no system tray support in your current environment. This sample may not behave as expected.",
            "Warning",
            wxOK | wxICON_EXCLAMATION
        );
    }

    m_timer = new ControlTimer(this);
    
    // Create the main window
    gs_dialog = new MyDialog(wxT("NUT Client"));

    gs_dialog->Show(true);

    return true;
}


// ----------------------------------------------------------------------------
// MyDialog implementation
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(MyDialog, wxDialog)
    EVT_BUTTON(wxID_ABOUT, MyDialog::OnAbout)
    EVT_BUTTON(wxID_OK, MyDialog::OnOK)
    EVT_BUTTON(wxID_EXIT, MyDialog::OnExit)
    EVT_CLOSE(MyDialog::OnCloseWindow)
    EVT_ICONIZE(MyDialog::OnIconize)
END_EVENT_TABLE()


MyDialog::MyDialog(const wxString& title)
        : wxDialog(NULL, wxID_ANY, title)
{
    wxSizer * const sizerTop = new wxBoxSizer(wxVERTICAL);

    wxSizerFlags flags;
    flags.Border(wxALL, 10);

    sizerTop->Add(new wxStaticText
                      (
                        this,
                        wxID_ANY,
                        wxT("Press 'Hide me' to hide this window, Exit to quit.")
                      ), flags);
    sizerTop->Add(new wxTextCtrl(this, wxID_ANY, wxEmptyString, 
                                    wxDefaultPosition, wxSize(300, -1)), flags);
    sizerTop->Add(new wxStaticText
                      (
                        this,
                        wxID_ANY,
                        wxT("Double-click on the taskbar icon to show me again.")
                      ), flags);
    sizerTop->Add(new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
                       wxSize(100, -1), wxSP_ARROW_KEYS, 2000, 5000, 3493), flags);

    //sizerTop->AddStretchSpacer()->SetMinSize(200, 50);

    wxSizer * const sizerBtns = new wxBoxSizer(wxHORIZONTAL);
    sizerBtns->Add(new wxButton(this, wxID_ABOUT, wxT("&About")), flags);
    sizerBtns->Add(new wxButton(this, wxID_OK, wxT("&Hide")), flags);
    sizerBtns->Add(new wxButton(this, wxID_EXIT, wxT("E&xit")), flags);

    sizerTop->Add(sizerBtns, flags.Align(wxALIGN_CENTER_HORIZONTAL));
    SetSizerAndFit(sizerTop);
    Centre();

    m_taskBarIcon = new MyTaskBarIcon();
    if(NULL == m_taskBarIcon)
    {
        wxLogError("m_taskBarIcon==NULL");
    }
    // we should be able to show up to 128 characters on recent Windows versions
    // (and 64 on Win9x)
    if ( !m_taskBarIcon->SetIcon(wxICON(nutclient),
                                 "NUT Client\n"
                                 "With a very, very, very, very\n"
                                 "long tooltip whose length is\n"
                                 "greater than 64 characters.") )
    {
        wxLogError(wxT("Could not set icon."));
    }

#if defined(__WXOSX__) && wxOSX_USE_COCOA
    m_dockIcon = new MyTaskBarIcon(wxTBI_DOCK);
    if ( !m_dockIcon->SetIcon(wxICON(nutclient)) )
    {
        wxLogError(wxT("Could not set icon."));
    }
#endif
}

MyDialog::~MyDialog()
{
    delete m_taskBarIcon;
#if defined(__WXCOCOA__)
    delete m_dockIcon;
#endif
}

void MyDialog::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    static const char * const title = "About wxWidgets Taskbar Sample";
    static const char * const message
        = "wxWidgets sample showing wxTaskBarIcon class\n"
          "\n"
          "(C) 1997 Julian Smart\n"
          "(C) 2007 Vadim Zeitlin";

#if defined(__WXMSW__) && wxUSE_TASKBARICON_BALLOONS
    m_taskBarIcon->ShowBalloon(title, message, 15000, wxICON_INFORMATION);
#else // !__WXMSW__
    wxMessageBox(message, title, wxICON_INFORMATION|wxOK, this);
#endif // __WXMSW__/!__WXMSW__
}

void MyDialog::OnOK(wxCommandEvent& WXUNUSED(event))
{
    Show(false);
}

void MyDialog::OnExit(wxCommandEvent& WXUNUSED(event))
{
    Close(true);
}

void MyDialog::OnCloseWindow(wxCloseEvent& WXUNUSED(event))
{
    Destroy();
}

void MyDialog::OnIconize(wxIconizeEvent& WXUNUSED(event))
{
    wxMessageBox("Got Here", "taskbaricon sample",  wxICON_INFORMATION|wxOK, this);
}


// ----------------------------------------------------------------------------
// MyTaskBarIcon implementation
// ----------------------------------------------------------------------------

enum
{
    PU_CONNECT = 10001,
    PU_SETTINGS,
    PU_EXIT,
    PU_HELP,
    PU_ABOUT,
};


BEGIN_EVENT_TABLE(MyTaskBarIcon, wxTaskBarIcon)
    EVT_MENU(PU_CONNECT, MyTaskBarIcon::OnMenuConnect)
    EVT_MENU(PU_EXIT,    MyTaskBarIcon::OnMenuExit)
    EVT_MENU(PU_SETTINGS,MyTaskBarIcon::OnMenuSettings)
    EVT_MENU(PU_HELP,    MyTaskBarIcon::OnMenuHelp)
    EVT_MENU(PU_ABOUT,   MyTaskBarIcon::OnMenuAbout)
    EVT_TASKBAR_LEFT_DCLICK  (MyTaskBarIcon::OnLeftButtonDClick)
END_EVENT_TABLE()

void MyTaskBarIcon::OnMenuSettings(wxCommandEvent& )
{
    gs_dialog->Show(true);
}

void MyTaskBarIcon::OnMenuExit(wxCommandEvent& )
{
    gs_dialog->Close(true);
}

static bool connected = false;

void MyTaskBarIcon::OnMenuConnect(wxCommandEvent& )
{

    /* THIS COMPILES BUT IS WRONG!!!!
     *
     * NEED TO USE THE MEMBER IN THE APP CLASS
     *
     */

    MyUPS * ups;



    if(false == connected)
    {
        wxString upsName = wxString(wxT("cp685avr"));
        //wxString hostName = wxString(wxT("little-harbor.local."));
        wxString hostName = wxString(wxT("10.0.1.2"));

        ups = new MyUPS(upsName.c_str(), hostName.c_str());

        if (upscli_connect(ups->GetConnection(), ups->GetHostname(), ups->GetPort(),
            UPSCLI_CONN_TRYSSL) < 0) 
        {
            wxLogError(wxString::Format(wxT("Error: %s"), 
                                            upscli_strerror(ups->GetConnection())));
        }

        menu->SetLabel(PU_CONNECT, wxT("&Disconnect from server"));

        wxIcon icon(GreenLEDOn_xpm);
        if (!SetIcon(icon, wxT("Connected to server")))
            wxMessageBox(wxT("Could not set new icon."));

        connected = true;
        wxLogVerbose(wxT("Connected"));
    }
    else
    {
        delete ups;

        wxIcon icon(LEDOff_xpm);
        if (!SetIcon(icon, wxT("Connected to server")))
            wxMessageBox(wxT("Could not set new icon."));

        menu->SetLabel(PU_CONNECT, wxT("&Connect to NUT server"));

        connected = false;
        wxLogVerbose(wxT("DISConnected"));
    }
}

void MyTaskBarIcon::OnMenuHelp(wxCommandEvent&)
{
    wxMessageBox(wxT("Could not set new icon."));
}

void MyTaskBarIcon::OnMenuAbout(wxCommandEvent&)
{
    wxMessageBox(wxT("You clicked on a submenu!"));
}

// Overridables
wxMenu *MyTaskBarIcon::CreatePopupMenu()
{
    menu = new wxMenu;
    menu->Append(PU_CONNECT, wxT("&Connect to NUT server"));
    menu->AppendSeparator();
    menu->Append(PU_SETTINGS, wxT("&Settings"));
    menu->AppendSeparator();
    menu->Append(PU_HELP, wxT("&Help"));
    menu->Append(PU_ABOUT, wxT("&About"));
    /* OSX has built-in quit menu for the dock menu, but not for the status item */
#ifdef __WXOSX__ 
    if ( OSXIsStatusItem() )
#endif
    {
        menu->AppendSeparator();
        menu->Append(PU_EXIT,    wxT("E&xit"));
    }
    return menu;
}

void MyTaskBarIcon::OnLeftButtonDClick(wxTaskBarIconEvent&)
{
    gs_dialog->Show(true);
}


MyUPS::MyUPS(const char * upsName, const char * hostName, int portNo)
{
    connection = (UPSCONN_t *)malloc(sizeof(UPSCONN_t));
    if(NULL != connection)
    {
        upsname = (char *)upsName;
        hostname = (char *)hostName;
        port = portNo;
    }
};


MyUPS::~MyUPS()
{
    if (connection) 
    {
        upscli_disconnect(connection);
    }

    //free(upsname);
    //free(hostname);
    free(connection);
}




