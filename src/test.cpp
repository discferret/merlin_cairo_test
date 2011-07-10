#include "wx/wx.h"
#include "wx/sizer.h"

#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <cairo.h>
#include <cmath>
#include <iostream>
using namespace std;

const double PI = 2*acos(0.0);

class BasicDrawPane : public wxPanel {

	public:
		BasicDrawPane(wxFrame* parent);

		// some useful events
		/*
		 void mouseMoved(wxMouseEvent& event);
		 void mouseDown(wxMouseEvent& event);
		 void mouseWheelMoved(wxMouseEvent& event);
		 void mouseReleased(wxMouseEvent& event);
		 void rightClick(wxMouseEvent& event);
		 void mouseLeftWindow(wxMouseEvent& event);
		 void keyPressed(wxKeyEvent& event);
		 void keyReleased(wxKeyEvent& event);
		 */

		void OnPaint(wxPaintEvent &WXUNUSED(event));
		void OnSize(wxSizeEvent& event)
		{
			wxRect rect = GetClientRect();
			//if(IsExposed(rect.x, rect.y, rect.width, rect.height))
			{
				Refresh(false);
			}
			event.Skip();
		}
};


class MyApp: public wxApp {
	private:
		bool OnInit();

		wxFrame *frame;
		BasicDrawPane * drawPane;
	public:

};

IMPLEMENT_APP(MyApp)


bool MyApp::OnInit()
{
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	frame = new wxFrame((wxFrame *)NULL, -1,  wxT("Hello wxDC"), wxPoint(50,50), wxSize(800,600));

	drawPane = new BasicDrawPane( (wxFrame*) frame );
	sizer->Add(drawPane, 1, wxEXPAND);

	frame->SetSizer(sizer);
	frame->SetAutoLayout(true);

	frame->Show();
	return true;
}

BasicDrawPane::BasicDrawPane(wxFrame* parent) :
	wxPanel(parent)
{
	Connect(this->GetId(),
			wxEVT_SIZE,
			wxSizeEventHandler(BasicDrawPane::OnSize));
	Connect(wxID_ANY,
			wxEVT_PAINT,
			wxPaintEventHandler(BasicDrawPane::OnPaint));
}

/*
 * Called by the system of by wxWidgets when the panel needs
 * to be redrawn. You can also trigger this call by
 * calling Refresh()/Update().
 */

