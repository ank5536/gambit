//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of tree layout representation
//

#include <math.h>
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "guishare/wxmisc.h"
#include "math/math.h"

#include "efg.h"

#include "treewin.h"
#include "efgshow.h"
#include "legend.h"

int SUBGAME_LARGE_ICON_SIZE = 20;
int SUBGAME_SMALL_ICON_SIZE = 10;
int DELTA = 8;
int MAX_TW = 60;
int MAX_TH = 20;

//-----------------------------------------------------------------------
//                    MISCELLANEOUS FUNCTIONS
//-----------------------------------------------------------------------

inline void DrawLine(wxDC &dc, double x_s, double y_s, double x_e, double y_e,
                     const wxColour &color, int thick = 0)
{
  dc.SetPen(*wxThePenList->FindOrCreatePen(color, (thick) ? 4 : 2, wxSOLID));
  dc.DrawLine((int) x_s, (int) y_s, (int) x_e, (int) y_e);
}

inline void DrawRectangle(wxDC &dc, int x_s, int y_s, int w, int h,
                          const wxColour &color)
{
  dc.SetPen(*wxThePenList->FindOrCreatePen(color, 2, wxSOLID));
  dc.DrawRectangle(x_s, y_s, w, h);
}

inline void DrawThinLine(wxDC &dc, int x_s, int y_s, int x_e, int y_e,
                         const wxColour &color)
{
  dc.SetPen(*wxThePenList->FindOrCreatePen(color, 1, wxSOLID));
  dc.DrawLine(x_s, y_s, x_e, y_e);
}

inline void DrawDashedLine(wxDC &dc, int x_s, int y_s, int x_e, int y_e,
			   const wxColour &color)
{
  dc.SetPen(*wxThePenList->FindOrCreatePen(color, 1, wxSHORT_DASH));
  dc.DrawLine(x_s, y_s, x_e, y_e);
}

inline void DrawCircle(wxDC &dc, int x, int y, int r, const wxColour &color)
{
  dc.SetPen(*wxThePenList->FindOrCreatePen(color, 3, wxSOLID));
  dc.DrawEllipse(x-r, y-r, 2*r, 2*r);
}


//-----------------------------------------------------------------------
//                   class NodeEntry: Member functions
//-----------------------------------------------------------------------

NodeEntry::NodeEntry(Node *p_node)
  : m_node(p_node), m_parent(0),
    m_x(-1), m_y(-1), m_nextMember(0), m_inSupport(true),
    m_selected(false), m_cursor(false),
    m_subgameRoot(false), m_subgameMarked(false), m_size(20),
    m_token(NODE_TOKEN_CIRCLE),
    m_branchStyle(BRANCH_STYLE_LINE), m_branchLabel(BRANCH_LABEL_HORIZONTAL),
    m_branchLength(0),
    m_sublevel(0), m_actionProb(0)
{ }

int NodeEntry::GetChildNumber(void) const
{
  if (m_node->GetParent()) {
    return m_node->GetAction()->GetNumber();
  }
  else {
    return 0;
  }
}

void NodeEntry::SetCursor(bool p_cursor)
{
  m_cursor = p_cursor;
  if (m_cursor) {
    m_selected = true;
  }
}

