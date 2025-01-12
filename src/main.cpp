#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "MyFrame.h"


class MyApp : public wxApp
{
public:
    virtual bool OnInit() {
        MyFrame* frame = new MyFrame(_T("wx+VTK"), wxPoint(50, 50), wxSize(800, 600));
        frame->Show(TRUE);
        return TRUE;
    };
};

IMPLEMENT_APP(MyApp)