void BasicDrawPane::OnPaint(wxPaintEvent & evt)
{
	wxPaintDC dc(this);

	// Get client rect and do a sanity check
	wxRect rect = GetClientRect();

	if(rect.width == 0 || rect.height == 0)
	{
		return;
	}

	cout << "ClientRect: width=" << rect.width << ", height=" << rect.height << endl;

	// If we're running on wxGTK (GTK widget kit) then we can grab the GDKWindow
	// and feed it straight to Cairo.
	cairo_t* cr = gdk_cairo_create(dc.GetGDKWindow());

#define DATALEN 750
	// generate some random data
	float data[DATALEN];
	float k = rand() % 128;
	for (int i=0; i<DATALEN; i++) {
		data[i] = (!(i % 250) ? (DATALEN-i)*4.0 : (rand() % 50)) + k;
	}

	// based on logarithmic linechart w/ GDB+/VB.NET
	// http://www.computer-consulting.com/logplotter.htm
	// also see http://www.codeproject.com/KB/miscctrl/DataPlotter.aspx

	int LMARGIN = 5;	// TODO add the max width of Yaxis text to this
	int RMARGIN = 5;
	int TMARGIN = 5;
	int BMARGIN = 5;	// TODO add the max height of Xaxis text to this
	int WIDTH = rect.width - LMARGIN - RMARGIN;
	int HEIGHT = rect.height - TMARGIN - BMARGIN;
	float OUTER_BORDER_WIDTH = 2.0;

	// draw outer box and background
	cairo_rectangle(cr, LMARGIN, TMARGIN, WIDTH, HEIGHT);
	cairo_set_source_rgb(cr, 1,1,1);	// CHARTAREA_FILL_COLOUR
	cairo_fill_preserve(cr);
	cairo_set_source_rgb(cr, 0,0,0);	// OUTER_BORDER_COLOUR
	cairo_set_line_width(cr, OUTER_BORDER_WIDTH);		// OUTER_BORDER_WIDTH
	cairo_stroke(cr);

	// find X and Y minima and maxima
	float XMIN = INFINITY, YMIN=INFINITY;
	float XMAX = -INFINITY, YMAX=-INFINITY;
	for (int i=0; i<DATALEN; i++) {
		float x = ((float)i);
		if (x < XMIN) XMIN = x;
		if (x > XMAX) XMAX = x;
		if (data[i] < YMIN) YMIN = data[i];
		if (data[i] > YMAX) YMAX = data[i];
	}

	cout << "XMin=" << XMIN << ", XMax=" << XMAX << ", XRange=" << XMAX-XMIN+1 << endl;
	cout << "YMin=" << YMIN << ", YMax=" << YMAX << ", YRange=" << YMAX-YMIN+1 << endl;

	cairo_set_source_rgba(cr, 0, 0, 0, 0.25);	// AXIS_COLOUR
	cairo_set_line_width(cr, 1.0);				// AXIS_WIDTH

#ifdef AXIS_DEBUG
	cairo_set_source_rgba(cr, 1.0, 0, 0, 1.0);	// AXIS_COLOUR
	double dashes[] = {5.0, 5.0};		// ink/skip
	cairo_set_dash(cr, dashes, sizeof(dashes)/sizeof(dashes[0]), 0.0);
	cairo_set_line_width(cr, 1.0);		// AXIS_WIDTH
#endif

	// draw Y axis
	// If AXIS_WIDTH == 1.0, we need a fudge factor of 0.5 to stop Cairo AAing the 1px line into 2px 50%-alpha
	float fudge = 0.5;
#ifndef SKIP_Y_AXIS
	if (!true) {
		const int LOGBASE = 10;

		// LOGARITHMIC AXIS
		int n = (int)round((log1p(YMAX + 1)/log1p(LOGBASE)) - (log1p(YMIN)/log1p(LOGBASE)));
		if (n == 0) n = 1.0;	// avoid divide by zero
		float d = HEIGHT / ((float)n);
		for (int i=0; i<=n; i++) {
			float y = TMARGIN + (HEIGHT - ((float)i * d));
			if (i < n) {	// do not draw detailed gradations past the border
				for (int j=1; j<=LOGBASE; j++) {
					int dl = (int)((log1p(LOGBASE-j)/log1p(LOGBASE)) * d);
					cairo_move_to(cr, LMARGIN + fudge, round(y - dl) + fudge);
					cairo_line_to(cr, LMARGIN + WIDTH + fudge, round(y - dl) + fudge);
				}
			}
		}
	} else {
		// LINEAR AXIS
//		int n = (YMAX - YMIN + 1) / HEIGHT;			// number of grid lines; one every ten N steps
//		if ((int)(YMAX - YMIN + 1) % HEIGHT) n++;	// add one if we have a partial grid unit
		int n = 10;
		float d = HEIGHT / ((float)n);		// pixels between grid lines
		for (int i=0; i<=n; i++) {
			float y = TMARGIN + (HEIGHT - ((float)i * d));	// Y position of this grid line
			if (i < n) {	// do not draw detailed gradations past the border
				cairo_move_to(cr, LMARGIN + fudge, round(y) + fudge);
				cairo_line_to(cr, LMARGIN + WIDTH + fudge, round(y) + fudge);
			}
		}
	}
#endif

#ifndef SKIP_X_AXIS
	// DRAW X AXIS
	if (!true) {
		const int LOGBASE = 10;

		// LOGARITHMIC X AXIS
		int n = (int)round((log1p(XMAX + 1)/log1p(LOGBASE)) - (log1p(XMIN)/log1p(LOGBASE)));
		if (n == 0) n = 1.0;	// avoid divide by zero
		float d = WIDTH / ((float)n);
		for (int i=0; i<=n; i++) {
			float x = LMARGIN + ((float)i * d);
			if (i < n) {	// do not draw detailed gradations past the border
				for (int j=1; j<=LOGBASE; j++) {
					int dl = (int)((log1p(LOGBASE-j)/log1p(LOGBASE)) * d);
					cairo_move_to(cr, round(x + dl) + fudge, TMARGIN + fudge);
					cairo_line_to(cr, round(x + dl) + fudge, TMARGIN + HEIGHT + fudge);
				}
			}
		}
	} else {
		// LINEAR X AXIS
//		int n = (XMAX - XMIN + 1) / 100;			// number of grid lines; one every 100 N steps
//		if ((int)(XMAX - XMIN + 1) % 100) n++;	// add one if we have a partial grid unit
		int n = 10;
		float d = WIDTH / ((float)n);		// pixels between grid lines
		for (int i=0; i<=n; i++) {
			float x = LMARGIN + ((float)i * d);	// X position of this grid line
			if (i < n) {	// do not draw detailed gradations past the border
				cairo_move_to(cr, round(x) + fudge, TMARGIN + fudge);
				cairo_line_to(cr, round(x) + fudge, TMARGIN + HEIGHT + fudge);
			}
		}
	}
#endif
	// stroke the grid
	cairo_stroke(cr);

	// ---- draw the graph ----
	cairo_set_dash(cr, NULL, 0, 0);
	cairo_set_source_rgba(cr, 0.00, 0.75, 0.00, 1.0);	// PLOT_COLOUR
	cairo_set_line_width(cr, 1.0);						// PLOT_WIDTH

	float d = (HEIGHT-TMARGIN-BMARGIN) / ((float)(YMAX - YMIN));		// one Y pixel = this number of data units

	// move to position of 1st data point
	cairo_move_to(cr, LMARGIN + (OUTER_BORDER_WIDTH/2.0), TMARGIN + (HEIGHT-((data[0] - YMIN)*d)));

	// draw other data points
	for (size_t i=1; i<DATALEN; i++) {
		float x = ((float)i)*((float)WIDTH/(float)DATALEN);
		cairo_line_to(cr, LMARGIN + (OUTER_BORDER_WIDTH/2.0) + x, TMARGIN + (HEIGHT-((data[i] - YMIN)*d)) - 1.0);
	}

	// stroke the chart
	cairo_stroke(cr);

	// we're done with the cairo reference. destroy it.
	cairo_destroy(cr);
}