//
// Draws the node token itself, as well as the incoming branch
// (if not the root node)
//
void NodeEntry::Draw(wxDC &p_dc) const
{
  if (m_node->GetParent() && m_inSupport) {
    DrawIncomingBranch(p_dc);
  }

  p_dc.SetPen(*wxThePenList->FindOrCreatePen(m_color, (IsSelected()) ? 4 : 2,
					     wxSOLID));
  if (m_token == NODE_TOKEN_LINE) {
    p_dc.DrawLine(m_x, m_y, m_x + m_size, m_y);
    if (m_branchStyle == BRANCH_STYLE_FORKTINE) {
      // "classic" Gambit style: draw a small 'token' to separate
      // the fork from the node
      p_dc.DrawEllipse(m_x - 1, m_y - 1, 3, 3);
    }
  }
  else if (m_token == NODE_TOKEN_BOX) {
    p_dc.SetBrush(*wxWHITE_BRUSH);
    p_dc.DrawRectangle(m_x, m_y - m_size / 2, m_size, m_size);
  }
  else if (m_token == NODE_TOKEN_DIAMOND) {
    wxPoint points[4] = { wxPoint(m_x + m_size / 2, m_y - m_size / 2),
			  wxPoint(m_x, m_y),
			  wxPoint(m_x + m_size / 2, m_y + m_size / 2),
			  wxPoint(m_x + m_size, m_y) };
    p_dc.SetBrush(*wxWHITE_BRUSH);
    p_dc.DrawPolygon(4, points);
  }
  else {
    // Default: draw circles
    p_dc.SetBrush(*wxWHITE_BRUSH);
    p_dc.DrawEllipse(m_x, m_y - m_size / 2, m_size, m_size); 
  }

  int textWidth, textHeight;
  p_dc.GetTextExtent(m_nodeAboveLabel, &textWidth, &textHeight);
  p_dc.DrawText(m_nodeAboveLabel,
		m_x + (m_size - textWidth) / 2, m_y - textHeight - 9);
  p_dc.GetTextExtent(m_nodeBelowLabel, &textWidth, &textHeight);
  p_dc.DrawText(m_nodeBelowLabel,
		m_x + (m_size - textWidth) / 2, m_y + 9);
  p_dc.GetTextExtent(m_nodeRightLabel, &textWidth, &textHeight);
  p_dc.DrawText(m_nodeRightLabel,
		m_x + GetSize() + 10, m_y - textHeight/2);

  if (m_subgameRoot) {
    if (m_subgameMarked) {
      p_dc.SetPen(*wxThePenList->FindOrCreatePen(*wxBLACK, 2, wxSOLID));
    }
    else {
      p_dc.SetPen(*wxThePenList->FindOrCreatePen(*wxLIGHT_GREY, 2, wxSOLID));
    }
    p_dc.DrawLine(m_x - m_size / 2, m_y,
		  m_x + 2 * m_size, m_y + 2 * m_size);
    p_dc.DrawLine(m_x - m_size / 2, m_y,
		  m_x + 2 * m_size, m_y - 2 * m_size);
  }
}

void NodeEntry::DrawIncomingBranch(wxDC &p_dc) const
{
  int xStart = m_parent->m_x + m_parent->m_size;
  int xEnd = m_x;
  int yStart = m_parent->m_y;
  int yEnd = m_y;

  p_dc.SetPen(*wxThePenList->FindOrCreatePen(m_parent->m_color, 2, wxSOLID)); 
  if (m_branchStyle == BRANCH_STYLE_LINE) {
    p_dc.DrawLine(xStart, yStart, xEnd, yEnd);

    // Draw in the highlight indicating action probabilities
    if (m_actionProb >= gNumber(0)) {
      p_dc.SetPen(*wxThePenList->FindOrCreatePen(*wxBLACK, 2, wxSOLID));
      p_dc.DrawLine(xStart, yStart, 
		    xStart +
		    (int) ((double) (xEnd - xStart) * (double) m_actionProb),
		    yStart +
		    (int) ((double) (yEnd - yStart) * (double) m_actionProb));
    }

    int textWidth, textHeight;
    p_dc.SetFont(m_branchAboveFont);
    p_dc.GetTextExtent(m_branchAboveLabel, &textWidth, &textHeight);

    // The angle of the branch
    double theta = -atan((double) (yEnd - yStart) / (double) (xEnd - xStart));
    // The "centerpoint" of the branch
    int xbar = (xStart + xEnd) / 2;
    int ybar = (yStart + yEnd) / 2;

    if (m_branchLabel == BRANCH_LABEL_HORIZONTAL) {
      if (yStart >= yEnd) {
	p_dc.DrawText(m_branchAboveLabel, xbar - textWidth / 2, 
		      ybar - textHeight + 
		      textWidth / 2 * (yEnd - yStart) / (xEnd - xStart));
      }
      else {
	p_dc.DrawText(m_branchAboveLabel, xbar - textWidth / 2, 
		      ybar - textHeight - 
		      textWidth / 2 * (yEnd - yStart) / (xEnd - xStart));
      }
    }
    else {
      // Draw the text rotated appropriately
      p_dc.DrawRotatedText(m_branchAboveLabel,
			   (int) ((double) xbar -
				  (double) textHeight * sin(theta) -
				  (double) textWidth * cos(theta) / 2.0),
			   (int) ((double) ybar - 
				  (double) textHeight * cos(theta) +
				  (double) textWidth * sin(theta) / 2.0),
			   theta * 180.0 / 3.14159);
    }

    p_dc.SetFont(m_branchBelowFont);
    p_dc.GetTextExtent(m_branchBelowLabel, &textWidth, &textHeight);

    if (m_branchLabel == BRANCH_LABEL_HORIZONTAL) {
      if (yStart >= yEnd) {
	p_dc.DrawText(m_branchBelowLabel, xbar - textWidth / 2,
		      ybar - textWidth/2 * (yEnd - yStart) / (xEnd - xStart));
      }
      else {
	p_dc.DrawText(m_branchBelowLabel, xbar - textWidth / 2,
		      ybar + textWidth/2 * (yEnd - yStart) / (xEnd - xStart));
      }
    }
    else {
      // Draw the text rotated appropriately
      p_dc.DrawRotatedText(m_branchBelowLabel,
			   (int) ((double) xbar -
				  (double) textWidth * cos(theta) / 2.0),
			   (int) ((double) ybar +
				  (double) textWidth * sin(theta) / 2.0),
			   theta * 180.0 / 3.14159);
    }
  }
  else {
    // Old style fork-tine 
    p_dc.DrawLine(xStart, yStart, xStart + m_branchLength, yEnd);
    p_dc.DrawLine(xStart + m_branchLength, yEnd, xEnd, yEnd);
    
    // Draw in the highlight indicating action probabilities
    if (m_actionProb >= gNumber(0)) {
      p_dc.SetPen(*wxThePenList->FindOrCreatePen(*wxBLACK, 2, wxSOLID));
      p_dc.DrawLine(xStart, yStart, 
		    xStart + 
		    (int) ((double) m_branchLength * (double) m_actionProb),
		    yStart + 
		    (int) ((double) (yEnd - yStart) * (double) m_actionProb));
    }

    int textWidth, textHeight;
    p_dc.SetFont(m_branchAboveFont);
    p_dc.GetTextExtent(m_branchAboveLabel, &textWidth, &textHeight);
    p_dc.DrawText(m_branchAboveLabel,
		  xStart + m_branchLength + 3, yEnd - textHeight - 3);
    
    p_dc.SetFont(m_branchBelowFont);
    p_dc.GetTextExtent(m_branchBelowLabel, &textWidth, &textHeight);
    p_dc.DrawText(m_branchBelowLabel,
		  xStart + m_branchLength + 3, yEnd + 3);
  }
}

