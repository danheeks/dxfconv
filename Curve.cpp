// Curve.cpp
// Copyright 2011, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "Curve.h"
#include "Circle.h"
#include "Arc.h"
#include "Area.h"
#include "geometry.h"

const Point operator*(const double &d, const Point &p){ return p * d;}

//static const double PI = 3.1415926535897932; duplicated in kurve/geometry.h

double Point::length()const
{
    return sqrt( x*x + y*y );
}

double Point::magnitude()const
{
	return sqrt(x*x + y*y);
}

double Point::magnitudesqd()const
{
	return x*x + y*y;
}

double Point::normalize()
{
	double len = length();
	if(fabs(len)> 0.000000000000001)
		*this = (*this) / len;
	return len;
}

Point Point::Mid(const Point& p1, double factor)const{
	// Mid
	return ::Mid(*this, p1, factor);
}

ostream & operator<<(ostream &os, const Point &p)
{
	return os << "Point x = " << p.x << ", y = " << p.y;
}

Line2d::Line2d(const Point& P0, const Point& V) :p0(P0), v(V)
{
}

double Line2d::Dist(const Point& p)const
{
	Point vn = v;
	vn.normalize();
	double d1 = p0 * vn;
	double d2 = p * vn;
	Point pn = p0 + vn * (d2 - d1);

	return pn.dist(p);
}

CVertex::CVertex(int type, const Point& p, const Point& c, int user_data):m_type(type), m_p(p), m_c(c), m_user_data(user_data)
{
}

CVertex::CVertex(const Point& p, int user_data):m_type(0), m_p(p), m_c(0.0, 0.0), m_user_data(user_data)
{
}

void CVertex::Transform(const Matrix& matrix)
{
	m_p.Transform(matrix);
	m_c.Transform(matrix);
}

ostream & operator<<(ostream &os, const CVertex &v)
{
	os << "Vertex type = " << v.m_type << ", p = " << v.m_p;
	if (v.m_type != 0)
	{
		os << ", c = " << v.m_c;
	}
	return os;
}

void CCurve::append(const CVertex& vertex)
{
	m_vertices.push_back(vertex);
}


class CArcOrLine
{
public:
	CArc m_arc;
	bool m_is_a_line;

	CArcOrLine(const CArc& arc, bool is_a_line) :m_arc(arc), m_is_a_line(is_a_line) {}
};

bool CCurve::CheckForArc(const CVertex& prev_vt, std::list<const CVertex*>& might_be_an_arc, CArcOrLine &arc_or_line_returned)
{
	// this examines the vertices in might_be_an_arc
	// if they do fit an arc, set arc to be the arc that they fit and return true
	// returns true, if arc added
	if(might_be_an_arc.size() < 2)return false;

	// find middle point
	unsigned int num = (unsigned int)might_be_an_arc.size();
	int i = 0;
	const CVertex* mid_vt = NULL;
	int mid_i = (num-1)/2;
	for(std::list<const CVertex*>::iterator It = might_be_an_arc.begin(); It != might_be_an_arc.end(); It++, i++)
	{
		if(i == mid_i)
		{
			mid_vt = *It;
			break;
		}
	}

	// create a circle to test
	Point p0(prev_vt.m_p);
	Point p1(mid_vt->m_p);
	Point p2(might_be_an_arc.back()->m_p);
	CircleOrLine c(p0, p1, p2);

	const CVertex* current_vt = &prev_vt;
	double accuracy = CArea::m_accuracy * 1.4 / CArea::m_units;
	for(std::list<const CVertex*>::iterator It = might_be_an_arc.begin(); It != might_be_an_arc.end(); It++)
	{
		const CVertex* vt = *It;

		if(!c.LineIsOn(current_vt->m_p, vt->m_p, accuracy))
			return false;
		current_vt = vt;
	}

	CArc arc;
	arc.m_s = prev_vt.m_p;
	arc.m_e = might_be_an_arc.back()->m_p;
	if (c.m_is_a_line)
	{
		arc_or_line_returned = CArcOrLine(arc, true);
		return true;
	}
	arc.m_c = c.m_c;
	arc.SetDirWithPoint(might_be_an_arc.front()->m_p);
	arc.m_user_data = might_be_an_arc.back()->m_user_data;

	double angs = atan2(arc.m_s.y - arc.m_c.y, arc.m_s.x - arc.m_c.x);
	double ange = atan2(arc.m_e.y - arc.m_c.y, arc.m_e.x - arc.m_c.x);
	if(arc.m_dir)
	{
		// make sure ange > angs
		if(ange < angs)ange += 6.2831853071795864;
	}
	else
	{
		// make sure angs > ange
		if(angs < ange)angs += 6.2831853071795864;
	}

	if(arc.IncludedAngle() >= 3.15)return false; // We don't want full arcs, so limit to about 180 degrees

	for(std::list<const CVertex*>::iterator It = might_be_an_arc.begin(); It != might_be_an_arc.end(); It++)
	{
		const CVertex* vt = *It;
		double angp = atan2(vt->m_p.y - arc.m_c.y, vt->m_p.x - arc.m_c.x);
		if(arc.m_dir)
		{
			// make sure angp > angs
			if(angp < angs)angp += 6.2831853071795864;
			if(angp > ange)return false;
		}
		else
		{
			// make sure angp > ange
			if(angp < ange)angp += 6.2831853071795864;
			if(angp > angs)return false;
		}
	}

	arc_or_line_returned = CArcOrLine(arc, false);
	return true;
}

