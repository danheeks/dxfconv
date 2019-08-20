// Area.cpp

// Copyright 2011, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "Area.h"
#include "AreaOrderer.h"

#include <map>

double CArea::m_accuracy = 0.01;
double CArea::m_units = 1.0;
bool CArea::m_fit_arcs = true;
//static const double PI = 3.1415926535897932;

void CArea::append(const CCurve& curve)
{
	m_curves.push_back(curve);
}

void CArea::FitArcs(){
	for(std::list<CCurve>::iterator It = m_curves.begin(); It != m_curves.end(); It++)
	{
		CCurve& curve = *It;
		curve.FitArcs();
	}
}

Point CArea::NearestPoint(const Point& p)const
{
	double best_dist = 0.0;
	Point best_point = Point(0, 0);
	for(std::list<CCurve>::const_iterator It = m_curves.begin(); It != m_curves.end(); It++)
	{
		const CCurve& curve = *It;
		Point near_point = curve.NearestPoint(p);
		double dist = near_point.dist(p);
		if(It == m_curves.begin() || dist < best_dist)
		{
			best_dist = dist;
			best_point = near_point;
		}
	}
	return best_point;
}

void CArea::GetBox(CBox2D &box)const
{
	for(std::list<CCurve>::const_iterator It = m_curves.begin(); It != m_curves.end(); It++)
	{
		const CCurve& curve = *It;
		curve.GetBox(box);
	}
}

void CArea::Reorder()
{
	// curves may have been added with wrong directions
	// test all kurves to see which one are outsides and which are insides and 
	// make sure outsides are anti-clockwise and insides are clockwise

	// returns 0, if the curves are OK
	// returns 1, if the curves are overlapping

	CAreaOrderer ao;
	for(std::list<CCurve>::iterator It = m_curves.begin(); It != m_curves.end(); It++)
	{
		CCurve& curve = *It;
		ao.Insert(&curve);
	}

	*this = ao.ResultArea();
}

void CArea::Split(std::list<CArea> &m_areas)const
{
	if(HolesLinked())
	{
		for(std::list<CCurve>::const_iterator It = m_curves.begin(); It != m_curves.end(); It++)
		{
			const CCurve& curve = *It;
			m_areas.push_back(CArea());
			m_areas.back().m_curves.push_back(curve);
		}
	}
	else
	{
		CArea a = *this;
		a.Reorder();

		for(std::list<CCurve>::const_iterator It = a.m_curves.begin(); It != a.m_curves.end(); It++)
		{
			const CCurve& curve = *It;
			if(curve.IsClockwise())
			{
				if(m_areas.size() > 0)
					m_areas.back().m_curves.push_back(curve);
			}
			else
			{
				m_areas.push_back(CArea());
				m_areas.back().m_curves.push_back(curve);
			}
		}
	}
}

double CArea::GetArea(bool always_add)const
{
	// returns the area of the area
	double area = 0.0;
	for(std::list<CCurve>::const_iterator It = m_curves.begin(); It != m_curves.end(); It++)
	{
		const CCurve& curve = *It;
		double a = curve.GetArea();
		if(always_add)area += fabs(a);
		else area += a;
	}
	return area;
}

eOverlapType GetOverlapType(const CCurve& c1, const CCurve& c2)
{
	CArea a1;
	a1.m_curves.push_back(c1);
	CArea a2;
	a2.m_curves.push_back(c2);

	return GetOverlapType(a1, a2);
}

eOverlapType GetOverlapType(const CArea& a1, const CArea& a2)
{
	CArea A1(a1);

	A1.Subtract(a2);
	if(A1.m_curves.size() == 0)
	{
		return eInside;
	}

	CArea A2(a2);
	A2.Subtract(a1);
	if(A2.m_curves.size() == 0)
	{
		return eOutside;
	}

	A1 = a1;
	A1.Intersect(a2);
	if(A1.m_curves.size() == 0)
	{
		return eSiblings;
	}

	return eCrossing;
}

bool IsInside(const Point& p, const CCurve& c)
{
	CArea a;
	a.m_curves.push_back(c);
	return IsInside(p, a);
}

