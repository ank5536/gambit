//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of window to draw correspondence plots
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
// This code is based loosely upon the plot windows in PXI, which was
// Copyright (c) Eugene Grayver.  The author has graciously permitted its
// use as a model for these classes.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include <math.h>
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "corplot.h"

//========================================================================
//                          General comments
//========================================================================

// This file implements some plotting classes.  These are in general
// specialized for Gambit's purposes, which is to plot strategy profiles
// generated by path-following algorithms.  Currently (and traditionally),
// this has been to plot quantal response equilibria; however, it could
// be profitably used to plot the progress of any path-following algorithm,
// including Yamamoto's algorithm.

// The implementation has two major influences:
// o PXI, by Eugene Grayver.  This was the plotting program Eugene wrote
//   for Richard McKelvey and Tom Palfrey to plot quantal response equilibria.
//   PXI was developed independently of Gambit, but was distributed
//   on the Gambit website.  Eugene has given permission for the Gambit
//   project to incorporate the PXI code into Gambit proper.  The
//   implementation here is in some ways significantly different than
//   the original PXI, but many features have been kept.
//
// o The wxPlotWindow class from wxWindows.  Unfortunately, wxPlotWindow
//   is not at this writing particularly useful for making the types of
//   plots that Gambit requires.  However, this implementation has been
//   modeled, at least in terms of class structure, to be somewhat similar
//   to wxPlotWindow, with the idea that, should a suitable set of plotting
//   classes ever become part of wxWindows, Gambit would be in good position
//   to take advantage of them.  (Or, perhaps, someday a version of these
//   classes would become part of wxWindows; who knows?)

// At present, these classes are somewhat rudimentary.  There are several
// areas in which abstraction could be improved, and additional user
// interaction and customization could be added.  These should be important
// goals in the future development of this code.

//========================================================================
//                  Implementation of gbtCorBranchMixed
//========================================================================

//------------------------------------------------------------------------
//                     gbtCorBranchMixed: Lifecycle
//------------------------------------------------------------------------

gbtCorBranchMixed::gbtCorBranchMixed(void)
{ }

gbtCorBranchMixed::gbtCorBranchMixed(const gList<MixedSolution> &p_data)
  : m_data(p_data)
{ }

//------------------------------------------------------------------------
//                    gbtCorBranchMixed: Data access
//------------------------------------------------------------------------

int gbtCorBranchMixed::NumDimensions(void) const
{ return m_data[1].Profile()->Length(); }

int gbtCorBranchMixed::NumData(void) const
{ return m_data.Length(); }

double gbtCorBranchMixed::GetValue(int p_index, int p_dim) const
{ return (*m_data[p_index].Profile())[p_dim]; }

double gbtCorBranchMixed::GetParameter(int p_index) const
{ return m_data[p_index].QreLambda(); }

double gbtCorBranchMixed::GetMaxParameter(void) const
{ return m_data[m_data.Length()].QreLambda(); }

double gbtCorBranchMixed::GetMinParameter(void) const
{ return 0.0; }

//========================================================================
//                  Implementation of gbtCorBranchBehav
//========================================================================

//------------------------------------------------------------------------
//                     gbtCorBranchBehav: Lifecycle
//------------------------------------------------------------------------

gbtCorBranchBehav::gbtCorBranchBehav(void)
{ }

gbtCorBranchBehav::gbtCorBranchBehav(const gList<BehavSolution> &p_data)
  : m_data(p_data)
{ }

//------------------------------------------------------------------------
//                    gbtCorBranchBehav: Data access
//------------------------------------------------------------------------

int gbtCorBranchBehav::NumDimensions(void) const
{ return m_data[1].Profile()->Length(); }

int gbtCorBranchBehav::NumData(void) const
{ return m_data.Length(); }

double gbtCorBranchBehav::GetValue(int p_index, int p_dim) const
{ return (*m_data[p_index].Profile())[p_dim]; }