bool NodeEntry::NodeHitTest(int p_x, int p_y) const
{
  if (p_x < m_x || p_x >= m_x + m_size) {
    return false;
  }

  if (m_token == NODE_TOKEN_LINE) {
    const int DELTA = 8;  // a fudge factor for "almost" hitting the node
    return (p_y >= m_y - DELTA && p_y <= m_y + DELTA);
  }
  else {
    return (p_y >= m_y - m_size / 2 && p_y <= m_y + m_size / 2);
  }
}

//-----------------------------------------------------------------------
//                class efgTreeLayout: Member functions
//-----------------------------------------------------------------------

efgTreeLayout::efgTreeLayout(FullEfg &p_efg, TreeWindow *p_parent)
  : m_efg(p_efg), m_parent(p_parent), m_infosetSpacing(40),
    c_leftMargin(20), c_topMargin(40)
{ }

Node *efgTreeLayout::NodeHitTest(int p_x, int p_y) const
{
  for (int i = 1; i <= m_nodeList.Length(); i++) {
    if (m_nodeList[i]->NodeHitTest(p_x, p_y)) {
      return m_nodeList[i]->GetNode();
    }
  }
  return 0;
}

Node *efgTreeLayout::BranchHitTest(int p_x, int p_y) const
{
  for (int i = 1; i <= m_nodeList.Length(); i++) {
    NodeEntry *entry = m_nodeList[i];
    NodeEntry *parent_entry = GetNodeEntry(entry->GetNode()->GetParent());

    if (parent_entry) {
      if (p_x > (parent_entry->X() + m_parent->DrawSettings().NodeSize() + 
		 parent_entry->GetSublevel() * m_infosetSpacing + 10) &&
	  p_x < (parent_entry->X() + m_parent->DrawSettings().NodeSize() +
		 m_parent->DrawSettings().BranchLength() +
		 parent_entry->GetSublevel() * m_infosetSpacing)) {
	// Good old slope/intercept method for finding a point on a line
	int y0 = (parent_entry->Y() + 
		  (int) (p_x - parent_entry->X() - 
			 m_parent->DrawSettings().NodeSize()) *
		  (entry->Y() - parent_entry->Y()) / 
		  m_parent->DrawSettings().BranchLength());

	if (p_y > y0-2 && p_y < y0+2) {
	  return entry->GetNode();
	}
      }
    }
  }
  return 0;
}


