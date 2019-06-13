// Sketch.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "Sketch.h"
#include "HLine.h"
#include "HArc.h"
#include "HSpline.h"

std::string CSketch::m_sketch_order_str[MaxSketchOrderTypes] = {
	std::string("unknown"),
	std::string("empty"),
	std::string("open"),
	std::string("reverse"),
	std::string("bad"),
	std::string("re-order"),
	std::string("clockwise"),
	std::string("counter-clockwise"),
	std::string("multiple"),
	std::string("has circles"),
};

CSketch::CSketch():m_order(SketchOrderTypeUnknown)
{
	m_title = L"Sketch";
}

CSketch::CSketch(const CSketch& c)
{
	operator=(c);
}

CSketch::~CSketch()
{
}

const CSketch& CSketch::operator=(const CSketch& c)
{
    if (this != &c)
    {
        // just copy all the lines and arcs, not the id
        IdNamedObjList::operator =(c);

        color = c.color;
        m_order = c.m_order;
        m_title = c.m_title;
    }

	return *this;
}

static bool SketchOrderAvailable(SketchOrderType old_order, SketchOrderType new_order)
{
	// can we change from the older order type to the new order type?
	if(old_order == new_order)return true;

	switch(old_order)
	{
	case SketchOrderTypeOpen:
		switch(new_order)
		{
		case SketchOrderTypeReverse:
			return true;
		default:
			break;
		}
		break;

	case SketchOrderTypeBad:
		switch(new_order)
		{
		case SketchOrderTypeReOrder:
			return true;
		default:
			break;
		}
		break;

	case SketchOrderTypeCloseCW:
		switch(new_order)
		{
		case SketchOrderTypeCloseCCW:
			return true;
		default:
			break;
		}
		break;

	case SketchOrderTypeCloseCCW:
		switch(new_order)
		{
		case SketchOrderTypeCloseCW:
			return true;
		default:
			break;
		}
		break;

	case SketchOrderTypeMultipleCurves:
		switch(new_order)
		{
		case SketchOrderTypeReOrder:
			return true;
		default:
			break;
		}
		break;

	default:
		break;
	}

	return false;
}

// static
void CSketch::ReverseObject(HeeksObj* object)
{
	// reverse object
	switch(object->GetType()){
case LineType:
	((HLine*)object)->Reverse();
	break;
case ArcType:
	((HArc*)object)->Reverse();
	break;
case SplineType:
	((HSpline*)object)->Reverse();
	break;
default:
	break;
	}
}

HeeksObj *CSketch::MakeACopy(void)const
{
	return (IdNamedObjList*)(new CSketch(*this));
}

void CSketch::SetColor(const HeeksColor &col)
{
	std::list<HeeksObj*>::iterator It;
	for(It=m_objects.begin(); It!=m_objects.end() ;It++)
	{
		HeeksObj* object = *It;
		object->SetColor(col);
	}
}

const HeeksColor* CSketch::GetColor()const
{
	if(m_objects.size() == 0)return NULL;
	return m_objects.front()->GetColor();
}

SketchOrderType CSketch::GetSketchOrder()
{
	if(m_order == SketchOrderTypeUnknown)CalculateSketchOrder();
	return m_order;
}

void CSketch::CalculateSketchOrder()
{
	if(m_objects.size() == 0)
	{
		m_order = SketchOrderTypeEmpty;
		return;
	}

	HeeksObj* prev_object = NULL;
	HeeksObj* first_object = NULL;

	bool well_ordered = true;

	std::list<HeeksObj*>::iterator It;
	for(It=m_objects.begin(); It!=m_objects.end() ;It++)
	{
		HeeksObj* object = *It;

		if(object->GetType() == CircleType)
		{
			m_order = SketchOrderHasCircles;
			return;
		}

		if(prev_object)
		{
			double prev_e[3], s[3];
			if(!prev_object->GetEndPoint(prev_e)){well_ordered = false; break;}
			if(!object->GetStartPoint(s)){well_ordered = false; break;}
			if(!(make_point(prev_e).IsEqual(make_point(s), theApp.m_sketch_reorder_tol))){well_ordered = false; break;}
		}

		if(first_object == NULL)first_object = object;
		prev_object = object;
	}

	if(well_ordered)
	{
		if(prev_object && first_object)
		{
			double e[3], s[3];
			if(prev_object->GetEndPoint(e))
			{
				if(first_object->GetStartPoint(s))
				{
					if(make_point(e).IsEqual(make_point(s), theApp.m_sketch_reorder_tol))
					{
						// closed
						if(IsClockwise())m_order = SketchOrderTypeCloseCW;
						else m_order = SketchOrderTypeCloseCCW;
						return;
					}
				}
			}
		}

		m_order = SketchOrderTypeOpen;
		return;
	}

	m_order = SketchOrderTypeBad; // although it might still be multiple, but will have to wait until ReOrderSketch is done.
}

