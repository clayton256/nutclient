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
    #include "images/YellowLEDOn.xpm"
    #include "images/nut.xpm"
#endif

#include <wx/log.h>
#include <wx/taskbar.h>
#include <wx/spinctrl.h>
#include <wx/notifmsg.h>
#include <wx/notifmsg.h>
#include <wx/config.h>
#include <wx/image.h>
#include <wx/icon.h>

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

MyUPS * g_ups;
MyTaskBarIcon   *m_taskBarIcon;

static MyDialog *gs_dialog = NULL;

// ============================================================================
// implementation
// ============================================================================



ControlTimer::ControlTimer(MyApp * myapp) : wxTimer()
{
    m_myApp = myapp;
    wxTimer::Start(5000);
} // ControlTimer c-tor


void ControlTimer::Notify()
{
    wxLogVerbose("ControlTimer:Notify");
    if(NULL != g_ups) g_ups->PollMaster();

    if(NULL != m_taskBarIcon)
    {
        m_taskBarIcon->IconUpdate(g_ups->GetStatus(), g_ups->GetBattLevel(),
            g_ups->GetUpsname(), g_ups->GetHostname());
    }
    return;
}


void MyUPS::PollMaster(void)
{
    int ret;
    unsigned int numq, numa;
    const char *query[4];
    char ** answer;
    char upsName[1024];
    enum upsStatus upsstatus = STATUS_NOCNXT;

    strncpy(upsName, (const char*)GetUpsname().mb_str(wxConvUTF8), 1023);

    wxLogVerbose(wxString::Format(wxT("logme: %s@%s"), upsName, GetHostname()));

    if(true == IsConnected())
    {
        query[0] = "VAR";
        query[1] = upsName;
        query[2] = "ups.status";
        numq = 3;

        ret = upscli_get(connection, numq, query, &numa, &answer);

        if (ret < 0) 
        {
            /* detect old upsd */
            if (upscli_upserror(connection) == UPSCLI_ERR_UNKCOMMAND) 
            {
                wxLogError(wxString::Format(wxT("UPS [%s]: Too old to monitor"), 
                                                                    upsName));
                return;
            }

            wxLogError(wxT("UPS [%s]: Unknown error"), upsName);
            /* some other error */
            return;
        }

        if (numa < numq) {
            wxLogError(wxString::Format(wxT("Error: insufficient data ")
                "(got %d args, need at least %d)", numa, numq));
            return;
        }

        wxLogVerbose(wxString::Format(wxT("status str: %s %d"), answer[numq], numq));
        //StartsWith(const wxChar *prefix, wxString *rest = NULL)
        unsigned int x;
        x = answer[numq][1] | (answer[numq][0] << 8);
        wxLogVerbose(wxString::Format(wxT("status hex: %x"), x));
        
        switch(x)
        {
            case 0x4F4C: /* 'OL' */
                upsstatus = STATUS_ONLINE;
                wxLogVerbose(wxString::Format(wxT("OL: %s"), answer[numq]));
                break;
            case 0x4F42: /* 'OB' */
                upsstatus = STATUS_ONBATT;
                wxLogVerbose(wxString::Format(wxT("OB: %s"), answer[numq]));
                break;
            case 0x4C42: /* 'LB' */
                upsstatus = STATUS_LOWBAT;
                wxLogVerbose(wxString::Format(wxT("LB: %s"), answer[numq]));
                break;
            case 0x4653: /* FSD */
                upsstatus = STATUS_SHUTDN;
                wxLogVerbose(wxString::Format(wxT("FSD: %s"), answer[numq]));
                break;
            default:
                upsstatus = STATUS_NOCNXT;
                wxLogVerbose(wxString::Format(wxT("Unknown Status: 0x%x %s"), 
                                                                x, answer[numq]));
                break;
        }

        query[0] = "VAR";
        query[1] = upsName;
        query[2] = "battery.charge";
        numq = 3;

        ret = upscli_get(connection, numq, query, &numa, &answer);

        if (ret < 0) 
        {
            /* detect old upsd */
            if (upscli_upserror(connection) == UPSCLI_ERR_UNKCOMMAND) 
            {
                wxLogError(wxString::Format(wxT("UPS [%s]: Too old to monitor"), 
                                                                        upsName));
                return;
            }

            wxLogError(wxT("UPS [%s]: Unknown error"), upsName);
            /* some other error */
            return;
        }

        if (numa < numq) {
            wxLogError(wxString::Format(wxT("Error: insufficient data ")
                "(got %d args, need at least %d)", numa, numq));
            return;
        }

        battLevel = atoi(answer[numq]);
        wxLogVerbose(wxString::Format(wxT("batt lvl str: %s %d"), answer[numq], numq));
    }
    else
    {
        upsstatus = STATUS_NOCNXT;
    }

    if(upsstatus != GetStatus())
    {
        SetStatus(upsstatus);
        if ( !wxNotificationMessage(wxString::Format(wxT("NUT: %s@%s"), 
                                                    GetUpsname(), GetHostname()),
                                    wxString::Format(wxT("Status:%s Batt:%d%%"),  
                                             upsStatusStrs[upsstatus], battLevel)
                                    ).Show())
        {
            wxLogVerbose("Failed to show notification message");
        }
    }

    return;
} //ControlTimer::Notify