Node *efgTreeLayout::InfosetHitTest(int p_x, int p_y) const
{
  for (int i = 1; i <= m_nodeList.Length(); i++) {
    NodeEntry *entry = m_nodeList[i];
    if (entry->GetNextMember() && entry->GetNode()->GetInfoset()) {
      if (p_x > entry->X() + entry->GetSublevel() * m_infosetSpacing - 2 &&
	  p_x < entry->X() + entry->GetSublevel() * m_infosetSpacing + 2) {
	if (p_y > entry->Y() && p_y < entry->GetNextMember()->Y()) {
	  // next iset is below this one
	  return entry->GetNode();
	}
	else if (p_y > entry->GetNextMember()->Y() && p_y < entry->Y()) {
	  // next iset is above this one
	  return entry->GetNode();
	}
      }
    }
  }
  return 0;
}

wxString efgTreeLayout::CreateNodeAboveLabel(const NodeEntry *p_entry) const
{
  const Node *n = p_entry->GetNode();
    
  switch (m_parent->DrawSettings().NodeAboveLabel()) {
  case NODE_ABOVE_NOTHING:
    return "";
  case NODE_ABOVE_LABEL:
    return (const char *) n->GetName();
  case NODE_ABOVE_PLAYER:
    return ((const char *) 
	    ((n->GetPlayer()) ? n->GetPlayer()->GetName() : gText("")));
  case NODE_ABOVE_ISETLABEL:
    return ((const char *)
	    ((n->GetInfoset()) ? n->GetInfoset()->GetName() : gText("")));
  case NODE_ABOVE_ISETID:
    return ((const char *)
	    ((n->GetInfoset()) ?
	     ("(" + ToText(n->GetPlayer()->GetNumber()) +
	      "," + ToText(n->GetInfoset()->GetNumber()) + ")") : gText("")));
  case NODE_ABOVE_OUTCOME:
    return (const char *) m_parent->OutcomeAsString(n);
  case NODE_ABOVE_REALIZPROB:
    return (const char *) m_parent->Parent()->GetRealizProb(n);
  case NODE_ABOVE_BELIEFPROB:
    return (const char *) m_parent->Parent()->GetBeliefProb(n);
  case NODE_ABOVE_VALUE:
    return (const char *) m_parent->Parent()->GetNodeValue(n);
  default:
    return "";
  }
}    

wxString efgTreeLayout::CreateNodeBelowLabel(const NodeEntry *p_entry) const
{
  const Node *n = p_entry->GetNode();

  switch (m_parent->DrawSettings().NodeBelowLabel()) { 
  case NODE_BELOW_NOTHING:
    return "";
  case NODE_BELOW_LABEL:
    return (const char *) n->GetName();
  case NODE_BELOW_PLAYER:
    return ((const char *)
	    ((n->GetPlayer()) ? n->GetPlayer()->GetName() : gText("")));
  case NODE_BELOW_ISETLABEL:
    return ((const char *)
	    ((n->GetInfoset()) ? n->GetInfoset()->GetName() : gText("")));
  case NODE_BELOW_ISETID:
    return ((const char *)
	    ((n->GetInfoset()) ?
	     ("(" + ToText(n->GetPlayer()->GetNumber()) +
	      "," + ToText(n->GetInfoset()->GetNumber()) + ")") : gText("")));
  case NODE_BELOW_OUTCOME:
    return (const char *) m_parent->OutcomeAsString(n);
  case NODE_BELOW_REALIZPROB:
    return (const char *) m_parent->Parent()->GetRealizProb(n);
  case NODE_BELOW_BELIEFPROB:
    return (const char *) m_parent->Parent()->GetBeliefProb(n);
  case NODE_BELOW_VALUE:
    return (const char *) m_parent->Parent()->GetNodeValue(n);
  default:
    return "";
  }
}

wxString efgTreeLayout::CreateNodeRightLabel(const NodeEntry *p_entry) const
{    
  const Node *node = p_entry->GetNode();

  switch (m_parent->DrawSettings().NodeRightLabel()) { 
  case NODE_RIGHT_NOTHING:
    return "";
  case NODE_RIGHT_OUTCOME:
    return (const char *) m_parent->OutcomeAsString(node);
  case NODE_RIGHT_NAME:
    if (!node->Game()->GetOutcome(node).IsNull()) {
      return (const char *) m_efg.GetOutcomeName(node->Game()->GetOutcome(node));
    }
    else {
      return "";
    }
  default:
    return "";
  }
}