bool CheckAddedRadii(const std::list<CVertex> &new_vertices)
{
	if (new_vertices.size() > 1)
	{
		std::list<CVertex>::const_iterator It = new_vertices.end();
		It--;
		const CVertex& v = *It;
		if (v.m_type != 0)
		{
			It--;
			const CVertex& p = *It;
			double r1 = p.m_p.dist(v.m_c);
			double r2 = v.m_p.dist(v.m_c);
			if (fabs(r1 - r2) > 0.0001)
			{
				return false;
			}
		}
	}

	return true;
}

void CCurve::AddArcOrLines(bool check_for_arc, std::list<CVertex> &new_vertices, std::list<const CVertex*>& might_be_an_arc, CArcOrLine &arc_or_line, bool &arc_found, bool &arc_added)
{
	if (check_for_arc && CheckForArc(new_vertices.back(), might_be_an_arc, arc_or_line))
	{
		arc_found = true;
	}
	else
	{
		if (arc_found)
		{
			if (arc_or_line.m_is_a_line || arc_or_line.m_arc.AlmostALine(CArea::m_accuracy))
			{
				new_vertices.push_back(CVertex(arc_or_line.m_arc.m_e, arc_or_line.m_arc.m_user_data));
			}
			else
			{
				new_vertices.push_back(CVertex(arc_or_line.m_arc.m_dir ? 1 : -1, arc_or_line.m_arc.m_e, arc_or_line.m_arc.m_c, arc_or_line.m_arc.m_user_data));
				CheckAddedRadii(new_vertices);
			}

			arc_added = true;
			arc_found = false;
			const CVertex* back_vt = might_be_an_arc.back();
			might_be_an_arc.clear();
			if (check_for_arc)might_be_an_arc.push_back(back_vt);
		}
		else
		{
			const CVertex* back_vt = might_be_an_arc.back();
			if (check_for_arc)might_be_an_arc.pop_back();
			for (std::list<const CVertex*>::iterator It = might_be_an_arc.begin(); It != might_be_an_arc.end(); It++)
			{
				const CVertex* v = *It;
				if (It != might_be_an_arc.begin() || (new_vertices.size() == 0) || (new_vertices.back().m_p != v->m_p))
				{
					new_vertices.push_back(*v);
					CheckAddedRadii(new_vertices);
				}
			}
			might_be_an_arc.clear();
			if (check_for_arc)might_be_an_arc.push_back(back_vt);
		}
	}
}

void CCurve::FitArcs()
{
	std::list<CVertex> new_vertices;

	std::list<const CVertex*> might_be_an_arc;
	CArcOrLine arc_or_line(CArc(), false);
	bool arc_found = false;
	bool arc_added = false;

	int i = 0;
	const CVertex* prev_vt = NULL;
	for (std::list<CVertex>::iterator It = m_vertices.begin(); It != m_vertices.end(); It++, i++)
	{
		CVertex& vt = *It;
		if (vt.m_type || i == 0)
		{
			if (i != 0)
			{
				AddArcOrLines(false, new_vertices, might_be_an_arc, arc_or_line, arc_found, arc_added);
			}
			new_vertices.push_back(vt);
		}
		else
		{
			if (vt.m_p != prev_vt->m_p)
			{
				might_be_an_arc.push_back(&vt);

				if (might_be_an_arc.size() == 1)
				{
				}
				else
				{
					AddArcOrLines(true, new_vertices, might_be_an_arc, arc_or_line, arc_found, arc_added);
				}
			}
		}
		prev_vt = &vt;
	}

	if (might_be_an_arc.size() > 0)AddArcOrLines(false, new_vertices, might_be_an_arc, arc_or_line, arc_found, arc_added);

	if (arc_added)
	{
		m_vertices.clear();
		for (std::list<CVertex>::iterator It = new_vertices.begin(); It != new_vertices.end(); It++)m_vertices.push_back(*It);
		for (std::list<const CVertex*>::iterator It = might_be_an_arc.begin(); It != might_be_an_arc.end(); It++)m_vertices.push_back(*(*It));
	}
}