bool CSketch::ReOrderSketch(SketchOrderType new_order)
{

	return false;
}

void CSketch::ReLinkSketch()
{
}

void CSketch::ReverseSketch()
{
}

void CSketch::ExtractSeparateSketches(std::list<HeeksObj*> &new_separate_sketches, const bool allow_individual_objects /* = false */ )
{
	CSketch* re_ordered_sketch = NULL;
	CSketch* sketch = this;

	if(GetSketchOrder() == SketchOrderTypeBad)
	{
	    // Duplicate and reorder the sketch to see if it's possible to make separate connected sketches.
		re_ordered_sketch = (CSketch*)(this->MakeACopy());
		re_ordered_sketch->ReOrderSketch(SketchOrderTypeReOrder);
		sketch = re_ordered_sketch;
	}
	if(sketch->GetSketchOrder() == SketchOrderTypeMultipleCurves || GetSketchOrder() == SketchOrderHasCircles)
	{
		std::list<HeeksObj*> objects_for_relinker;
		for(std::list<HeeksObj*>::iterator It=m_objects.begin(); It!=m_objects.end() ;It++)
		{
			HeeksObj* object = *It;
			if(object->GetType() == CircleType)
			{
				CSketch* new_object = new CSketch();
				new_object->color = color;
				new_object->Add(object->MakeACopy(), NULL);
				new_separate_sketches.push_back(new_object);
			}
			else
			{
				objects_for_relinker.push_back(object);
			}
		}

		// Make separate connected sketches from the child elements.
		CSketchRelinker relinker(objects_for_relinker);

		relinker.Do();

		for(std::list< std::list<HeeksObj*> >::iterator It = relinker.m_new_lists.begin(); It != relinker.m_new_lists.end(); It++)
		{
			std::list<HeeksObj*>& list = *It;
			CSketch* new_object = new CSketch();
			new_object->color = color;
			for(std::list<HeeksObj*>::iterator It2 = list.begin(); It2 != list.end(); It2++)
			{
				HeeksObj* object = *It2;
				new_object->Add(object->MakeACopy(), NULL);
			}
			new_separate_sketches.push_back(new_object);
		}
	}
	else
	{
        // The sketch does not seem to relink into separate connected shapes.  Just export
        // all the sketch's children as separate objects instead.
		if (allow_individual_objects)
		{
			for (HeeksObj *child = sketch->GetFirstChild(); child != NULL; child = sketch->GetNextChild())
			{
				new_separate_sketches.push_back( child->MakeACopy() );
			}
		}
	}

	if(re_ordered_sketch)delete re_ordered_sketch;
}

double CSketch::GetArea()const
{
	double area = 0.0;

	for(std::list<HeeksObj*>::const_iterator It=m_objects.begin(); It!=m_objects.end() ;It++)
	{
		HeeksObj* object = *It;
		switch(object->GetType())
		{
		case ArcType:
			{
				double angle = ((HArc*)object)->IncludedAngle();
				double radius = ((HArc*)object)->m_radius;
				double p0x = ((HArc*)object)->A.X();
				double p0y = ((HArc*)object)->A.Y();
				double p1x = ((HArc*)object)->B.X();
				double p1y = ((HArc*)object)->B.Y();
				double pcx = ((HArc*)object)->C.X();
				double pcy = ((HArc*)object)->C.Y();
				area += ( 0.5 * ((pcx - p0x) * (pcy + p0y) - (pcx - p1x) * (pcy + p1y) - angle * radius * radius));
			}
			break;
		default:
			// treat all others as lines
			{
				double s[3], e[3];
				if(!object->GetStartPoint(s))break;
				if(!object->GetEndPoint(e))break;
				area += (0.5 * (e[0] - s[0]) * (s[1] + e[1]));
			}
			break;
		}
	}

	return area;
}

