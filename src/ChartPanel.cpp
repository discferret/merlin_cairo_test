#include <cmath>
#include <iostream>

#include "wx/wx.h"

#if defined(__WXGTK__)
 #include <gdk/gdk.h>
 #include <gtk/gtk.h>
 #include <cairo.h>
#elif defined(__WXOSX_COCOA__)
 #include "wx/dcgraph.h"
 #include "wx/osx/private.h"
 #include <cairo.h>
 #include <cairo-quartz.h>
#else
 /* didn't recognise platform ID. TODO: add support for __WXMSW__ */
 #error "Unsupported platform. ChartPanel only works on WXGTK and WXOSX_COCOA."
#endif

#include "ChartPanel.h"

using namespace std;

void InitColour(COLOUR *co, float r, float g, float b, float a)
{
	co->r = r;
	co->g = g;
	co->b = b;
	co->a = a;
}

ChartPanel::ChartPanel(wxFrame* parent) :
	wxPanel(parent)
{
	// Set some sane defaults for config parameters
	LMARGIN = RMARGIN = TMARGIN = BMARGIN = 5;

	LogBase = 10;

	OuterBorderWidth = 2;
	InitColour(&ChartBorderColour, 0.0, 0.0, 0.0, 1.0);
	InitColour(&ChartBackgroundColour, 1.0, 1.0, 1.0, 1.0);

	AxisLineWidth = 1;
	InitColour(&AxisLineColour, 0.0, 0.0, 0.0, 0.25);

	XAxisType = AXIS_LOG;
	YAxisType = AXIS_LOG;

	PlotType = PLOT_LINE;
	PlotType = PLOT_SCATTER;

	if (PlotType == PLOT_LINE) {
		// Green, no transparency (ideal for line plots)
		InitColour(&PlotColour, 0.00, 0.75, 0.00, 1.00);
	} else {
		// Kryoflux blue (for scatter plots) -- TODO -- FIXME: change this!
		InitColour(&PlotColour, 0.0/255.0, 86.0/255.0, 245.0/255.0, /*0.20*/1.0-(241.0/255.0));
		InitColour(&PlotColour, 0.0/255.0, 86.0/255.0, 245.0/255.0, 0.20);
	}
	PlotLineWidth = 1;

	// Set up event handlers
	Connect(this->GetId(),
			wxEVT_SIZE,
			wxSizeEventHandler(ChartPanel::OnSize));
	Connect(wxID_ANY,
			wxEVT_PAINT,
			wxPaintEventHandler(ChartPanel::OnPaint));
}


/*
 * Called by the system of by wxWidgets when the panel needs
 * to be redrawn. You can also trigger this call by
 * calling Refresh()/Update().
 */
void ChartPanel::OnPaint(wxPaintEvent & evt)
{
	wxPaintDC dc(this);

	// Get client rect and do a sanity check
	wxRect rect = GetClientRect();

	if(rect.width == 0 || rect.height == 0)
	{
		return;
	}

#ifndef NDEBUG
	cout << __func__ << " - ClientRect: width=" << rect.width << ", height=" << rect.height << endl;
#endif

#if defined(__WXGTK__)
	// If we're running on wxGTK (GTK widget kit) then we can grab the GDKWindow
	// and feed it straight to Cairo.
	cairo_t* cr = gdk_cairo_create(dc.GetGDKWindow());
#elif defined(__WXOSX_COCOA__)
	CGContextRef context = (CGContextRef) static_cast<wxGCDCImpl *>(dc.GetImpl())->GetGraphicsContext()->GetNativeContext();

	if(context == 0)
	{
		return;
	}

	cairo_surface_t* cairo_surface = cairo_quartz_surface_create_for_cg_context(context, rect.width, rect.height);
	cairo_t* cr = cairo_create(cairo_surface);
#endif

	// Render the chart
	Render(cr, rect.width, rect.height);

#if defined(__WXOSX_COCOA__)
	cairo_surface_flush(cairo_surface);
	CGContextFlush( context );
	cairo_surface_destroy(cairo_surface);
#endif

	// we're done with the cairo reference. destroy it.
	cairo_destroy(cr);
}