void CCurve::UnFitArcs()
{
	std::list<Point> new_pts;

	const CVertex* prev_vertex = NULL;
	for(std::list<CVertex>::const_iterator It2 = m_vertices.begin(); It2 != m_vertices.end(); It2++)
	{
		const CVertex& vertex = *It2;
		if(vertex.m_type == 0 || prev_vertex == NULL)
		{
			new_pts.push_back(vertex.m_p * CArea::m_units);
		}
		else
		{
			if(vertex.m_p != prev_vertex->m_p)
			{
				double phi,dphi,dx,dy;
				int Segments;
				int i;
				double ang1,ang2,phit;

				dx = (prev_vertex->m_p.x - vertex.m_c.x) * CArea::m_units;
				dy = (prev_vertex->m_p.y - vertex.m_c.y) * CArea::m_units;

				ang1=atan2(dy,dx);
				if (ang1<0) ang1+=2.0*PI;
				dx = (vertex.m_p.x - vertex.m_c.x) * CArea::m_units;
				dy = (vertex.m_p.y - vertex.m_c.y) * CArea::m_units;
				ang2=atan2(dy,dx);
				if (ang2<0) ang2+=2.0*PI;

				if (vertex.m_type == -1)
				{ //clockwise
					if (ang2 > ang1)
						phit=2.0*PI-ang2+ ang1;
					else
						phit=ang1-ang2;
				}
				else
				{ //counter_clockwise
					if (ang1 > ang2)
						phit=-(2.0*PI-ang1+ ang2);
					else
						phit=-(ang2-ang1);
				}

				//what is the delta phi to get an accurancy of aber
				double radius = sqrt(dx*dx + dy*dy);
				dphi=2*acos((radius-CArea::m_accuracy)/radius);

				//set the number of segments
				if (phit > 0)
					Segments=(int)ceil(phit/dphi);
				else
					Segments=(int)ceil(-phit/dphi);

				if (Segments < 1)
					Segments=1;
				if (Segments > 100)
					Segments=100;

				dphi=phit/(Segments);

				double px = prev_vertex->m_p.x * CArea::m_units;
				double py = prev_vertex->m_p.y * CArea::m_units;

				for (i=1; i<=Segments; i++)
				{
					dx = px - vertex.m_c.x * CArea::m_units;
					dy = py - vertex.m_c.y * CArea::m_units;
					phi=atan2(dy,dx);

					double nx = vertex.m_c.x * CArea::m_units + radius * cos(phi-dphi);
					double ny = vertex.m_c.y * CArea::m_units + radius * sin(phi-dphi);

					new_pts.push_back(Point(nx, ny));

					px = nx;
					py = ny;
				}
			}
		}
		prev_vertex = &vertex;
	}

	m_vertices.clear();

	for(std::list<Point>::iterator It = new_pts.begin(); It != new_pts.end(); It++)
	{
		Point &pt = *It;
		CVertex vertex(0, pt / CArea::m_units, Point(0.0, 0.0));
		m_vertices.push_back(vertex);
	}
}

Point CCurve::NearestPoint(const Point& p)const
{
	double best_dist = 0.0;
	Point best_point = Point(0, 0);
	bool best_point_valid = false;
	Point prev_p = Point(0, 0);
	bool prev_p_valid = false;
	bool first_span = true;
	for(std::list<CVertex>::const_iterator It = m_vertices.begin(); It != m_vertices.end(); It++)
	{
		const CVertex& vertex = *It;
		if(prev_p_valid)
		{
			Point near_point = Span(prev_p, vertex, first_span).NearestPoint(p);
			first_span = false;
			double dist = near_point.dist(p);
			if(!best_point_valid || dist < best_dist)
			{
				best_dist = dist;
				best_point = near_point;
				best_point_valid = true;
			}
		}
		prev_p = vertex.m_p;
		prev_p_valid = true;
	}
	return best_point;
}

Point CCurve::NearestPoint(const CCurve& c, double *d)const
{
	double best_dist = 0.0;
	Point best_point = Point(0, 0);
	bool best_point_valid = false;
	Point prev_p = Point(0, 0);
	bool prev_p_valid = false;
	bool first_span = true;
	for(std::list<CVertex>::const_iterator It = c.m_vertices.begin(); It != c.m_vertices.end(); It++)
	{
		const CVertex& vertex = *It;
		if(prev_p_valid)
		{
			double dist;
			Point near_point = NearestPoint(Span(prev_p, vertex, first_span), &dist);
			first_span = false;
			if(!best_point_valid || dist < best_dist)
			{
				best_dist = dist;
				best_point = near_point;
				best_point_valid = true;
			}
		}
		prev_p = vertex.m_p;
		prev_p_valid = true;
	}
	if(d)*d = best_dist;
	return best_point;
}

void CCurve::GetBox(CBox2D &box)const
{
	Point prev_p = Point(0, 0);
	bool prev_p_valid = false;
	for(std::list<CVertex>::const_iterator It = m_vertices.begin(); It != m_vertices.end(); It++)
	{
		const CVertex& vertex = *It;
		if(prev_p_valid)
		{
			Span(prev_p, vertex).GetBox(box);
		}
		prev_p = vertex.m_p;
		prev_p_valid = true;
	}
}

void CCurve::Reverse()
{
	std::list<CVertex> new_vertices;

	CVertex* prev_v = NULL;

	for(std::list<CVertex>::reverse_iterator It = m_vertices.rbegin(); It != m_vertices.rend(); It++)
	{
		CVertex &v = *It;
		int type = 0;
		Point cp(0.0, 0.0);
		if(prev_v)
		{
			type = -prev_v->m_type;
			cp = prev_v->m_c;
		}
		CVertex new_v(type, v.m_p, cp);
		new_vertices.push_back(new_v);
		prev_v = &v;
	}

	m_vertices = new_vertices;
}

double CCurve::GetArea()const
{
	double area = 0.0;
	Point prev_p = Point(0, 0);
	bool prev_p_valid = false;
	for(std::list<CVertex>::const_iterator It = m_vertices.begin(); It != m_vertices.end(); It++)
	{
		const CVertex& vertex = *It;
		if(prev_p_valid)
		{
			area += Span(prev_p, vertex).GetArea();
		}
		prev_p = vertex.m_p;
		prev_p_valid = true;
	}
	return area;
}

bool CCurve::IsClosed()const
{
	if(m_vertices.size() == 0)return false;
	return m_vertices.front().m_p == m_vertices.back().m_p;
}