bool CSketch::Add(HeeksObj* object, HeeksObj* prev_object)
{
	m_order = SketchOrderTypeUnknown;
	return IdNamedObjList::Add(object, prev_object);
}

void CSketch::Remove(HeeksObj* object)
{
	m_order = SketchOrderTypeUnknown;
	IdNamedObjList::Remove(object);
}

bool CSketchRelinker::TryAdd(HeeksObj* object)
{
	// if the object is not already added
	if(m_added_from_old_set.find(object) == m_added_from_old_set.end())
	{
		double old_point[3];
		double new_point[3];
		m_new_back->GetEndPoint(old_point);

		// try the object, the right way round
		object->GetStartPoint(new_point);
		if(make_point(old_point).IsEqual(make_point(new_point), theApp.m_sketch_reorder_tol))
		{
			m_new_lists.back().push_back(object);
			m_new_back = object;
			m_added_from_old_set.insert(object);
			return true;
		}

		// try the object, the wrong way round
		object->GetEndPoint(new_point);
		if(make_point(old_point).IsEqual(make_point(new_point), theApp.m_sketch_reorder_tol))
		{
			CSketch::ReverseObject(object);
			m_new_lists.back().push_back(object);
			m_new_back = object;
			m_added_from_old_set.insert(object);
			return true;
		}

		// try at the start
		m_new_front->GetStartPoint(old_point);

		// try the object, the right way round
		object->GetEndPoint(new_point);
		if(make_point(old_point).IsEqual(make_point(new_point), theApp.m_sketch_reorder_tol))
		{
			m_new_lists.back().push_front(object);
			m_new_front = object;
			m_added_from_old_set.insert(object);
			return true;
		}

		// try the object, the wrong way round
		object->GetStartPoint(new_point);
		if(make_point(old_point).IsEqual(make_point(new_point), theApp.m_sketch_reorder_tol))
		{
			CSketch::ReverseObject(object);
			m_new_lists.back().push_front(object);
			m_new_front = object;
			m_added_from_old_set.insert(object);
			return true;
		}
	}

	return false;
}

bool CSketchRelinker::AddNext()
{
	// returns true, if another object was added to m_new_lists

	if(m_new_back)
	{
		bool added = false;

		// look through all of the old list, starting at m_old_front
		std::list<HeeksObj*>::const_iterator It = m_old_front;
		do{
			It++;
			if(It == m_old_list.end())It = m_old_list.begin();
			HeeksObj* object = *It;

			added = TryAdd(object);

		}while(It != m_old_front && !added);

		if(added)return true;

		// nothing fits the current new list

		m_new_back = NULL;
		m_new_front = NULL;

		if(m_old_list.size() > m_added_from_old_set.size())
		{
			// there are still some to add, find a unused object
			for(std::list<HeeksObj*>::const_iterator It = m_old_list.begin(); It != m_old_list.end(); It++)
			{
				HeeksObj* object = *It;
				if(m_added_from_old_set.find(object) == m_added_from_old_set.end())
				{
					std::list<HeeksObj*> empty_list;
					m_new_lists.push_back(empty_list);
					m_new_lists.back().push_back(object);
					m_added_from_old_set.insert(object);
					m_old_front = It;
					m_new_back = object;
					m_new_front = object;
					return true;
				}
			}
		}
	}

	return false;
}

bool CSketchRelinker::Do()
{
	if(m_old_list.size() > 0)
	{
		HeeksObj* new_object = m_old_list.front();
		std::list<HeeksObj*> empty_list;
		m_new_lists.push_back(empty_list);
		m_new_lists.back().push_back(new_object);
		m_added_from_old_set.insert(m_old_list.front());
		m_old_front = m_old_list.begin();
		m_new_back = new_object;
		m_new_front = new_object;

		while(AddNext()){}
	}

	return true;
}