double gbtCorBranchBehav::GetParameter(int p_index) const
{ return m_data[p_index].QreLambda(); }

double gbtCorBranchBehav::GetMaxParameter(void) const
{ return m_data[m_data.Length()].QreLambda(); }

double gbtCorBranchBehav::GetMinParameter(void) const
{ return 0.0; }


//========================================================================
//                 Implementation of gbtCorPlotXAxis
//========================================================================

gbtCorPlotXAxis::gbtCorPlotXAxis(void)
  : m_numDivisions(10), m_minValue(0.0), m_maxValue(1.0),
    m_labelFont(10, wxSWISS, wxNORMAL, wxBOLD), m_labelColor(*wxBLUE)
{ }

//========================================================================
//                 Implementation of gbtCorPlotYAxis
//========================================================================

gbtCorPlotYAxis::gbtCorPlotYAxis(void)
  : m_numDivisions(10), m_minValue(0.0), m_maxValue(1.0),
    m_labelFont(10, wxSWISS, wxNORMAL, wxBOLD), m_labelColor(*wxBLUE)
{ }

//========================================================================
//                 Implementation of gbtCorPlotWindow
//========================================================================

BEGIN_EVENT_TABLE(gbtCorPlotWindow, wxScrolledWindow)
  EVT_PAINT(gbtCorPlotWindow::OnPaint)
END_EVENT_TABLE()

//------------------------------------------------------------------------
//                   gbtCorPlotWindow: Lifecycle
//------------------------------------------------------------------------

gbtCorPlotWindow::gbtCorPlotWindow(wxWindow *p_parent, 
				   const wxPoint &p_position,
				   const wxSize &p_size)
  : wxScrolledWindow(p_parent, -1, p_position, p_size),
    m_marginX(50), m_marginY(65)
{
}

gbtCorPlotWindow::~gbtCorPlotWindow()
{ }

//------------------------------------------------------------------------
//                  gbtCorPlotWindow: Conversions
//------------------------------------------------------------------------

void gbtCorPlotWindow::DataToXY(double p_param, double p_value, 
				int &p_x, int &p_y) const
{
  int width, height;
  GetClientSize(&width, &height);
  height -= 2 * m_marginY;
  width -= 2 * m_marginX;
  p_y = (int) (m_marginY + (1.0 - p_value) * height);
  
  // This computes the location of p_param in terms of grid ticks
  double step = pow(m_xAxis.MaxValue() - m_xAxis.MinValue(),
		    1.0 / (double) m_xAxis.NumDivisions());
  double tickX = log(p_param - m_xAxis.MinValue() + 1.0) / log(step);
  p_x = (int) (m_marginX + tickX / (double) m_xAxis.NumDivisions() * width); 
}

//------------------------------------------------------------------------
//                    gbtCorPlotWindow: Drawing
//------------------------------------------------------------------------

void gbtCorPlotWindow::DrawXAxis(wxDC &p_dc)
{
  int width, height;
  GetClientSize(&width, &height);
  p_dc.DrawLine(m_marginX, height - m_marginY,
		width - m_marginX, height - m_marginY);
  int plotWidth = width - 2 * m_marginX;
  const int c_tickWidth = 5;
  double step = pow(m_xAxis.MaxValue() - m_xAxis.MinValue(),
		    1.0 / (double) m_xAxis.NumDivisions());
  double cumstep = step;

  p_dc.SetFont(m_xAxis.GetLabelFont());
  p_dc.SetTextForeground(m_xAxis.GetLabelColor());

  for (int i = 1; i <= m_xAxis.NumDivisions(); i++) {
    int xCoord = m_marginX + i * (plotWidth / m_xAxis.NumDivisions());
    p_dc.DrawLine(xCoord, height - m_marginY - c_tickWidth,
		  xCoord, height - m_marginY + c_tickWidth);
    double value = m_cor->GetMinParameter() + cumstep - 1.0;
    wxString label = wxString::Format((value < 10000.0) ? 
				      wxT("%6.2f") : wxT("%6.2e"), 
				      value);
    wxCoord textWidth, textHeight;
    p_dc.GetTextExtent(label, &textWidth, &textHeight);
    p_dc.DrawRotatedText(label,
			 xCoord - textHeight / 2,
			 height - m_marginY + 2 * c_tickWidth + textWidth, 90);
    cumstep *= step;
  }
}