void CCurve::ChangeStart(const Point &p) {
	CCurve new_curve;

	bool started = false;
	bool finished = false;
	int start_span;
	bool closed = IsClosed();

	for(int i = 0; i < (closed ? 2:1); i++)
	{
		const Point *prev_p = NULL;

		int span_index = 0;
		for(std::list<CVertex>::const_iterator VIt = m_vertices.begin(); VIt != m_vertices.end() && !finished; VIt++)
		{
			const CVertex& vertex = *VIt;

			if(prev_p)
			{
				Span span(*prev_p, vertex);
				if(span.On(p))
				{
					if(started)
					{
						if(p == *prev_p || span_index != start_span)
						{
							new_curve.m_vertices.push_back(vertex);
						}
						else
						{
							if(p == vertex.m_p)new_curve.m_vertices.push_back(vertex);
							else
							{
								CVertex v(vertex);
								v.m_p = p;
								new_curve.m_vertices.push_back(v);
							}
							finished = true;
						}
					}
					else
					{
						new_curve.m_vertices.push_back(CVertex(p));
						started = true;
						start_span = span_index;
						if(p != vertex.m_p)new_curve.m_vertices.push_back(vertex);
					}
				}
				else
				{
					if(started)
					{
						new_curve.m_vertices.push_back(vertex);
					}
				}
				span_index++;
			}
			prev_p = &(vertex.m_p);
		}
	}

	if(started)
	{
		*this = new_curve;
	}
}

void CCurve::Break(const Point &p) {
	// inserts a point, if it lies on the curve
	const Point *prev_p = NULL;

	for(std::list<CVertex>::iterator VIt = m_vertices.begin(); VIt != m_vertices.end(); VIt++)
	{
		CVertex& vertex = *VIt;

		if(p == vertex.m_p)break; // point is already on a vertex

		if(prev_p)
		{
			Span span(*prev_p, vertex);
			if(span.On(p))
			{
				CVertex v(vertex);
				v.m_p = p;
				m_vertices.insert(VIt, v);
				break;
			}
		}
		prev_p = &(vertex.m_p);
	}
}

void CCurve::ExtractSeparateCurves(const std::list<Point> &ordered_points, std::list<CCurve> &separate_curves)const
{
	// returns separate curves for this curve split at points
	// the points must be in order along this curve, already, and lie on this curve
	const Point *prev_p = NULL;

	if(ordered_points.size() == 0)
	{
		separate_curves.push_back(*this);
		return;
	}

	CCurve current_curve;

	std::list<Point>::const_iterator PIt = ordered_points.begin();
	Point point = *PIt;

	for(std::list<CVertex>::const_iterator VIt = m_vertices.begin(); VIt != m_vertices.end(); VIt++)
	{
		const CVertex& vertex = *VIt;
		if(prev_p)// not the first vertex
		{
			Span span(*prev_p, vertex);
			while((PIt != ordered_points.end()) && span.On(point))
			{
				CVertex v(vertex);
				v.m_p = point;
				current_curve.m_vertices.push_back(v);
				if(current_curve.m_vertices.size() > 1)// don't add single point curves
					separate_curves.push_back(current_curve); // add the curve
				current_curve = CCurve();// make a new curve
				current_curve.m_vertices.push_back(v); // add it's first point
				PIt++;
				if(PIt != ordered_points.end())point = *PIt; // increment the point
			}

			// add the end of span
			if(current_curve.m_vertices.back().m_p != vertex.m_p)
				current_curve.m_vertices.push_back(vertex);
		}
		if((current_curve.m_vertices.size() == 0) || (current_curve.m_vertices.back().m_p != vertex.m_p))
		{
			// very first vertex, start the current curve
			current_curve.m_vertices.push_back(vertex);
		}
		prev_p = &(vertex.m_p);
	}

	// add whatever is left
	if(current_curve.m_vertices.size() > 1)// don't add single point curves
		separate_curves.push_back(current_curve); // add the curve
}



void CCurve::RemoveTinySpans() {
	CCurve new_curve;

	std::list<CVertex>::const_iterator VIt = m_vertices.begin(); 
	new_curve.m_vertices.push_back(*VIt);
	VIt++;

	for(; VIt != m_vertices.end(); VIt++)
	{
		const CVertex& vertex = *VIt;

		if(vertex.m_type != 0 || new_curve.m_vertices.back().m_p.dist(vertex.m_p) > TOLERANCE)
		{
			new_curve.m_vertices.push_back(vertex);
		}
	}
	*this = new_curve;
}

void CCurve::ChangeEnd(const Point &p) {
	// changes the end position of the Kurve, doesn't keep closed kurves closed
	CCurve new_curve;

	const Point *prev_p = NULL;

	for(std::list<CVertex>::const_iterator VIt = m_vertices.begin(); VIt != m_vertices.end(); VIt++)
	{
		const CVertex& vertex = *VIt;

		if(prev_p)
		{
			Span span(*prev_p, vertex);
			if(span.On(p))
			{
				CVertex v(vertex);
				v.m_p = p;
				new_curve.m_vertices.push_back(v);
				break;
			}
			else
			{
				if(p != vertex.m_p)new_curve.m_vertices.push_back(vertex);
			}
		}
		else
		{
			new_curve.m_vertices.push_back(vertex);
		}
		prev_p = &(vertex.m_p);
	}

	*this = new_curve;
}

Point CCurve::NearestPoint(const Span& p, double *d)const
{
	double best_dist = 0.0;
	Point best_point = Point(0, 0);
	bool best_point_valid = false;
	Point prev_p = Point(0, 0);
	bool prev_p_valid = false;
	bool first_span = true;
	for(std::list<CVertex>::const_iterator It = m_vertices.begin(); It != m_vertices.end(); It++)
	{
		const CVertex& vertex = *It;
		if(prev_p_valid)
		{
			double dist;
			Point near_point = Span(prev_p, vertex, first_span).NearestPoint(p, &dist);
			first_span = false;
			if(!best_point_valid || dist < best_dist)
			{
				best_dist = dist;
				best_point = near_point;
				best_point_valid = true;
			}
		}
		prev_p = vertex.m_p;
		prev_p_valid = true;
	}
	if(d)*d = best_dist;
	return best_point;
}

