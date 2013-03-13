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
		/**
		 * @brief Render the chart on a Cairo surface
		 */
		void Render(cairo_t *cr, long width, long height);

		long LMARGIN, RMARGIN, TMARGIN, BMARGIN;
		long LogBase;
		int OuterBorderWidth;
		int AxisLineWidth, PlotLineWidth;					// PlotLineWidth only used when drawing line plots
		AXIS_TYPE XAxisType, YAxisType;
		PLOT_TYPE PlotType;
		COLOUR ChartBackgroundColour, ChartBorderColour;	// alpha channel ignored!
		COLOUR AxisLineColour, PlotColour;

		const float *dataSrcX, *dataSrcY;
		size_t dataLength;

	public:
		ChartPanel(wxFrame* parent);

		void setDataSource(const float *dataSrcX, const float *dataSrcY, const size_t dataLength)
		{
			this->dataSrcX = dataSrcX;
			this->dataSrcY = dataSrcY;
			this->dataLength = dataLength;
		}

		void setMargins(const long left, const long right, const long top, const long bottom)
		{
			this->LMARGIN = left;
			this->RMARGIN = right;
			this->TMARGIN = top;
			this->BMARGIN = bottom;
		}

		void setLogBase(const long logBase)
		{
			this->LogBase = logBase;
		}

		void setWidths(const int outerBorderWidth, const int axisLineWidth, const int plotLineWidth)
		{
			this->OuterBorderWidth = outerBorderWidth;
			this->AxisLineWidth = AxisLineWidth;
			this->PlotLineWidth = PlotLineWidth;
		}

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