bool IsInside(const Point& p, const CArea& a)
{
	CArea a2;
	CCurve c;
	c.m_vertices.push_back(CVertex(Point(p.x - 0.01, p.y - 0.01)));
	c.m_vertices.push_back(CVertex(Point(p.x + 0.01, p.y - 0.01)));
	c.m_vertices.push_back(CVertex(Point(p.x + 0.01, p.y + 0.01)));
	c.m_vertices.push_back(CVertex(Point(p.x - 0.01, p.y + 0.01)));
	c.m_vertices.push_back(CVertex(Point(p.x - 0.01, p.y - 0.01)));
	a2.m_curves.push_back(c);
	a2.Intersect(a);
	if(fabs(a2.GetArea()) < 0.0004)return false;
	return true;
}

void CArea::SpanIntersections(const Span& span, std::list<Point> &pts)const
{
	// this returns all the intersections of this area with the given span, ordered along the span

	// get all points where this area's curves intersect the span
	std::list<Point> pts2;
	for(std::list<CCurve>::const_iterator It = m_curves.begin(); It != m_curves.end(); It++)
	{
		const CCurve &c = *It;
		c.SpanIntersections(span, pts2);
	}

	// order them along the span
	std::multimap<double, Point> ordered_points;
	for(std::list<Point>::iterator It = pts2.begin(); It != pts2.end(); It++)
	{
		Point &p = *It;
		double t;
		if(span.On(p, &t))
		{
			ordered_points.insert(std::make_pair(t, p));
		}
	}

	// add them to the given list of points
	for(std::multimap<double, Point>::iterator It = ordered_points.begin(); It != ordered_points.end(); It++)
	{
		Point p = It->second;
		pts.push_back(p);
	}
}

void CArea::CurveIntersections(const CCurve& curve, std::list<Point> &pts)const
{
	// this returns all the intersections of this area with the given curve, ordered along the curve
	std::list<Span> spans;
	curve.GetSpans(spans);
	for(std::list<Span>::iterator It = spans.begin(); It != spans.end(); It++)
	{
		Span& span = *It;
		std::list<Point> pts2;
		SpanIntersections(span, pts2);
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

class ThickLine
{
public:
	CArea m_area;
	CCurve m_curve;

	ThickLine(const CCurve& curve)
	{
		m_curve = curve;
		m_area.append(curve);
		m_area.Thicken(0.001);
	}
};

void CArea::InsideCurves(const CCurve& curve, std::list<CCurve> &curves_inside)const
{
	//1. find the intersectionpoints between these two curves.
	std::list<Point> pts;
	CurveIntersections(curve, pts);

	//2.seperate curve2 in multiple curves between these intersections.
	std::list<CCurve> separate_curves;
	curve.ExtractSeparateCurves(pts, separate_curves);

	//3. if the midpoint of a seperate curve lies in a1, then we return it.
	for(std::list<CCurve>::iterator It = separate_curves.begin(); It != separate_curves.end(); It++)
	{
		CCurve &curve = *It;
		double length = curve.Perim();
		Point mid_point = curve.PerimToPoint(length * 0.5);
		if(IsInside(mid_point, *this))curves_inside.push_back(curve);
	}
}

CArea CArea::Swept(const Point& vector)const
{
	// make a sweep area by uniting lots of lozenges
	std::list<CCurve> curves;

	for (std::list<CCurve>::const_iterator It = m_curves.begin(); It != m_curves.end(); It++)
	{
		const CCurve &curve = *It;
		std::list<Span> spans;
		curve.GetSpans(spans);
		for (std::list<Span>::iterator It = spans.begin(); It != spans.end(); It++)
		{
			Span& span = *It;
			CCurve lozenge;
			lozenge.m_vertices.push_back(span.m_p);
			lozenge.m_vertices.push_back(span.m_v);
			lozenge.m_vertices.push_back(span.m_v.m_p + vector);
			lozenge.m_vertices.push_back(CVertex(-span.m_v.m_type, span.m_p + vector, span.m_v.m_c + vector));
			lozenge.m_vertices.push_back(span.m_p);
			if (lozenge.IsClockwise())lozenge.Reverse();
			curves.push_back(lozenge);
		}
	}

	CArea area = CArea::UniteCurves(curves);
	area.Union(*this);
	return area;
}


void CArea::Transform(const Matrix& matrix)
{
	for (std::list<CCurve>::iterator It = m_curves.begin(); It != m_curves.end(); It++)
	{
		CCurve &curve = *It;
		curve.Transform(matrix);
	}
}