wxString efgTreeLayout::CreateBranchAboveLabel(const NodeEntry *p_entry) const
{
  const Node *parent = p_entry->GetParent()->GetNode();

  switch (m_parent->DrawSettings().BranchAboveLabel()) {
  case BRANCH_ABOVE_NOTHING:
    return "";
  case BRANCH_ABOVE_LABEL:
    return (const char *) parent->GetInfoset()->GetActionName(p_entry->GetChildNumber());
  case BRANCH_ABOVE_PROBS:
    return (const char *) m_parent->Parent()->GetActionProb(parent,
							    p_entry->GetChildNumber());
  case BRANCH_ABOVE_VALUE:
    return (const char *) m_parent->Parent()->GetActionValue(parent,
							     p_entry->GetChildNumber());
  default:
    return "";
  }
}

wxString efgTreeLayout::CreateBranchBelowLabel(const NodeEntry *p_entry) const
{
  const Node *parent = p_entry->GetParent()->GetNode();

  switch (m_parent->DrawSettings().BranchBelowLabel()) {
  case BRANCH_BELOW_NOTHING:
    return "";
  case BRANCH_BELOW_LABEL:
    return (const char *) parent->GetInfoset()->GetActionName(p_entry->GetChildNumber());
  case BRANCH_BELOW_PROBS:
    return (const char *) m_parent->Parent()->GetActionProb(parent,
							    p_entry->GetChildNumber());
  case BRANCH_BELOW_VALUE:
    return (const char *) m_parent->Parent()->GetActionValue(parent,
							     p_entry->GetChildNumber());
  default:
    return "";
  }
}

NodeEntry *efgTreeLayout::GetValidParent(Node *e)
{
  NodeEntry *n = GetNodeEntry(e->GetParent());
  if (n) {
    return n;
  }
  else { 
    return GetValidParent(e->GetParent());
  }
}

NodeEntry *efgTreeLayout::GetValidChild(Node *e)
{
  for (int i = 1; i <= e->Game()->NumChildren(e); i++)  {
    NodeEntry *n = GetNodeEntry(e->GetChild(i));
    if (n) {
      return n;
    }
    else  {
      n = GetValidChild(e->GetChild(i));
      if (n) return n;
    }
  }
  return 0;
}

NodeEntry *efgTreeLayout::GetEntry(Node *p_node) const
{
  for (int i = 1; i <= m_nodeList.Length(); i++) {
    if (m_nodeList[i]->GetNode() == p_node) {
      return m_nodeList[i];
    }
  }
  return 0;
}

Node *efgTreeLayout::PriorSameLevel(Node *p_node) const
{
  NodeEntry *entry = GetEntry(p_node);
  if (entry) {
    for (int i = m_nodeList.Find(entry) - 1; i >= 1; i--) {
      if (m_nodeList[i]->GetLevel() == entry->GetLevel())
	return m_nodeList[i]->GetNode();
    }
  }
  return 0;
}

Node *efgTreeLayout::NextSameLevel(Node *p_node) const
{
  NodeEntry *entry = GetEntry(p_node);
  if (entry) {
    for (int i = m_nodeList.Find(entry) + 1; i <= m_nodeList.Length(); i++) {
      if (m_nodeList[i]->GetLevel() == entry->GetLevel()) { 
	return m_nodeList[i]->GetNode();
      }
    }
  }
  return 0;
}