bool CCurve::Offset(double leftwards_value)
{
	// use the kurve code donated by Geoff Hawkesford, to offset the curve as an open curve
	// returns true for success, false for failure
	bool success = true;

	CCurve save_curve = *this;

	try
	{
		CCurve kOffset;
		int ret = 0;
		OffsetMethod1(kOffset, fabs(leftwards_value), (leftwards_value > 0) ? 1:-1, ret);
		success = (ret == 0);
		if(success)*this = kOffset;
	}
	catch(...)
	{
		success = false;
	}

	if(success == false)
	{
		if(this->IsClosed())
		{
			double inwards_offset = leftwards_value;
			bool cw = false;
			if(this->IsClockwise())
			{
				inwards_offset = -inwards_offset;
				cw = true;
			}
			CArea a;
			a.append(*this);
			a.Offset(inwards_offset);
			if(a.m_curves.size() == 1)
			{
				Span* start_span = NULL;
				if(this->m_vertices.size() > 1)
				{
					std::list<CVertex>::iterator It = m_vertices.begin();
					CVertex &v0 = *It;
					It++;
					CVertex &v1 = *It;
					start_span = new Span(v0.m_p, v1, true);
				}
				*this = a.m_curves.front();
				if(this->IsClockwise() != cw)this->Reverse();
				if(start_span)
				{
					Point forward = start_span->GetVector(0.0);
					Point left(-forward.y, forward.x);
					Point offset_start = start_span->m_p + left * leftwards_value;
					this->ChangeStart(this->NearestPoint(offset_start));
					delete start_span;
				}
				success = true;
			}
		}
	}

	return success;
}

void CCurve::GetSpans(std::list<Span> &spans)const
{
	const Point *prev_p = NULL;
	for(std::list<CVertex>::const_iterator It = m_vertices.begin(); It != m_vertices.end(); It++)
	{
		const CVertex& vertex = *It;
		if(prev_p)
		{
			spans.push_back(Span(*prev_p, vertex));
		}
		prev_p = &(vertex.m_p);
	}
}

void CCurve::OffsetForward(double forwards_value, bool refit_arcs)
{
	// for drag-knife compensation

	// replace arcs with lines
	UnFitArcs();

	std::list<Span> spans;
	GetSpans(spans);

	m_vertices.clear();

	// shift all the spans
	for(std::list<Span>::iterator It = spans.begin(); It != spans.end(); It++)
	{
		Span &span = *It;
		Point v = span.GetVector(0.0);
		v.normalize();
		Point shift = v * forwards_value;
		span.m_p = span.m_p + shift;
		span.m_v.m_p = span.m_v.m_p + shift;
	}

	// loop through the shifted spans
	for(std::list<Span>::iterator It = spans.begin(); It != spans.end();)
	{
		Span &span = *It;
		Point v = span.GetVector(0.0);
		v.normalize();

		// add the span
		if(It == spans.begin())m_vertices.push_back(span.m_p);
		m_vertices.push_back(span.m_v.m_p);

		It++;
		if(It != spans.end())
		{
			Span &next_span = *It;
			Point nv = next_span.GetVector(0.0);
			nv.normalize();
			double sin_angle = v ^ nv;
			bool sharp_corner = ( fabs(sin_angle) > 0.5 ); // angle > 30 degrees

			if(sharp_corner)
			{
				// add an arc to the start of the next span
				int arc_type = ((sin_angle > 0) ? 1 : (-1));
				Point centre = span.m_v.m_p - v * forwards_value;
				m_vertices.push_back(CVertex(arc_type, next_span.m_p, centre));
			}
		}
	}

	if(refit_arcs)
		FitArcs(); // find the arcs again
	else
		UnFitArcs(); // convert those little arcs added to lines
}

double CCurve::Perim()const
{
	const Point *prev_p = NULL;
	double perim = 0.0;
	for(std::list<CVertex>::const_iterator It = m_vertices.begin(); It != m_vertices.end(); It++)
	{
		const CVertex& vertex = *It;
		if(prev_p)
		{
			Span span(*prev_p, vertex);
			perim += span.Length();
		}
		prev_p = &(vertex.m_p);
	}

	return perim;
}

Point CCurve::PerimToPoint(double perim)const
{
	if(m_vertices.size() == 0)return Point(0, 0);

	const Point *prev_p = NULL;
	double kperim = 0.0;
	for(std::list<CVertex>::const_iterator It = m_vertices.begin(); It != m_vertices.end(); It++)
	{
		const CVertex& vertex = *It;
		if(prev_p)
		{
			Span span(*prev_p, vertex);
			double length = span.Length();
			if(perim < kperim + length)
			{
				Point p = span.MidPerim(perim - kperim);
				return p;
			}
			kperim += length;
		}
		prev_p = &(vertex.m_p);
	}

	return m_vertices.back().m_p;
}

