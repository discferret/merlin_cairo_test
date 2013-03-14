#ifndef H_CHARTPANEL
#define H_CHARTPANEL

#include "wx/wx.h"
#include <cairo.h>

typedef enum {
	AXIS_NONE,
	AXIS_LIN,
	AXIS_LOG
} AXIS_TYPE;

typedef enum {
	PLOT_LINE,
	PLOT_SCATTER
} PLOT_TYPE;

typedef struct {
	float r,g,b,a;
} COLOUR;

class ChartPanel : public wxPanel
{
	private:
		// Chart data source, X and Y, and data length
		const float *dataSrcX, *dataSrcY;
		size_t dataLength;

		// Left, right, top and bottom margins
		long LMARGIN, RMARGIN, TMARGIN, BMARGIN;

		// Logarithm base for logarithmic charts; defaults to 10
		long LogBase;

		// Width of outer border in pixels
		int OuterBorderWidth;

		// Width of axis lines in pixels
		int AxisLineWidth;

		// Width of plot lines in pixels. For scatter plots, this is the size of the scatter block.
		int PlotLineWidth;

		// Axis line and plot colour
		COLOUR AxisLineColour, PlotColour;

		// Chart background and border colour -- alpha channel is ignored!
		COLOUR ChartBackgroundColour, ChartBorderColour;

		/**
		 * @brief Render the chart on a Cairo surface
		 */
		void Render(cairo_t *cr, long width, long height);

	public:
		ChartPanel(wxFrame* parent);

		AXIS_TYPE XAxisType, YAxisType;
		PLOT_TYPE PlotType;

		float XMIN, XMAX, YMIN, YMAX;						// X/Y axis scaling (minimum and maximum value)
		float XSTEP, YSTEP;									// X/Y axis major gridline step

		void setAxisLineFormat(const float width, const float r, const float g, const float b, const float a);
		void setBorderFormat(const float width, const float r, const float g, const float b);
		void setPlotFormat(const float width, const float r, const float g, const float b, const float a);
		void setBackgroundColour(const float r, const float g, const float b);
		void setDataSource(const float *dataSrcX, const float *dataSrcY, const size_t dataLength);
		void setMargins(const long left, const long right, const long top, const long bottom);
		void setLogBase(const long logBase);

		void autoScale(void);

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

		/**
		 * @brief wxWidgets paint event handler
		 *
		 * Handles repaint events for the chart control. Do not call this from user
		 * code unless you REALLY know what you're doing!
		 */
		void OnPaint(wxPaintEvent &WXUNUSED(event));

		/**
		 * @brief wxWidgets resize event handler
		 *
		 * Handles resize events for the chart control. Do not call this from user
		 * code unless you REALLY know what you're doing!
		 */
		void OnSize(wxSizeEvent& event)
		{
			// making refresh conditional on IsExposed breaks refresh when the window is resized
			//wxRect rect = GetClientRect();
			//if(IsExposed(rect.x, rect.y, rect.width, rect.height))
			{
				Refresh(false);
			}
			event.Skip();
		}
};

#endif // H_CHARTPANEL