int efgTreeLayout::LayoutSubtree(Node *p_node, const EFSupport &p_support,
				 int &p_maxy, int &p_miny, int &p_ycoord)
{
  int y1 = -1, yn = 0;
  const TreeDrawSettings &settings = m_parent->DrawSettings();
    
  NodeEntry *entry = m_nodeList[p_node->GetNumber()];
  entry->SetNextMember(0);
  if (p_node->NumChildren() > 0) {
    for (int i = 1; i <= p_node->NumChildren(); i++) {
      yn = LayoutSubtree(p_node->GetChild(i), p_support,
			 p_maxy, p_miny, p_ycoord);
      if (y1 == -1) {
	y1 = yn;
      }

      if (!p_node->GetPlayer()->IsChance() &&
	  !p_support.Find(p_node->GetInfoset()->Actions()[i])) {
	m_nodeList[p_node->GetChild(i)->GetNumber()]->SetInSupport(false);
      }
    }
    entry->SetY((y1 + yn) / 2);
  }
  else {
    entry->SetY(p_ycoord);
    p_ycoord += settings.TerminalSpacing();
  }
    
  if (settings.BranchStyle() == BRANCH_STYLE_LINE) {
    entry->SetX(c_leftMargin + entry->GetLevel() * (settings.NodeSize() +
						    settings.BranchLength()));
  }
  else {
    entry->SetX(c_leftMargin + entry->GetLevel() * (settings.NodeSize() +
						    settings.BranchLength() +
						    settings.TineLength()));
  }
  const UserPreferences &prefs = wxGetApp().GetPreferences();
  if (p_node->GetPlayer() && p_node->GetPlayer()->IsChance()) {
    entry->SetColor(prefs.GetChanceColor());
    entry->SetToken(settings.ChanceToken());
  }
  else if (p_node->GetPlayer()) {
    entry->SetColor(prefs.GetPlayerColor(p_node->GetPlayer()->GetNumber()));
    entry->SetToken(settings.PlayerToken());
  }
  else {
    entry->SetColor(prefs.GetTerminalColor());
    entry->SetToken(settings.TerminalToken());
  }  
  
  entry->SetSize(settings.NodeSize());
  entry->SetBranchStyle(settings.BranchStyle());
  if (settings.BranchStyle() == BRANCH_STYLE_LINE) {
    entry->SetBranchLabelStyle(settings.BranchLabels());
  }
  entry->SetBranchLength(settings.BranchLength());

  if (settings.SubgameStyle() == SUBGAME_ARC &&
      p_node->Game()->IsLegalSubgame(p_node)) {
    entry->SetSubgameRoot(true);
    entry->SetSubgameMarked(p_node->GetSubgameRoot() == p_node);
  }
  p_maxy = gmax(entry->Y(), p_maxy);
  p_miny = gmin(entry->Y(), p_miny);
    
  return entry->Y();
}

//
// Checks if there are any nodes in the same infoset as e that are either
// on the same level (if SHOWISET_SAME) or on any level (if SHOWISET_ALL)
//
NodeEntry *efgTreeLayout::NextInfoset(NodeEntry *e)
{
  const TreeDrawSettings &draw_settings = m_parent->DrawSettings();
  
  for (int pos = m_nodeList.Find(e) + 1; pos <= m_nodeList.Length(); pos++) {
    NodeEntry *e1 = m_nodeList[pos];
    // infosets are the same and the nodes are on the same level
    if (e->GetNode()->GetInfoset() == e1->GetNode()->GetInfoset()) {
      if (draw_settings.InfosetConnect() == INFOSET_CONNECT_ALL) {
	return e1;
      }
      else if (e->GetLevel() == e1->GetLevel()) {
	return e1;
      }
    }
  }
  return 0;
}

//
// CheckInfosetEntry.  Checks how many infoset lines are to be drawn at each
// level, spaces them by setting each infoset's node's num to the previous
// infoset node+1.  Also lengthens the nodes by the amount of space taken up
// by the infoset lines.
//
void efgTreeLayout::CheckInfosetEntry(NodeEntry *e)
{
  int pos;
  NodeEntry *infoset_entry, *e1;
  // Check if the infoset this entry belongs to (on this level) has already
  // been processed.  If so, make this entry->num the same as the one already
  // processed and return
  infoset_entry = NextInfoset(e);
  for (pos = 1; pos <= m_nodeList.Length(); pos++) {
    e1 = m_nodeList[pos];
    // if the infosets are the same and they are on the same level and e1 has been processed
    if (e->GetNode()->GetInfoset() == e1->GetNode()->GetInfoset() && 
	e->GetLevel() == e1->GetLevel() && e1->GetSublevel() > 0) {
      e->SetSublevel(e1->GetSublevel());
      if (infoset_entry) {
	e->SetNextMember(infoset_entry);
      }
      return;
    }
  }
    
  // If we got here, this entry does not belong to any processed infoset yet.
  // Check if it belongs to ANY infoset, if not just return
  if (!infoset_entry) return;
    
  // If we got here, then this entry is new and is connected to other entries
  // find the entry on the same level with the maximum num.
  // This entry will have num = num+1.
  int num = 0;
  for (pos = 1; pos <= m_nodeList.Length(); pos++) {
    e1 = m_nodeList[pos];
    // Find the max num for this level
    if (e->GetLevel() == e1->GetLevel())  {
      num = gmax(e1->GetSublevel(), num);
    }
  }
  num++;
  e->SetSublevel(num);
  e->SetNextMember(infoset_entry);
}