double CCurve::PointToPerim(const Point& p)const
{
	double best_dist = 0.0;
	double perim_at_best_dist = 0.0;
	Point best_point = Point(0, 0);
	bool best_dist_found = false;

	double perim = 0.0;

	const Point *prev_p = NULL;
	bool first_span = true;
	for(std::list<CVertex>::const_iterator It = m_vertices.begin(); It != m_vertices.end(); It++)
	{
		const CVertex& vertex = *It;
		if(prev_p)
		{
			Span span(*prev_p, vertex, first_span);
			Point near_point = span.NearestPoint(p);
			first_span = false;
			double dist = near_point.dist(p);
			if(!best_dist_found || dist < best_dist)
			{
				best_dist = dist;
				Span span_to_point(*prev_p, CVertex(span.m_v.m_type, near_point, span.m_v.m_c));
				perim_at_best_dist = perim + span_to_point.Length();
				best_dist_found = true;
			}
			perim += span.Length();
		}
		prev_p = &(vertex.m_p);
	}
	return perim_at_best_dist;
}

void CCurve::operator+=(const CCurve& curve)
{
	for(std::list<CVertex>::const_iterator It = curve.m_vertices.begin(); It != curve.m_vertices.end(); It++)
	{
		const CVertex &vt = *It;
		if(It == curve.m_vertices.begin())
		{
			if((m_vertices.size() == 0) || (It->m_p != m_vertices.back().m_p))
			{
				m_vertices.push_back(CVertex(It->m_p));
			}
		}
		else
		{
			m_vertices.push_back(vt);
		}
	}
}

void CCurve::CurveIntersections(const CCurve& c, std::list<Point> &pts)const
{
	CArea a;
	a.append(*this);
	a.CurveIntersections(c, pts);
}

void CCurve::Transform(const Matrix& matrix)
{
	for (std::list<CVertex>::iterator It = m_vertices.begin(); It != m_vertices.end(); It++)
	{
		CVertex& vertex = *It;
		vertex.Transform(matrix);
	}
}

void CCurve::SpanIntersections(const Span& s, std::list<Point> &pts)const
{
	std::list<Span> spans;
	GetSpans(spans);
	for(std::list<Span>::iterator It = spans.begin(); It != spans.end(); It++)
	{
		Span& span = *It;
		std::list<Point> pts2;
		span.Intersect(s, pts2);
		for(std::list<Point>::iterator It = pts2.begin(); It != pts2.end(); It++)
		{
			Point &pt = *It;
			if(pts.size() == 0)
			{
				pts.push_back(pt);
			}
			else
			{
				if(pt != pts.back())pts.push_back(pt);
			}
		}
	}
}

bool CCurve::GetMaxCutterRadius(double &radius)const
{
	// assuming anti-clockwise for outer curve, clockwise for inside curve
	bool max_rad_found = false;
	double max_rad = 0.0;

	std::list<Span> spans;
	GetSpans(spans);
	const Span *prev_span = NULL;

	if (m_vertices.size() > 2 && IsClosed())
	{
		prev_span = &(spans.back());
	}

	for (std::list<Span>::iterator It = spans.begin(); It != spans.end(); It++)
	{
		Span &span = *It;
		if (span.m_v.m_type == -1)
		{
			if (!max_rad_found || (span.GetRadius() < max_rad))
			{
				max_rad = span.GetRadius();
				max_rad_found = true;
			}
		}

		if (prev_span)
		{
			Point v0 = prev_span->GetVector(1.0);
			Point left = ~v0;
			Point v1 = span.GetVector(0.0);
			double dotp = v1*left;
			if (dotp < -0.2)
			{
				radius = 0.0;// sharp internal corner found
				return true;
			}
		}
		prev_span = &span;
	}

	if (max_rad_found)radius = max_rad;
	return max_rad_found;
}

const Point Span::null_point = Point(0, 0);
const CVertex Span::null_vertex = CVertex(Point(0, 0));

Span::Span():m_start_span(false), m_p(null_point), m_v(null_vertex){}

Point Span::NearestPointNotOnSpan(const Point& p)const
{
	if(m_v.m_type == 0)
	{
		Point Vs = (m_v.m_p - m_p);
		Vs.normalize();
		double dp = (p - m_p) * Vs;		
		return (Vs * dp) + m_p;
	}
	else
	{
		double radius = m_p.dist(m_v.m_c);
		double r = p.dist(m_v.m_c);
		if(r < TOLERANCE)return m_p;
		Point vc = (m_v.m_c - p);
		return p + vc * ((r - radius) / r);
	}
}

Point Span::NearestPoint(const Point& p)const
{
	Point np = NearestPointNotOnSpan(p);
	double t = Parameter(np);
	if(t >= 0.0 && t <= 1.0)return np;

	double d1 = p.dist(this->m_p);
	double d2 = p.dist(this->m_v.m_p);

	if(d1 < d2)return this->m_p;
	else return m_v.m_p;
}

double Span::Dist(const Point& p)const
{
	return p.Dist(NearestPoint(p));
}

Point Span::MidPerim(double d)const {
	/// returns a point which is 0-d along span
	Point p;
	if(m_v.m_type == 0) {
		Point vs = m_v.m_p - m_p;
		vs.normalize();
		p = vs * d + m_p;
	}
	else {
		Point v = m_p - m_v.m_c;
		double radius = v.length();
		v.Rotate(d * m_v.m_type / radius);
		p = v + m_v.m_c;
	}
	return p;
}

Point Span::MidParam(double param)const {
	/// returns a point which is 0-1 along span
	if(fabs(param) < 0.00000000000001)return m_p;
	if(fabs(param - 1.0) < 0.00000000000001)return m_v.m_p;

	Point p;
	if(m_v.m_type == 0) {
		Point vs = m_v.m_p - m_p;
		p = vs * param + m_p;
	}
	else {
		Point v = m_p - m_v.m_c;
		v.Rotate(param * IncludedAngle());
		p = v + m_v.m_c;
	}
	return p;
}