// ----------------------------------------------------------------------------
// MyApp
// ----------------------------------------------------------------------------

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
    g_ups = NULL;

    if ( !wxApp::OnInit() )
        return false;

    wxLog::SetVerbose(false);
    FILE *logFile;
    logFile = fopen("/Users/mark/Projects/nutclient/trace.log","w");
    wxLogStderr *mStandardLog = new wxLogStderr(logFile);
    wxLog::SetActiveTarget(mStandardLog);

    if ( !wxTaskBarIcon::IsAvailable() )
    {
        wxMessageBox
        (
            "There appears to be no system tray support in your current environment. This sample may not behave as expected.",
            "Warning",
            wxOK | wxICON_EXCLAMATION
        );
    }

    // Create the main window
    gs_dialog = new MyDialog(wxT("NUT Client"));

    m_timer = new ControlTimer(this);

    // Don't show settings dialog on startup
    //gs_dialog->Show(true);

    return true;
}

int MyApp::OnExit()
{
    wxLogVerbose("MyApp::OnExit");

    // clean up: Set() returns the active config object as Get() does, but unlike
    // Get() it doesn't try to create one if there is none (definitely not what
    // we want here!)
    delete wxConfigBase::Set((wxConfigBase *) NULL);

    delete m_timer;

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

    sizerTop->Add(new wxStaticText(this, wxID_ANY,
                        wxT("Press 'Hide me' to hide this window, Exit to quit.")
                      ), flags);
    hostnameText = new wxTextCtrl(this, wxID_ANY, wxEmptyString,
                                    wxDefaultPosition, wxSize(300, -1));
    sizerTop->Add(hostnameText, flags);
    upsnameText = new wxTextCtrl(this, wxID_ANY, wxEmptyString,
                                    wxDefaultPosition, wxSize(300, -1));
    sizerTop->Add(upsnameText, flags);
    sizerTop->Add(new wxStaticText(this, wxID_ANY,
                        wxT("Double-click on the taskbar icon to show me again.")
                      ), flags);
    portSpin= new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
                       wxSize(100, -1), wxSP_ARROW_KEYS, 2000, 5000, PORT_DEFAULT);
    sizerTop->Add(portSpin, flags);

    //sizerTop->AddStretchSpacer()->SetMinSize(200, 50);

    wxSizer * const sizerBtns = new wxBoxSizer(wxHORIZONTAL);
    sizerBtns->Add(new wxButton(this, wxID_ABOUT, wxT("&About")), flags);
    sizerBtns->Add(new wxButton(this, wxID_OK, wxT("&Ok")), flags);
    sizerBtns->Add(new wxButton(this, wxID_EXIT, wxT("E&xit")), flags);

    sizerTop->Add(sizerBtns, flags.Align(wxALIGN_CENTER_HORIZONTAL));
    SetSizerAndFit(sizerTop);
    Centre();

    wxString upsName; // = wxString(wxT("cp685avr"));
    wxString hostName; // = wxString(wxT("10.0.1.2")); "little-harbor.local."
    m_config = wxConfigBase::Get(_T("nutclient"));
    m_config->Read(_T("hostname"), &hostName, wxT("hostname"));
    m_config->Read(_T("upsname"), &upsName, wxT("upsname"));
    int portNo = m_config->ReadLong(_T("port"), portNo);
    hostnameText->SetValue(hostName);
    upsnameText->SetValue(upsName);
    portSpin->SetValue(portNo);

    m_taskBarIcon = new MyTaskBarIcon();
    if(NULL == m_taskBarIcon)
    {
        wxLogError("m_taskBarIcon==NULL");
    }
    // we should be able to show up to 128 characters on recent Windows versions
    // (and 64 on Win9x)
    if ( !m_taskBarIcon->SetIcon(wxICON(nut_32),
                                 "NUT Client\n"
                                 "With a very, very, very, very\n"
                                 "long tooltip whose length is\n"
                                 "greater than 64 characters.") )
    {
        wxLogError(wxT("Could not set icon."));
    }

#if defined(__WXOSX__) && wxOSX_USE_COCOA
    m_dockIcon = new MyTaskBarIcon(wxTBI_DOCK);
    if ( !m_dockIcon->SetIcon(wxICON(nut_32)) )
    {
        wxLogError(wxT("Could not set icon."));
    }
#endif

    g_ups = new MyUPS();
    if(true)
    {
        g_ups->Connect(upsName, hostName, portNo);
    }

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
    static const char * const title = "About nutclient";
    static const char * const message
        = "A lite client to monitor a server UPS.\n"
          "\n"
          __DATE__ __TIME__ "\n"
          "(C) 2013 Mark Clayton";
//#define NUTCLIENT_BUILD = "__DATE__ __TIME__"

