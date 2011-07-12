#ifndef H_CHARTPANEL
#define H_CHARTPANEL

#include "wx/wx.h"
#include <cairo.h>

class ChartPanel : public wxPanel
{
	public:
		ChartPanel(wxFrame* parent);

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
			wxRect rect = GetClientRect();
			//if(IsExposed(rect.x, rect.y, rect.width, rect.height))
			{
				Refresh(false);
			}
			event.Skip();
		}

	private:
		/**
		 * @brief Render the chart on a Cairo surface
		 */
		void Render(cairo_t *cr, long width, long height);
};

#endif // H_CHARTPANEL
