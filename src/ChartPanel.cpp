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
	wxPanel(parent),
	dataSrcX(NULL), dataSrcY(NULL), dataLength(0),
	LMARGIN(5), RMARGIN(5), TMARGIN(5), BMARGIN(5),
	LogBase(10),
	OuterBorderWidth(2), AxisLineWidth(1), PlotLineWidth(1),
	XAxisType(AXIS_LIN), YAxisType(AXIS_LIN),
	PlotType(PLOT_SCATTER),
	XMIN(0.0), XMAX(1.0),
	YMIN(0.0), YMAX(1.0)
{
	// Initialise colour parameters
	InitColour(&ChartBorderColour, 0.0, 0.0, 0.0, 1.0);
	InitColour(&ChartBackgroundColour, 1.0, 1.0, 1.0, 1.0);
	InitColour(&AxisLineColour, 0.0, 0.0, 0.0, 0.25);

	// FIXME - maybe just set to red...
	if (PlotType == PLOT_LINE) {
		// Green, no transparency (ideal for line plots)
		InitColour(&PlotColour, 0.00, 0.75, 0.00, 1.00);
	} else {
		// Kryoflux blue (for scatter plots) -- TODO -- FIXME: change this!
//		InitColour(&PlotColour, 0.0/255.0, 86.0/255.0, 245.0/255.0, /*0.20*/1.0-(241.0/255.0));
		InitColour(&PlotColour, 0.0/255.0, 86.0/255.0, 245.0/255.0, 0.20);
	}

	// Set up event handlers
	Connect(this->GetId(),
			wxEVT_SIZE,
			wxSizeEventHandler(ChartPanel::OnSize));
	Connect(wxID_ANY,
			wxEVT_PAINT,
			wxPaintEventHandler(ChartPanel::OnPaint));
}


/**
 * Auto-scale the chart.
 */