void efgTreeLayout::FillInfosetTable(Node *n, const EFSupport &cur_sup)
{
  const TreeDrawSettings &draw_settings = m_parent->DrawSettings();
  NodeEntry *entry = GetNodeEntry(n);
  if (n->Game()->NumChildren(n)>0) {
    for (int i = 1; i <= n->Game()->NumChildren(n); i++) {
      bool in_sup = true;
      if (n->GetPlayer()->GetNumber()) {
	in_sup = cur_sup.Find(n->GetInfoset()->Actions()[i]);
      }
            
      if (in_sup || !draw_settings.RootReachable()) {
	FillInfosetTable(n->GetChild(i), cur_sup);
      }
    }
  }

  if (entry) {
    CheckInfosetEntry(entry);
  }
}

void efgTreeLayout::UpdateTableInfosets(void)
{
  // Note that levels are numbered from 0, not 1.
  // create an array to hold max num for each level
  gArray<int> nums(0, m_maxLevel + 1); 
    
  for (int i = 0; i <= m_maxLevel + 1; nums[i++] = 0);
  // find the max e->num for each level
  for (int pos = 1; pos <= m_nodeList.Length(); pos++) {
    NodeEntry *entry = m_nodeList[pos];
    nums[entry->GetLevel()] = gmax(entry->GetSublevel() + 1,
				   nums[entry->GetLevel()]);
  }
    
  for (int i = 0; i <= m_maxLevel; i++) {
    nums[i+1] += nums[i];
  }
    
  // now add the needed length to each level, and set maxX accordingly
  m_maxX = 0;
  for (int pos = 1; pos <= m_nodeList.Length(); pos++) {
    NodeEntry *entry = m_nodeList[pos];
    if (entry->GetLevel() != 0) {
      entry->SetX(entry->X() + 
		  (nums[entry->GetLevel()-1] +
		   entry->GetSublevel()) * m_infosetSpacing);
    }
    m_maxX = gmax(m_maxX, entry->X());
  }
}

void efgTreeLayout::UpdateTableParents(void)
{
  for (int pos = 1; pos <= m_nodeList.Length(); pos++) {
    NodeEntry *e = m_nodeList[pos];
    e->SetParent((e->GetNode() == m_efg.RootNode()) ? 
		 e : GetValidParent(e->GetNode()));
  }
}

void efgTreeLayout::Layout(const EFSupport &p_support)
{
  // Kinda kludgey; probably should query draw settings whenever needed.
  m_infosetSpacing = 
    (m_parent->DrawSettings().InfosetJoin() == INFOSET_JOIN_LINES) ? 10 : 40;

  if (m_nodeList.Length() != NumNodes(m_efg)) {
    // A rebuild is in order; force it
    BuildNodeList(p_support);
  }

  int miny = 0, maxy = 0, ycoord = c_topMargin;
  LayoutSubtree(m_efg.RootNode(), p_support, maxy, miny, ycoord);

  const TreeDrawSettings &draw_settings = m_parent->DrawSettings();
  if (draw_settings.InfosetConnect() != INFOSET_CONNECT_NONE) {
    // FIXME! This causes lines to disappear... sometimes.
    FillInfosetTable(m_efg.RootNode(), p_support);
    UpdateTableInfosets();
  }

  UpdateTableParents();
  GenerateLabels();

  const int OUTCOME_LENGTH = 60;

  m_maxX += draw_settings.NodeSize() + OUTCOME_LENGTH;
  m_maxY = maxy + 25;
}

void efgTreeLayout::BuildNodeList(Node *p_node, const EFSupport &p_support,
				  int p_level)
{
  NodeEntry *entry = new NodeEntry(p_node);
  m_nodeList += entry;
  entry->SetLevel(p_level);
  if (p_node->NumChildren() > 0) {
    for (int i = 1; i <= p_node->NumChildren(); i++) {
      BuildNodeList(p_node->GetChild(i), p_support, p_level + 1);
    }
  }
  m_maxLevel = gmax(p_level, m_maxLevel);
}

void efgTreeLayout::BuildNodeList(const EFSupport &p_support)
{
  while (m_nodeList.Length() > 0) {
    delete m_nodeList.Remove(1);
  }

  m_maxLevel = 0;
  BuildNodeList(m_efg.RootNode(), p_support, 0);
}