/**
	The Intersects() method is included in the heeks CAD interface as well as being
	a virtual method in the HeeksObj base class.  Since this Sketch object is, itself,
	simply a list of HeeksObj objects, we should be able to simply aggregate the
	intersection of the specified HeeksObj with all of 'our' HeeksObj objects.
 */
int CSketch::Intersects(const HeeksObj *object, std::list< double > *rl) const
{
	int number_of_intersections = 0;

	for (std::list<HeeksObj *>::const_iterator l_itObject = m_objects.begin(); l_itObject != m_objects.end(); l_itObject++)
	{
		number_of_intersections += (*l_itObject)->Intersects( object, rl );
	} // End for

	return(number_of_intersections);
} // End Intersects() method

bool CSketch::operator==( const CSketch & rhs ) const
{
    if (color != rhs.color) return(false);
	if (m_title != rhs.m_title) return(false);
	if (m_order != rhs.m_order) return(false);

	return(IdNamedObjList::operator==(rhs));
}

static bool FindClosestVertex(const gp_Pnt& p, const TopoDS_Face &face, TopoDS_Vertex &closest_vertex)
{
	// find closest vertex
	TopExp_Explorer ex1;
	double best_dist = -1;

	for(ex1.Init(face,TopAbs_VERTEX); ex1.More(); ex1.Next())
	{
		TopoDS_Vertex Vertex =TopoDS::Vertex(ex1.Current());
		gp_Pnt pos = BRep_Tool::Pnt(Vertex);
		double d = pos.Distance(p);
		if(best_dist < 0 || d < best_dist)
		{
			best_dist = d;
			closest_vertex = Vertex;
		}
	}

	return best_dist > -0.1;
}

#undef Status

bool CSketch::FilletAtPoint(const gp_Pnt& p, double rad)
{
	std::list<TopoDS_Shape> faces;
	bool fillet_done = false;


	return fillet_done;
}

CSketch* CSketch::SplineToBiarcs(double tolerance)const
{
	CSketch *new_sketch = new CSketch;

	for(std::list<HeeksObj*>::const_iterator It = m_objects.begin(); It != m_objects.end(); It++)
	{
		HeeksObj* span = *It;
		if(span->GetType() == SplineType)
		{
			std::list<HeeksObj*> new_spans;
			((HSpline*)span)->ToBiarcs(new_spans, tolerance);
			for(std::list<HeeksObj*>::iterator ItS = new_spans.begin(); ItS != new_spans.end(); ItS++)
			{
				new_sketch->Add(*ItS, NULL);
			}
		}
		else
		{
			new_sketch->Add(span->MakeACopy(), NULL);
		}
	}

	return new_sketch;
}

bool CSketch::IsCircle()const
{
	if (m_objects.size() == 1)
	{
		return m_objects.front()->GetType() == CircleType;
	}

	if (m_objects.size() > 1)
	{
		if (m_objects.front()->GetType() != ArcType)
			return false;
		HArc* reference_arc = (HArc*)(m_objects.front());
		gp_Circ reference_circle = reference_arc->GetCircle();

		for (std::list<HeeksObj*>::const_iterator It = m_objects.begin(); It != m_objects.end(); It++)
		{
			HeeksObj* span = *It;
			if (span->GetType() != ArcType)
				return false;

			HArc* arc = (HArc*)span;
			gp_Circ circle = arc->GetCircle();

			if (fabs(circle.Radius() - reference_circle.Radius()) > theApp.m_geom_tol)
				return false;

			if (!circle.Axis().Direction().IsEqual(reference_circle.Axis().Direction(), 0.01))
				return false;
		}
		return true;
	}

	return false;
}

bool CSketch::IsClosed()
{
	switch (GetSketchOrder())
	{
	case SketchOrderTypeCloseCW:
	case SketchOrderTypeCloseCCW:
		return true;
	default:
		return false;
	}
}

bool CSketch::HasMultipleSketches()
{
	switch (GetSketchOrder())
	{
	case SketchOrderTypeMultipleCurves:
		return true;
	case SketchOrderHasCircles:
		if (this->m_objects.size() > 1)
			return true;
		return false;
	default:
		return false;
	}
}