Point Span::NearestPointToSpan(const Span& p, double &d)const
{
	Point midpoint = MidParam(0.5);
	Point np = p.NearestPoint(m_p);
	Point best_point = m_p;
	double dist = np.dist(m_p);
	if(p.m_start_span)dist -= (CArea::m_accuracy * 2); // give start of curve most priority
	Point npm = p.NearestPoint(midpoint);
	double dm = npm.dist(midpoint) - CArea::m_accuracy; // lie about midpoint distance to give midpoints priority
	if(dm < dist){dist = dm; best_point = midpoint;}
	Point np2 = p.NearestPoint(m_v.m_p);
	double dp2 = np2.dist(m_v.m_p);
	if(dp2 < dist){dist = dp2; best_point = m_v.m_p;}
	d = dist;
	return best_point;
}

Point Span::NearestPoint(const Span& p, double *d)const
{
	double best_dist;
	Point best_point = this->NearestPointToSpan(p, best_dist);

	// try the other way round too
	double best_dist2;
	Point best_point2 = p.NearestPointToSpan(*this, best_dist2);
	if(best_dist2 < best_dist)
	{
		best_point = NearestPoint(best_point2);
		best_dist = best_dist2;
	}

	if(d)*d = best_dist;
	return best_point;
}

static int GetQuadrant(const Point& v){
	// 0 = [+,+], 1 = [-,+], 2 = [-,-], 3 = [+,-]
	if(v.x > 0)
	{
		if(v.y > 0)
			return 0;
		return 3;
	}
	if(v.y > 0)
		return 1;
	return 2;
}

static Point QuadrantEndPoint(int i)
{
	if(i >3)i-=4;
	switch(i)
	{
	case 0:
		return Point(0.0,1.0);
	case 1:
		return Point(-1.0,0.0);
	case 2:
		return Point(0.0,-1.0);
	default:
		return Point(1.0,0.0);
	}
}

void Span::GetBox(CBox2D &box)const
{
	box.Insert(m_p);
	box.Insert(m_v.m_p);

	if(this->m_v.m_type)
	{
		// arc, add quadrant points
		Point vs = m_p - m_v.m_c;
		Point ve = m_v.m_p - m_v.m_c;
		int qs = GetQuadrant(vs);
		int qe = GetQuadrant(ve);
		if(m_v.m_type == -1)
		{
			// swap qs and qe
			int t=qs;
			qs = qe;
			qe = t;
		}

		if(qe<qs)qe = qe + 4;

		double rad = m_v.m_p.dist(m_v.m_c);

		for(int i = qs; i<qe; i++)
		{
			box.Insert(m_v.m_c + QuadrantEndPoint(i) * rad);
		}
	}
}

double Span::IncludedAngle()const
{
	if(m_v.m_type)
	{
		Point vs = ~(m_p - m_v.m_c);
		Point ve = ~(m_v.m_p - m_v.m_c);
		if(m_v.m_type == -1)
		{
			vs = -vs;
			ve = -ve;
		}
		vs.normalize();
		ve.normalize();

		return ::IncludedAngle(vs, ve, m_v.m_type);
	}

	return 0.0;
}

double Span::GetArea()const
{
	if(m_v.m_type)
	{
		double angle = IncludedAngle();
		double radius = m_p.dist(m_v.m_c);
		return ( 0.5 * ((m_v.m_c.x - m_p.x) * (m_v.m_c.y + m_p.y) - (m_v.m_c.x - m_v.m_p.x) * (m_v.m_c.y + m_v.m_p.y) - angle * radius * radius));
	}

	return 0.5 * (m_v.m_p.x - m_p.x) * (m_p.y + m_v.m_p.y);
}

double Span::Parameter(const Point& p)const
{
	double t;
	if(m_v.m_type == 0) {
		Point v0 = p - m_p;
		Point vs = m_v.m_p - m_p;
		double length = vs.length();
		vs.normalize();
		t = vs * v0;
		t = t / length;
	}
	else
	{
		// true if p lies on arc span sp (p must be on circle of span)
		Point vs = ~(m_p - m_v.m_c);
		Point v = ~(p - m_v.m_c);
		vs.normalize();
		v.normalize();
		if(m_v.m_type == -1){
			vs = -vs;
			v = -v;
		}
		double ang = ::IncludedAngle(vs, v, m_v.m_type);
		double angle = IncludedAngle();
		t = ang / angle;
	}
	return t;
}

bool Span::On(const Point& p, double* t)const
{
	if(p != NearestPoint(p))return false;
	if(t)*t = Parameter(p);
	return true;
}

double Span::Length()const
{
	if(m_v.m_type) {
		double radius = m_p.dist(m_v.m_c);
		return fabs(IncludedAngle()) * radius;
	}

	return m_p.dist(m_v.m_p);
}

Point Span::GetVector(double fraction)const
{
	/// returns the direction vector at point which is 0-1 along span
	if(m_v.m_type == 0){
		Point v(m_p, m_v.m_p);
		v.normalize();
		return v;
	}

	Point p= MidParam(fraction);
	Point v(m_v.m_c, p);
	v.normalize();
	if(m_v.m_type == 1)
	{
		return Point(-v.y, v.x);
	}
	else
	{
		return Point(v.y, -v.x);
	}
}