void ChartPanel::autoScale(void)
{
	// FIXME - sanity check - dataLength > 0, dataSrcY != 0
	if ((dataLength < 1) || (dataSrcY == NULL)) {
		// FIXME throw something sane
		throw 1;
	}

	// find X and Y data minima and maxima
	float _XMIN = INFINITY, _YMIN=INFINITY;
	float _XMAX = -INFINITY, _YMAX=-INFINITY;
	for (size_t i=0; i<this->dataLength; i++) {
		// If no X data is provided, use the offset pointer
		float x = (this->dataSrcX != NULL) ? ((float)this->dataSrcX[i]) : ((float)i);
		// Y data must be provided :)
		float y = ((float)this->dataSrcY[i]);
		if (x < _XMIN) _XMIN = x;
		if (x > _XMAX) _XMAX = x;
		if (y < _YMIN) _YMIN = y;
		if (y > _YMAX) _YMAX = y;
	}

#ifdef DEBUG
	cout << "XMin=" << _XMIN << ", XMax=" << _XMAX << ", XRange=" << _XMAX-_XMIN+1 << endl;
	cout << "YMin=" << _YMIN << ", YMax=" << _YMAX << ", YRange=" << _YMAX-_YMIN+1 << endl;
#endif

	this->XMIN = _XMIN; this->YMIN = _YMIN;
	this->XMAX = _XMAX; this->YMAX = _YMAX;
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
	if (this->dataSrcY == NULL) {
		return;
	}

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

	// calculate number of data units per X/Y pixel
	// one X pixel = xd data units; one Y pixel = yd data units.
	float xd = (XAxisType == AXIS_LIN) ?
		(WIDTH - ((float)OuterBorderWidth)/* - (plot_fudge*2.0)*/) / ((float)(XMAX - XMIN)) :							// Linear X axis
		(WIDTH - ((float)OuterBorderWidth)/* - (plot_fudge*2.0)*/) / ((float)log1p(XMAX - XMIN) / log1p(LogBase));		// Logarithmic X axis
	float yd = (YAxisType == AXIS_LIN) ?
		(HEIGHT - ((float)OuterBorderWidth)/* - (plot_fudge*2.0)*/) / ((float)(YMAX - YMIN)) :							// Linear Y axis
		(HEIGHT - ((float)OuterBorderWidth)/* - (plot_fudge*2.0)*/) / ((float)log1p(YMAX - YMIN) / log1p(LogBase));		// Logarithmic Y axis

	// Prepare to draw axes
	cairo_set_source_rgba(cr, AxisLineColour.r, AxisLineColour.g, AxisLineColour.b, AxisLineColour.a);
	cairo_set_line_width(cr, AxisLineWidth);

	// If AXIS_WIDTH == 1.0, we need a fudge factor of 0.5 to stop Cairo AAing the 1px line into 2px 50%-alpha
	float axis_fudge = AxisLineWidth / 2.0;

	// Draw Y axis
	// FIXME make ystep configurable -- either (YMAX-YMIN)/10.0 for "N divisions" or fixed span
	float YSTEP = (YMAX-YMIN)/10.0; //1.0;
	// Start at the nearest whole grid unit to the Y minima
	float Y = ((int)(YMIN / YSTEP)) * YSTEP;

	while (Y < YMAX) {
		float y = (YAxisType == AXIS_LIN) ?
			TMARGIN - (OuterBorderWidth/2.0) + (HEIGHT-((Y - YMIN)*yd)) :						// Linear Y axis
			TMARGIN - (OuterBorderWidth/2.0) + (HEIGHT-((log1p(Y - YMIN)/log1p(LogBase))*yd));	// Logarithmic Y axis

		if ((Y > (0.0-YSTEP)) && (Y < (0.0+YSTEP))) {
			// Solid line for zero line
			cairo_set_dash(cr, NULL, 0, 0);
			cairo_set_line_width(cr, AxisLineWidth * 2.0);
		} else {
			// Dashed lines for axis subdivisions
			double dashes[] = {5.0, 5.0};		// ink/skip
			cairo_set_dash(cr, dashes, sizeof(dashes)/sizeof(dashes[0]), 0.0);
			cairo_set_line_width(cr, AxisLineWidth);
		}

		cairo_move_to(cr, LMARGIN + axis_fudge, round(y) + axis_fudge);
		cairo_line_to(cr, LMARGIN + WIDTH + axis_fudge, round(y) + axis_fudge);
		cairo_stroke(cr);

		Y += YSTEP;
		if ((YAxisType == AXIS_LOG) && (Y >= (YSTEP * LogBase))) {
			YSTEP *= LogBase;
		}
	}

	// Draw X axis
	// FIXME make xstep configurable -- either (XMAX-XMIN)/10.0 for "N divisions" or fixed span
	float XSTEP = (XMAX-XMIN)/10.0;

	// Start at the nearest whole grid unit to the X minima
	float X = ((int)(XMIN / XSTEP)) * XSTEP;

	while (X < XMAX) {
		float x = (XAxisType == AXIS_LIN) ?
			LMARGIN + OuterBorderWidth + ((X - XMIN) * xd) :									// Linear X axis
			LMARGIN + OuterBorderWidth + (log1p(X - XMIN)/log1p(LogBase) * xd);					// Logarithmic X axis

		if ((X > (0.0-XSTEP)) && (X < (0.0+XSTEP))) {
			// Solid line for zero line
			cairo_set_dash(cr, NULL, 0, 0);
			cairo_set_line_width(cr, AxisLineWidth * 2.0);
		} else {
			// Dashed lines for axis subdivisions
			double dashes[] = {5.0, 5.0};		// ink/skip
			cairo_set_dash(cr, dashes, sizeof(dashes)/sizeof(dashes[0]), 0.0);
			cairo_set_line_width(cr, AxisLineWidth);
		}

		cairo_move_to(cr, round(x) + axis_fudge, TMARGIN + axis_fudge);
		cairo_line_to(cr, round(x) + axis_fudge, TMARGIN + HEIGHT + axis_fudge);
		cairo_stroke(cr);

		X += XSTEP;
		if ((XAxisType == AXIS_LOG) && (X >= (XSTEP * LogBase))) {
			XSTEP *= LogBase;
		}
	}

	// Prepare to draw the chart
	cairo_set_source_rgba(cr, PlotColour.r, PlotColour.g, PlotColour.b, PlotColour.a);

	// If PlotLineWidth == 1.0, we need a fudge factor of 0.5 to stop Cairo AAing the 1px line into 2px 50%-alpha
	// TODO: RTFM the cairo docs, see if we need this for larger sizes
	float plot_fudge = (PlotLineWidth == 1.0) ? 0.5 : 0.0;

	// Set block size for scatter plots
	float BSZ = 10.0;
	float BSZH = BSZ / 2.0;

	// Set dashtype and line width for line charts
	if (PlotType == PLOT_LINE) {
		cairo_set_dash(cr, NULL, 0, 0);
		cairo_set_line_width(cr, PlotLineWidth);
	}

	for (size_t i=0; i<this->dataLength; i++) {
		// these two are here to make debugging easier. setting these to
		// various combinations of X/Y MIN/MAX draws lines on different
		// edges of the chart area, allowing the eqns to be calibrated.
		float x = (this->dataSrcX != NULL) ? ((float)this->dataSrcX[i]) : ((float)i);
		float y = ((float)this->dataSrcY[i]);

		// Calculate X/Y centre point for the scatter point
		x = (XAxisType == AXIS_LIN) ?
			LMARGIN + OuterBorderWidth + ((x - XMIN) * xd) :									// Linear X axis
			LMARGIN + OuterBorderWidth + (log1p(x - XMIN)/log1p(LogBase) * xd);					// Logarithmic X axis
		y = (YAxisType == AXIS_LIN) ?
			TMARGIN - (OuterBorderWidth/2.0) + (HEIGHT-((y - YMIN)*yd)) :						// Linear Y axis
			TMARGIN - (OuterBorderWidth/2.0) + (HEIGHT-((log1p(y - YMIN)/log1p(LogBase))*yd));	// Logarithmic Y axis

		if (PlotType == PLOT_LINE) {
			if (i == 0) {
				cairo_move_to(cr, x - (plot_fudge*2.0), y);
			}
			cairo_line_to(cr, x - (plot_fudge*2.0), y);
		} else if (PlotType == PLOT_SCATTER) {
			// For each block, call RECTANGLE then FILL. Calling fill() once right at
			// the end fubars the alpha calculations.

			// Calculate X and Y start point and size

			// First subtract half the block size to centre the block around
			// the plot point
			float xsta = x - BSZH;
			float xsz  = BSZ;
			float ysta = y - BSZH;
			float ysz  = BSZ;

			// Clip the data against the outer borders
			// This code took DAYS to perfect. Really, you don't want to
			// mess with it. If you mess with it, be prepared to provide a
			// mathematical or experimental proof as to why your method works
			// better.

			// Clip: left border
			if (xsta < (LMARGIN + (OuterBorderWidth/2.0))) {
				xsz -= (LMARGIN - xsta) + (OuterBorderWidth / 2.0);
				xsta = LMARGIN + (OuterBorderWidth / 2.0);
			}

			// Clip: top border
			if (ysta < (TMARGIN + (OuterBorderWidth / 2.0))) {
				ysz -= (TMARGIN - ysta) + (OuterBorderWidth / 2.0);
				ysta = TMARGIN + (OuterBorderWidth / 2.0);
			}

			// Clip: right border
			// Bear in mind we have TWO half-OuterBorderWidths to add here.
			// There's one on the left side, and one on the right. But when
			// we correct the Xsize value, we only need to correct for the
			// one on the left.
			if ((xsta+xsz) > (WIDTH + LMARGIN - OuterBorderWidth)) {
				xsz = ((WIDTH + LMARGIN) - xsta) - (OuterBorderWidth / 2.0);
			}

			// Clip: bottom border
			// Once again, two half-OuterBorderWidths to deal with. One on
			// the top edge, one on the bottom edge. Except here, we correct
			// the Ysize value, and we only correct for the left edge when
			// we do that.
			if ((ysta+ysz) > (HEIGHT + TMARGIN - (OuterBorderWidth/2.0))) {
				ysz = ((HEIGHT + TMARGIN) - ysta) - (OuterBorderWidth / 2.0);
			}

			// Draw a filled rectangle
			// Look, Ma! No fudge factor!
			cairo_rectangle(cr, xsta, ysta, xsz, ysz);	// syntax hint: cairo_obj, x, y, width, height
			cairo_fill(cr);
		}
	}

	// For line charts, we define the line first, then stroke (draw) it on the
	// canvas at the end. Scatter plots are built up on-the-fly using
	// rectangle() and fill() operations.
	if (PlotType == PLOT_LINE) {
		cairo_stroke(cr);
	}
}