#if defined(__WXMSW__) && wxUSE_TASKBARICON_BALLOONS
    m_taskBarIcon->ShowBalloon(title, message, 15000, wxICON_INFORMATION);
#else // !__WXMSW__
    wxMessageBox(message, title, wxICON_INFORMATION|wxOK, this);
#endif // __WXMSW__/!__WXMSW__
}

void MyDialog::OnOK(wxCommandEvent& WXUNUSED(event))
{
    wxString upsName;
    wxString hostName;
    int portNo = PORT_DEFAULT;

    hostName = hostnameText->GetValue();
    upsName = upsnameText->GetValue();
    portNo = portSpin->GetValue();
    m_config->Write(_T("upsname"), upsName);
    m_config->Write(_T("hostname"), hostName);
    m_config->Write(_T("port"), portNo);
    g_ups->SetHostname(hostName);
    g_ups->SetUpsname(upsName);
    g_ups->SetPort(portNo);
    
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


void MyTaskBarIcon::OnMenuConnect(wxCommandEvent& )
{
    if(false == g_ups->IsConnected())
    {
        wxString upsName = g_ups->GetUpsname();
        wxString hostName = g_ups->GetHostname();
        int portNo = g_ups->GetPort();
        g_ups->Connect(upsName, hostName, portNo);

        wxLogVerbose(wxT("Connected"));
    }
    else
    {
        g_ups->Disconnect();

        wxLogVerbose(wxT("DISConnected"));
    }
}

void MyTaskBarIcon::OnMenuHelp(wxCommandEvent&)
{
    wxMessageBox(wxT("Help yourself."));
}

void MyTaskBarIcon::OnMenuAbout(wxCommandEvent&)
{
    wxMessageBox(wxT("About What?!?"));
}

// Overridables
wxMenu *MyTaskBarIcon::CreatePopupMenu()
{
    menu = new wxMenu;
    menu->AppendCheckItem(PU_CONNECT, wxT("&Connect to NUT server"));
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

void MyTaskBarIcon::IconUpdate(enum upsStatus status, unsigned int battLvl,
        wxString upsName, wxString hostName)
{
    wxLogVerbose("MyTaskBarIcon::IconUpdate");

    wxString iconBubbleMsg(wxString::Format(wxT("%s@%s:%s Batt:%d%%"),
                upsName, hostName, upsStatusStrs[status], battLvl));

    switch(status)
    {
        case STATUS_ONLINE:
            {
                if (!SetIcon(iconGreenLED, iconBubbleMsg))
                    wxLogVerbose(wxT("Could not set new icon."));
            }
            break;
        case STATUS_ONBATT:
            {
                if (!SetIcon(iconYellowLED, iconBubbleMsg))
                    wxLogVerbose(wxT("Could not set new icon."));
            }
            break;
        case STATUS_LOWBAT:
            {
                if (!SetIcon(iconRedLED, iconBubbleMsg))
                    wxLogVerbose(wxT("Could not set new icon."));
            }
            break;
        case STATUS_SHUTDN:
            {
                if (!SetIcon(iconRedLED, iconBubbleMsg))
                    wxLogVerbose(wxT("Could not set new icon."));
            }
            DoShutdown();
            break;
        case STATUS_NOCNXT:
            {
                if (!SetIcon(iconOffLED, iconBubbleMsg))
                    wxLogVerbose(wxT("Could not set new icon."));
            }
            break;
        default:
            wxLogVerbose("Shouldnt have a NON-STATUS_");
            break;
    }

    return;
}



MyUPS::MyUPS(void)
{
    wxLogVerbose(wxT("New UPS"));

    connection = (UPSCONN_t *)malloc(sizeof(UPSCONN_t));
    if(NULL != connection)
    {
        wxLogError(wxT("Error: connection is NULL"));
    }
    connected = false;
}

void MyUPS::Connect(wxString upsName, wxString hostName, int portNo)
{
    wxLogVerbose(wxString::Format(wxT("Connect to UPS: %s@%s:%d"), 
                                                upsname, hostname, portno));

    if(NULL == connection)
    {
        wxLogError(wxT("Error: connection is NULL"));
    }
    else
    {
        upsname = upsName;
        hostname = hostName;
        portno = portNo;
        if(upscli_connect(connection, hostname, portno, UPSCLI_CONN_TRYSSL) < 0) 
        {
            wxLogError(wxString::Format(wxT("Error: %s"), upscli_strerror(connection)));
        }
        else
        {
            connected = true;
        }
    }

    return;
};


void MyUPS::Disconnect(void)
{
    wxLogVerbose(wxString::Format(wxT("DISconnect UPS: %s@%s:%d"), 
                                                upsname, hostname, portno));

    if (true == IsConnected()) 
    {
        upscli_disconnect(connection);
        connected = false;
    }
}


MyUPS::~MyUPS()
{
    wxLogVerbose("MyUPS Delete");
    upsname = wxEmptyString;
    hostname = wxEmptyString;
    if(NULL != connection) free(connection);
    connection = NULL;
}