void efgTreeLayout::GenerateLabels(void)
{
  const TreeDrawSettings &settings = m_parent->DrawSettings();
  for (int i = 1; i <= m_nodeList.Length(); i++) {
    NodeEntry *entry = m_nodeList[i];
    entry->SetNodeAboveLabel(CreateNodeAboveLabel(entry));
    entry->SetNodeBelowLabel(CreateNodeBelowLabel(entry));
    entry->SetNodeRightLabel(CreateNodeRightLabel(entry));
    if (entry->GetChildNumber() > 0) {
      entry->SetBranchAboveLabel(CreateBranchAboveLabel(entry));
      entry->SetBranchAboveFont(settings.BranchAboveFont());
      entry->SetBranchBelowLabel(CreateBranchBelowLabel(entry));
      entry->SetBranchBelowFont(settings.BranchBelowFont());
      entry->SetActionProb(m_parent->Parent()->ActionProb(entry->GetNode()->GetParent(),
							  entry->GetChildNumber()));
    }
  }
}

//
// RenderSubtree: Render branches and labels
//
// The following speed optimizations have been added:
// The algorithm now traverses the tree as a linear linked list, eliminating
// expensive searches.
//
// There was some clipping code in here, but it didn't correctly
// deal with drawing information sets while scrolling.  So, the code
// has temporarily been removed.  It remains to be seen whether
// performance will require a more sophisticated solution to the
// problem.  (TLT 5/2001)
//
void efgTreeLayout::RenderSubtree(wxDC &p_dc) const
{
  const TreeDrawSettings &settings = m_parent->DrawSettings();

  for (int pos = 1; pos <= m_nodeList.Length(); pos++) {
    NodeEntry *entry = m_nodeList[pos];  
    NodeEntry *parentEntry = entry->GetParent();
        
    if (entry->GetChildNumber() == 1) {
      parentEntry->Draw(p_dc);

      if (m_parent->DrawSettings().InfosetConnect() != INFOSET_CONNECT_NONE &&
	  parentEntry->GetNextMember()) {
	int nextX = parentEntry->GetNextMember()->X();
	int nextY = parentEntry->GetNextMember()->Y();

	if ((m_parent->DrawSettings().InfosetConnect() !=
	     INFOSET_CONNECT_SAMELEVEL) ||
	    parentEntry->X() == nextX) {
#ifdef __WXGTK__
	  // A problem with using styled pens and user scaling on wxGTK
	  p_dc.SetPen(wxPen(parentEntry->GetColor(), 1, wxSOLID));
#else
	  p_dc.SetPen(wxPen(parentEntry->GetColor(), 1, wxDOT));
#endif   // __WXGTK__
	  p_dc.DrawLine(parentEntry->X(), parentEntry->Y(), 
			parentEntry->X(), nextY);
	  if (settings.InfosetJoin() == INFOSET_JOIN_CIRCLES) {
	    p_dc.DrawLine(parentEntry->X() + parentEntry->GetSize(), 
			  parentEntry->Y(),
			  parentEntry->X() + parentEntry->GetSize(), 
			  nextY);
	  }

	  if (parentEntry->GetNextMember()->X() != parentEntry->X()) {
	    // Draw a little arrow in the direction of the iset.
	    if (settings.InfosetJoin() == INFOSET_JOIN_LINES) {
	      p_dc.DrawLine(parentEntry->X(), nextY, 
			    parentEntry->X() + m_infosetSpacing * 
			    ((parentEntry->GetNextMember()->X() > 
			      parentEntry->X()) ? 1 : -1),
			    nextY);

	    }
	    else {
	      if (parentEntry->GetNextMember()->X() < parentEntry->X()) {
		// information set is continued to the left
		p_dc.DrawLine(parentEntry->X() + parentEntry->GetSize(),
			      nextY,
			      parentEntry->X() - m_infosetSpacing,
			      nextY);
	      }
	      else {
		// information set is continued to the right
		p_dc.DrawLine(parentEntry->X(), nextY,
			      parentEntry->X() + parentEntry->GetSize() +
			      m_infosetSpacing, nextY);
	      }
	    }
	  }
	}
      }
    }

    if (entry->GetNode()->NumChildren() == 0) {
      entry->Draw(p_dc);
    }

  }
}

void efgTreeLayout::Render(wxDC &p_dc) const
{ 
  RenderSubtree(p_dc);
}

