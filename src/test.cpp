#include "wx/wx.h"
#include "wx/sizer.h"

#include "ChartPanel.h"

const double PI = 2*acos(0.0);

class MyApp: public wxApp {
	private:
		bool OnInit();

		wxFrame *frame;
		ChartPanel * drawPane;
	public:

};

IMPLEMENT_APP(MyApp)


bool MyApp::OnInit()
{
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	frame = new wxFrame((wxFrame *)NULL, -1, wxT("Crystal chart plotter"), wxPoint(50,50), wxSize(800,600));

	drawPane = new ChartPanel( (wxFrame*) frame );
	sizer->Add(drawPane, 1, wxEXPAND);

	frame->SetSizer(sizer);
	frame->SetAutoLayout(true);

	frame->Show();
	return true;
}

//////////////////////////////////////////////////


