#include <fstream>
#include <iostream>

#include "wx/wx.h"
#include "wx/sizer.h"

#include "ChartPanel.h"

using namespace std;

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


#if 0
#define DATALEN 10000
	// generate some random data
	float *x = new float[DATALEN];
	float *data = new float[DATALEN];

//#define DBG_GEN_LINEAR
#define DBG_GEN_RANDOM
//#define DBG_GEN_SINC


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
		data[i] = (!(i % (DATALEN/4)) ? (i / (DATALEN/4.0)) * 200.0 : (rand() % 400)) + k;
	}
#endif

#ifdef DBG_GEN_SINC
	for (int i=0; i<DATALEN; i++) {
#define XF (((float)i) / ((float)DATALEN))
#define XK (-20.0 + (XF * 45.0))
		if (XK != 0.0) {
			data[i] = sin(XK * 3.14159) / (XK * 3.14159);
		} else {
			data[i] = 1.0;
		}
		x[i] = XK;
#undef XF
#undef XK
	}
#endif
#endif

	ifstream *ifs = new ifstream("discim.dfi", ifstream::in | ifstream::binary);

	char buf[4];
	ifs->read(buf, 4); // magic
	cout << buf[0] << buf[1] << buf[2] << buf[3] << endl;
	ifs->read(buf, 2); // cyl
	ifs->read(buf, 2); // head
	ifs->read(buf, 2); // sec

	ifs->read(buf, 4); // cyl
	size_t datlen =
		((unsigned char)buf[0] << 24) |
		((unsigned char)buf[1] << 16) |
		((unsigned char)buf[2] <<  8) |
		((unsigned char)buf[3]);
	char *bigbuf = new char[datlen];
	ifs->read(bigbuf, datlen);
	delete ifs;

	float *x = new float[datlen];
	float *data = new float[datlen];
	size_t bufpos;

	{
		size_t carry = 0, abspos = 0;
		bufpos = 0;
		for (size_t i=0; i<datlen; i++) {
			unsigned char byte = static_cast<unsigned char>(bigbuf[i]);
			if ((byte & 0x7f) == 0x7f) {
				// carry
				carry += 127;
				abspos += 127;
			} else if ((byte & 0x80) != 0) {
				// index
				carry += (byte & 0x7f);
				abspos += (byte & 0x7f);
				cout << "Index at abspos=" << abspos << endl;
			} else {
				abspos += (byte & 0x7f);
				carry += (byte & 0x7f);
				if (carry < 1500) {
					x[bufpos] = bufpos;
					data[bufpos] = static_cast<float>(carry) / 100.0e6;	// 100MHz
					bufpos++;
				}
				carry = 0;
			}
		}
	}
	delete bigbuf;

	drawPane->setDataSource(x, data, bufpos);

	drawPane->autoScale();
	drawPane->YMIN=0;
	drawPane->YMAX=5.0e-6;

	frame->SetSizer(sizer);
	frame->SetAutoLayout(true);

	frame->Show();
	return true;
}

