#include "wx/wx.h"
#include "wx/sizer.h"

#include "ChartPanel.h"

//const double PI = 2*acos(0.0);

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

#define DATALEN 20000
	// generate some random data
	float *x = new float[DATALEN];
	float *data = new float[DATALEN];

//#define DBG_GEN_LINEAR
//#define DBG_GEN_RANDOM
#define DBG_GEN_SINC


#ifdef DBG_GEN_LINEAR
	for (int i=0; i<DATALEN; i++) {
		x[i] = i;
		data[i] = i;
	}
#endif

#ifdef DBG_GEN_RANDOM
	float k = rand() % 128;
	//cout << "K-value = " << k << endl;
	for (int i=0; i<DATALEN; i++) {
		x[i] = i;
		if (i < (DATALEN / 4))
			data[i] = k + (rand() % 100);
		else
			data[i] = (!(i % (DATALEN/4)) ? (i / (DATALEN/4.0)) * 200.0 : (rand() % 50)) + k;
	}
#endif

#ifdef DBG_GEN_SINC
	for (int i=0; i<DATALEN; i++) {
		x[i] = i;
#define XF (((float)i) / ((float)DATALEN))
#define XK (-20.0 + (XF * 40.0))
		if (XK != 0.0) {
			data[i] = sin(XK * 3.14159) / (XK * 3.14159);
		} else {
			data[i] = 1.0;
		}
#undef XF
#undef XK
	}
#endif

	drawPane->setDataSource(x, data, DATALEN);

	frame->SetSizer(sizer);
	frame->SetAutoLayout(true);

	frame->Show();
	return true;
}