void gbtCorPlotWindow::DrawYAxis(wxDC &p_dc)
{
  int width, height;
  GetClientSize(&width, &height);
  p_dc.DrawLine(m_marginX, m_marginY, m_marginX, height - m_marginY);
  int plotHeight = height - 2 * m_marginY;
  const int c_tickWidth = 5;

  p_dc.SetFont(m_xAxis.GetLabelFont());
  p_dc.SetTextForeground(m_xAxis.GetLabelColor());

  for (int i = 0; i <= m_yAxis.NumDivisions(); i++) {
    int yCoord = height - m_marginY - i * (plotHeight/m_yAxis.NumDivisions());
    p_dc.DrawLine(m_marginX - c_tickWidth, yCoord, 
		  m_marginX + c_tickWidth, yCoord);
    wxString label = wxString::Format(wxT("%3.2f"), 
				      (double) i /
				      (double) m_yAxis.NumDivisions());
    wxCoord textWidth, textHeight;
    p_dc.GetTextExtent(label, &textWidth, &textHeight);
    p_dc.DrawText(label, m_marginX - 2 * c_tickWidth - textWidth, 
		  yCoord - textHeight / 2);
  }
}

void gbtCorPlotWindow::DrawDimension(wxDC &p_dc, int p_dim)
{
  int lastX = -1, lastY = -1;
  for (int i = 1; i <= m_cor->NumData(); i++) {
    int x, y;
    DataToXY(m_cor->GetParameter(i), m_cor->GetValue(i, p_dim), x, y);
    if (lastX >= 0) {
      p_dc.DrawLine(lastX, lastY, x, y);
    }
    lastX = x;
    lastY = y;
  }
}

void gbtCorPlotWindow::OnPaint(wxPaintEvent &)
{
  wxPaintDC dc(this);

  dc.SetBackground(*wxWHITE_BRUSH);
  dc.Clear();
  DrawXAxis(dc);
  DrawYAxis(dc);

  int width, height;
  GetClientSize(&width, &height);

  for (int i = 1; i <= m_cor->NumDimensions(); i++) {
    static wxPen *pens[] = { wxRED_PEN, wxGREEN_PEN, wxCYAN_PEN,
			     wxLIGHT_GREY_PEN, wxBLACK_PEN };
    dc.SetPen(*pens[(i - 1) % 5]);

    dc.SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD));
    dc.SetTextForeground(*wxBLUE);
    wxCoord tw,th;
    dc.GetTextExtent(wxString::Format(wxT("%s"),
				      (char *) m_cor->GetLabel(i)), &tw, &th);
    dc.DrawLine(width - m_marginX - 50, 3*th*i/2+th/2,
		width - m_marginX - 40, 3*th*i/2+th/2);
    dc.DrawText(wxString::Format(wxT("%s"), (char *) m_cor->GetLabel(i)), 
		width - m_marginX - 35, 3*th*i/2);
    DrawDimension(dc, i);
  }
}

//------------------------------------------------------------------------
//                   gbtCorPlotWindow: Accessors
//------------------------------------------------------------------------

void gbtCorPlotWindow::SetCorrespondence(gbtCorBranch *p_cor)
{ 
  m_cor = p_cor; 
  m_xAxis.SetMaxValue(m_cor->GetMaxParameter());
}

gbtCorBranch *gbtCorPlotWindow::GetCorrespondence(void) const
{ return m_cor; }


