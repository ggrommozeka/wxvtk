#include "wx/wx.h"
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




