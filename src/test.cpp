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

#if 0
	// draw some text
	dc.DrawText(wxT("Testing"), 40, 60);

	// draw a circle
	dc.SetBrush(*wxGREEN_BRUSH); // green filling
	dc.SetPen( wxPen( wxColor(255,0,0), 5 ) ); // 5-pixels-thick red outline
	dc.DrawCircle( wxPoint(200,100), 25 /* radius */ );

	// draw a rectangle
	dc.SetBrush(*wxBLUE_BRUSH); // blue filling
	dc.SetPen( wxPen( wxColor(255,175,175), 10 ) ); // 10-pixels-thick pink outline
	dc.DrawRectangle( 300, 100, 400, 200 );

	// draw a line
	dc.SetPen( wxPen( wxColor(0,0,0), 3 ) ); // black line, 3 pixels thick
	dc.DrawLine( 300, 100, 700, 300 ); // draw line across the rectangle

	// Look at the wxDC docs to learn how to draw other stuff
#endif

	wxRect rect = GetClientRect();

	if(rect.width == 0 || rect.height == 0)
	{
		return;
	}

	cout << "GCR: wid " << rect.width << ", hgt " << rect.height << endl;

	// If it's GTK then use the gdk_cairo_create() method. The GdkDrawable object
	// is stored in m_window of the wxPaintDC.
	cairo_t* cr = gdk_cairo_create(dc.GetGDKWindow());
//    Render(cairo_image, rect.width, rect.height);

#if 0
cairo_text_extents_t extents;

const char *utf8 = "cairo";
double x,y;

cairo_select_font_face (cr, "Sans",
    CAIRO_FONT_SLANT_NORMAL,
    CAIRO_FONT_WEIGHT_NORMAL);

cairo_set_font_size (cr, 100.0);
cairo_text_extents (cr, utf8, &extents);

x=25.0;
y=150.0;

cairo_move_to (cr, x,y);
cairo_show_text (cr, utf8);

/* draw helping lines */
cairo_set_source_rgba (cr, 1, 0.2, 0.2, 0.6);
cairo_set_line_width (cr, 6.0);
cairo_arc (cr, x, y, 10.0, 0, 2*M_PI);
cairo_fill (cr);
cairo_move_to (cr, x,y);
cairo_rel_line_to (cr, 0, -extents.height);
cairo_rel_line_to (cr, extents.width, 0);
cairo_rel_line_to (cr, extents.x_bearing, -extents.y_bearing);
cairo_stroke (cr);
#endif

#define DATALEN 75000
	float data[DATALEN];
	float width = rect.width;
	float height = rect.height;

	cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
	cairo_rectangle(cr, 0, 0, width, height);
	cairo_fill(cr);

	cairo_set_source_rgba(cr, 1.0, 0.2, 0.2, 0.2);

	for (int i=0; i<DATALEN; i++) {
		float x = (((float)(i) / (float)(DATALEN)) * 2.0) - 1.0;
		x *= 50.0;
		data[i] = x == 0.0 ? 1.0 : sin(x)/(x);
//		data[i] = ((float)rand() / ((float)RAND_MAX) * 2.0) - 1.0;
	}

	// plot
	float CountsPerX = width / ((float)DATALEN);	// DATALEN = X_RANGE
	float CountsPerY = height / 2.0;				// 2.0 = Y_RANGE

	cairo_move_to(cr, 0,  height - ((data[0] + 1.0) * CountsPerY));
	for (int i=0; i<DATALEN; i++) {
//		cairo_rectangle(cr, (float)(i) * CountsPerX, height - ((data[i] + 1.0) * CountsPerY), 1.0, 1.0);
		cairo_line_to(cr, (float)(i) * CountsPerX, height - ((data[i] + 1.0) * CountsPerY));
	}
//	cairo_fill(cr);
	cairo_stroke(cr);

	cairo_destroy(cr);
}
