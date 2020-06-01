#include <cmath>
#include <iostream>

#include "wx/wx.h"
#include "wx/dcgraph.h"

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

/**
 * Set formatting for the axis lines.
 *
 * @param	width		Width in pixels
 * @param	r			Red component, range 0 to 1
 * @param	g			Green component, range 0 to 1
 * @param	b			Blue component, range 0 to 1
 * @param	a			Transparency, range 0 to 1
 */
void ChartPanel::setAxisLineFormat(const float width, const float r, const float g, const float b, const float a)
{
	this->AxisLineWidth = width;
	this->AxisLineColour.r = r;
	this->AxisLineColour.g = g;
	this->AxisLineColour.b = b;
	this->AxisLineColour.a = a;
}

void ChartPanel::setBorderFormat(const float width, const float r, const float g, const float b)
{
	this->OuterBorderWidth = width;
	this->ChartBorderColour.r = r;
	this->ChartBorderColour.g = g;
	this->ChartBorderColour.b = b;
}

void ChartPanel::setPlotFormat(const float width, const float r, const float g, const float b, const float a)
{
	this->PlotLineWidth = width;
	this->PlotColour.r = r;
	this->PlotColour.g = g;
	this->PlotColour.b = b;
	this->PlotColour.a = a;
}

void ChartPanel::setBackgroundColour(const float r, const float g, const float b)
{
	this->ChartBackgroundColour.r = r;
	this->ChartBackgroundColour.g = g;
	this->ChartBackgroundColour.b = b;
}

void ChartPanel::setDataSource(const float *dataSrcX, const float *dataSrcY, const size_t dataLength)
{
	this->dataSrcX = dataSrcX;
	this->dataSrcY = dataSrcY;
	this->dataLength = dataLength;
}

void ChartPanel::setMargins(const long left, const long right, const long top, const long bottom)
{
	this->LMARGIN = left;
	this->RMARGIN = right;
	this->TMARGIN = top;
	this->BMARGIN = bottom;
}

void ChartPanel::setLogBase(const long logBase)
{
	this->LogBase = logBase;
}





ChartPanel::ChartPanel(wxFrame* parent) :
	wxPanel(parent),
	dataSrcX(NULL), dataSrcY(NULL), dataLength(0),
	LMARGIN(5), RMARGIN(5), TMARGIN(5), BMARGIN(5),
	LogBase(10),
	XAxisType(AXIS_LIN), YAxisType(AXIS_LIN),
	PlotType(PLOT_SCATTER),
	XMIN(0.0), XMAX(1.0),
	YMIN(0.0), YMAX(1.0)
{
	// Initialise colour parameters
	this->setBorderFormat(2, 0,0,0);				// 2px Black
	this->setBackgroundColour(1,1,1);				// White
	this->setAxisLineFormat(1, 0, 0, 0, 0.25);		// 1px grey (actually black), 25% opacity
	this->setPlotFormat(1, 0.0, 0.75, 0.00, 1.00);	// 1px green, 100% opacity

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
	wxPaintDC pdc(this);

	// Get client rect and do a sanity check
	wxRect rect = GetClientRect();

	if(rect.width == 0 || rect.height == 0)
	{
		return;
	}

#if defined(__WXGTK__)
	wxGCDC gdc;
	wxGraphicsRenderer * const renderer = wxGraphicsRenderer::GetCairoRenderer();
	wxGraphicsContext *context = renderer->CreateContext(pdc);
	gdc.SetGraphicsContext(context);

	cairo_t *cr = (cairo_t*)context->GetNativeContext();
	wxASSERT(cr != NULL);

	if(context == 0)
	{
		return;
	}


#elif defined(__WXOSX_COCOA__)
	CGContextRef context = (CGContextRef) static_cast<wxGCDCImpl *>(pdc.GetImpl())->GetGraphicsContext()->GetNativeContext();

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
	//cairo_destroy(cr);
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

	// Set clipping region
	cairo_save(cr);
	cairo_rectangle(cr,
			LMARGIN + (OuterBorderWidth/2.0),
			TMARGIN + (OuterBorderWidth/2.0),
			WIDTH - OuterBorderWidth,
			HEIGHT - OuterBorderWidth);
	cairo_clip(cr);

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
	// Start at the nearest whole grid unit to the Y minima
	float Y = (ceil(YMIN / YSTEP)) * YSTEP;

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
	// Start at the nearest whole grid unit to the X minima
	float X = (ceil(XMIN / XSTEP)) * XSTEP;

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
			LMARGIN + (OuterBorderWidth/2.0) + ((x - XMIN) * xd) :									// Linear X axis
			LMARGIN + (OuterBorderWidth/2.0) + (log1p(x - XMIN)/log1p(LogBase) * xd);					// Logarithmic X axis
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
			// Subtract half the block size to centre the block around the plot point
			float xsta = x - (PlotLineWidth / 2.0);
			float ysta = y - (PlotLineWidth / 2.0);

			// Draw a filled rectangle
			// Look, Ma! No fudge factor!
			cairo_rectangle(cr, xsta, ysta, PlotLineWidth, PlotLineWidth);
			cairo_fill(cr);
		}
	}

	// For line charts, we define the line first, then stroke (draw) it on the
	// canvas at the end. Scatter plots are built up on-the-fly using
	// rectangle() and fill() operations.
	if (PlotType == PLOT_LINE) {
		cairo_stroke(cr);
	}

	// Reset clipping region
	cairo_restore(cr);
}