void ChartPanel::Render(cairo_t *cr, long width, long height)
{
#define DATALEN 9750
	// generate some random data
	float data[DATALEN];
#ifdef DBG_GEN_LINEAR
	for (int i=0; i<DATALEN; i++) {
		data[i] = i;
	}
#else
	float k = rand() % 128;
	cout << "K-value = " << k << endl;
	for (int i=0; i<DATALEN; i++) {
		if (i < (DATALEN / 4))
			data[i] = k + (rand() % 100);
		else
			data[i] = (!(i % (DATALEN/4)) ? (i / (DATALEN/4.0)) * 200.0 : (rand() % 50)) + k;
	}
#endif

	// based on logarithmic linechart w/ GDB+/VB.NET
	// http://www.computer-consulting.com/logplotter.htm
	// also see http://www.codeproject.com/KB/miscctrl/DataPlotter.aspx

	// LMARGIN should have the maximal width of the Y-axis value text added to it.
	// BMARGIN should have the maximal height of the X-axis value text added to it.

	// calculate width and height TODO should be long?
	float WIDTH = width - LMARGIN - RMARGIN - OuterBorderWidth;
	float HEIGHT = height - TMARGIN - BMARGIN - OuterBorderWidth;

	// draw outer box and background
	cairo_rectangle(cr, LMARGIN, TMARGIN, WIDTH, HEIGHT);
	cairo_set_source_rgb(cr, ChartBackgroundColour.r, ChartBackgroundColour.g, ChartBackgroundColour.b);
	cairo_fill_preserve(cr);
	cairo_set_source_rgb(cr, ChartBorderColour.r, ChartBorderColour.g, ChartBorderColour.b);
	cairo_set_line_width(cr, OuterBorderWidth);
	cairo_stroke(cr);

	// find X and Y data minima and maxima
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

	cairo_set_source_rgba(cr, AxisLineColour.r, AxisLineColour.g, AxisLineColour.b, AxisLineColour.a);
	cairo_set_line_width(cr, AxisLineWidth);

#if 0
	// Dashed lines for axis subdivisions
	cairo_set_source_rgba(cr, 1.0, 0, 0, 1.0);	// AXIS_COLOUR
	double dashes[] = {5.0, 5.0};		// ink/skip
	cairo_set_dash(cr, dashes, sizeof(dashes)/sizeof(dashes[0]), 0.0);
	cairo_set_line_width(cr, AxisLineWidth);
#endif

	// draw Y axis
	// If AXIS_WIDTH == 1.0, we need a fudge factor of 0.5 to stop Cairo AAing the 1px line into 2px 50%-alpha
	float axis_fudge = AxisLineWidth / 2.0;

	if (YAxisType == AXIS_LOG) {
		// LOGARITHMIC AXIS
		int n = (int)round((log1p(YMAX + 1)/log1p(LogBase)) - (log1p(YMIN)/log1p(LogBase)));
		if (n == 0) n = 1.0;	// avoid divide by zero
		float d = HEIGHT / ((float)n);
		for (int i=0; i<=n; i++) {
			float y = TMARGIN + (HEIGHT - ((float)i * d));
			if (i < n) {	// do not draw detailed gradations past the border
				for (int j=1; j<=LogBase; j++) {
					int dl = (int)((log1p(LogBase-j)/log1p(LogBase)) * d);
					cairo_move_to(cr, LMARGIN + axis_fudge, round(y - dl) + axis_fudge);
					cairo_line_to(cr, LMARGIN + WIDTH + axis_fudge, round(y - dl) + axis_fudge);
				}
			}
		}
	} else if (YAxisType == AXIS_LIN) {
		// LINEAR AXIS
//		int n = (YMAX - YMIN + 1) / HEIGHT;			// number of grid lines; one every ten N steps
//		if ((int)(YMAX - YMIN + 1) % HEIGHT) n++;	// add one if we have a partial grid unit
		int n = 10;
		float d = HEIGHT / ((float)n);		// pixels between grid lines
		for (int i=0; i<=n; i++) {
			float y = TMARGIN + (HEIGHT - ((float)i * d));	// Y position of this grid line
			if (i < n) {	// do not draw detailed gradations past the border
				cairo_move_to(cr, LMARGIN + axis_fudge, round(y) + axis_fudge);
				cairo_line_to(cr, LMARGIN + WIDTH + axis_fudge, round(y) + axis_fudge);
			}
		}
	}

	// draw X axis
	if (XAxisType == AXIS_LOG) {
		// LOGARITHMIC X AXIS
		int n = (int)round((log1p(XMAX + 1)/log1p(LogBase)) - (log1p(XMIN)/log1p(LogBase)));
		if (n == 0) n = 1.0;	// avoid divide by zero
		float d = WIDTH / ((float)n);
		for (int i=0; i<=n; i++) {
			float x = LMARGIN + ((float)i * d);
			if (i < n) {	// do not draw detailed gradations past the border
				for (int j=1; j<=LogBase; j++) {
					int dl = (int)((log1p(LogBase-j)/log1p(LogBase)) * d);
					cairo_move_to(cr, round(x + dl) + axis_fudge, TMARGIN + axis_fudge);
					cairo_line_to(cr, round(x + dl) + axis_fudge, TMARGIN + HEIGHT + axis_fudge);
				}
			}
		}
	} else if (XAxisType == AXIS_LIN) {
		// LINEAR X AXIS
//		int n = (XMAX - XMIN + 1) / 100;			// number of grid lines; one every 100 N steps
//		if ((int)(XMAX - XMIN + 1) % 100) n++;	// add one if we have a partial grid unit
		int n = 10;
		float d = WIDTH / ((float)n);		// pixels between grid lines
		for (int i=0; i<=n; i++) {
			float x = LMARGIN + ((float)i * d);	// X position of this grid line
			if (i < n) {	// do not draw detailed gradations past the border
				cairo_move_to(cr, round(x) + axis_fudge, TMARGIN + axis_fudge);
				cairo_line_to(cr, round(x) + axis_fudge, TMARGIN + HEIGHT + axis_fudge);
			}
		}
	}

	// stroke the grid
	cairo_stroke(cr);

	// Prepare to draw the chart
	cairo_set_source_rgba(cr, PlotColour.r, PlotColour.g, PlotColour.b, PlotColour.a);

	if (PlotType == PLOT_LINE) {
		// If PlotLineWidth == 1.0, we need a fudge factor of 0.5 to stop Cairo AAing the 1px line into 2px 50%-alpha
		float plot_fudge = PlotLineWidth / 2.0;

		cairo_set_dash(cr, NULL, 0, 0);
		cairo_set_line_width(cr, PlotLineWidth / 2.0);	// antialiasing is a bitch

		float xd = (float)(WIDTH  - OuterBorderWidth) / ((float)XMAX - (float)XMIN);
		float yd = (float)(HEIGHT - OuterBorderWidth) / ((float)YMAX - (float)YMIN);
		for (size_t i=0; i<DATALEN; i++) {
			float x = LMARGIN + (((float)/*xdata[i]*/i - (float)XMIN) * xd);
			float y = TMARGIN + ((HEIGHT - OuterBorderWidth) - ((float)data[i] - (float)YMIN) * yd);
			if (i == 0) {
				cairo_move_to(cr, x + plot_fudge, y + plot_fudge);
			} else {
				cairo_line_to(cr, x + plot_fudge, y + plot_fudge);
			}
		}

		// stroke the chart
		cairo_stroke(cr);
	} else if (PlotType == PLOT_SCATTER) {
		// ---- scatter plot, linear ----

		// For each block, call RECTANGLE then FILL. Calling fill() once right at
		// the end fubars the alpha calculations.
		//
		// TODO: fix issue with 'overdrawing' -- where boxes overlap the margin lines

		float yd = (HEIGHT-TMARGIN-BMARGIN) / ((float)(YMAX - YMIN));		// one Y pixel = this number of data units
		float xd = (WIDTH-LMARGIN-RMARGIN) / ((float)(XMAX - XMIN));			// one X pixel = this number of data units

		// calculate "ideal" block size
		float BSZ = (yd>xd) ? yd : xd;
		if (BSZ < 1.0) BSZ = 1.0;
		BSZ *= 2.0;
		float BSZH = BSZ / 2.0;

		// TODO: clip blocks along the margins (DO NOT plot outside the boundary line)
		for (size_t i=0; i<DATALEN; i++) {
			float x = LMARGIN + ((float)OuterBorderWidth/2.0) + ((float)i)*((float)WIDTH/(float)DATALEN);
			float y = TMARGIN + (HEIGHT-((data[i] - YMIN)*yd)) - 1.0;
			cairo_rectangle(cr, x-BSZH, y-BSZH, BSZ, BSZ);	// x, y, wid, hgt
			cairo_fill(cr);
		}
	}
}