void LineLineIntof(const Span& sp0, const Span& sp1, std::list<Point> &pts) {
	// intersection between 2 Line2d
	// returns 0 for no intersection in range of either span
	// returns 1 for intersction in range of both spans
	// t[0] is parameter on sp0,
	// t[1] is parameter on sp1
	Point v0(sp0.m_p, sp0.m_v.m_p);
	Point v1(sp1.m_p, sp1.m_v.m_p);
	Point v2(sp0.m_p, sp1.m_p);

	double cp = v1 ^ v0;

	if (fabs(cp) >= UNIT_VECTOR_TOLERANCE) {
		double toler = TOLERANCE / sp0.Length();				// calc a parametric tolerance
		double t0 = (v1 ^ v2) / cp;
		if (t0 < -toler || t0 > 1 + toler) return;
		toler = TOLERANCE / sp1.Length();
		double t1 = (v0 ^ v2) / cp;
		if (t1 < -toler || t1 > 1 + toler) return;
		pts.push_back(v0 * t0 + sp0.m_p);
	}
}

void LineArcIntof(const Span& line, const Span& arc, std::list<Point> &pts) {
	// inters of line arc
	// solving	x = x0 + dx * t			x = y0 + dy * t
	//			x = xc + R * cos(a)		y = yc + R * sin(a)		for t
	// gives :-  t� (dx� + dy�) + 2t(dx*dx0 + dy*dy0) + (x0-xc)� + (y0-yc)� - R� = 0
	int nRoots;
	Point v0(arc.m_v.m_c, line.m_p);
	Point v1(line.m_p, line.m_v.m_p);
	double s = v1.magnitudesqd();
	double arc_radius = arc.GetRadius();

	double t[2];

	if ((nRoots = quadratic(s, 2 * (v0 * v1), v0.magnitudesqd() - arc_radius * arc_radius, t[0], t[1])) != 0) {
		double toler = TOLERANCE / sqrt(s);							// calc a parametric tolerance
		if (t[0] > -toler && t[0] < 1 + toler) {
			Point p = v1 * t[0] + line.m_p;
			if (arc.On(p))
				pts.push_back(p);
		}
		if (nRoots == 2) {
			if (t[1] > -toler && t[1] < 1 + toler) {
				Point p = v1 * t[1] + line.m_p;
				if (arc.On(p))
					pts.push_back(p);
			}
		}
	}
}

void ArcArcIntof(const Span& arc0, const Span& arc1, std::list<Point> &pts) {
	// Intof 2 arcs
	Point pLeft, pRight;
	int numInts = Intof(Circle(arc0.m_v.m_c, arc0.GetRadius()), Circle(arc1.m_v.m_c, arc1.GetRadius()), pLeft, pRight);

	if (numInts > 0) {
		if (arc0.On(pLeft) && arc1.On(pLeft))
			pts.push_back(pLeft);
		if (arc0.On(pRight) && arc1.On(pRight))
			pts.push_back(pRight);
	}
}

void Span::Intersect(const Span& s, std::list<Point> &pts)const
{
	if (!m_v.m_type) {
		if (!s.m_v.m_type) {
			// line line
			LineLineIntof(*this, s, pts);
		}
		else {
			// line arc
			LineArcIntof(*this, s, pts);
}
		}
	else {
		if (!s.m_v.m_type) {
			// arc line
			LineArcIntof(s, *this, pts);
		}
		else {
			// arc arc
			ArcArcIntof(*this, s, pts);
		}
	}
}

void Span::Reverse()
{
	Point t = m_p;
	m_p = m_v.m_p;
	m_v.m_p = t;
	m_v.m_type = -m_v.m_type;
}

double Span::GetRadius()const
{
	if (m_v.m_type == 0)
		return 0.0;
	else
	{
		return m_p.dist(m_v.m_c);
	}
}

Span Span::Offset(double offset)
{
	Span Offsp = *this;
	if (FNEZ(offset)) {
		Point vs = this->GetVector(0.0);
		if (!m_v.m_type) {
			// straight
			Offsp.m_p.x -= offset * vs.y;
			Offsp.m_p.y += offset * vs.x;

			Offsp.m_v.m_p.x -= offset * vs.y;
			Offsp.m_v.m_p.y += offset * vs.x;
		}
		else {
			//	 circular span
			//	double coffset = (double) dir * offset;
			Point ve = this->GetVector(1.0);
			Offsp.m_p.x -= vs.y * offset;
			Offsp.m_p.y += vs.x * offset;

			Offsp.m_v.m_p.x -= ve.y * offset;
			Offsp.m_v.m_p.y += ve.x * offset;

			//				Offsp.radius -= dir * offset;
		}
	}
	return Offsp;
}


ostream & operator<<(ostream &os, const Span &span)
{
	return os << "Span p = " << span.m_p << ", v = " << span.m_v;
}

void tangential_arc(const Point &p0, const Point &p1, const Point &v0, Point &c, int &dir)
{
	// sets dir to 0, if a line is needed, else to 1 or -1 for acw or cw arc and sets c
	dir = 0;

	if (p0.Dist(p1) > 0.0000000001 && v0.magnitude() > 0.0000000001){
		Point v1(p0, p1);
		Point halfway(p0 + Point(v1 * 0.5));
		Plane pl1(halfway, v1);
		Plane pl2(p0, v0);
		Line plane_line;
		if (pl1.Intof(pl2, plane_line))
		{
			Line l1(halfway, v1);
			double t1, t2;
			Line lshort;
			plane_line.Shortest(l1, lshort, t1, t2);
			c.x = lshort.p0.x;
			c.y = lshort.p0.y;
			Point3d cross = Point3d(v0) ^ Point3d(v1);
			dir = (cross.z > 0) ? 1 : -1;
		}
	}
}