#include "stdafx.h"
#include "App.h"
#include "dxf.h"
#include "HLine.h"
#include "HPoint.h"
#include "HArc.h"
#include "HEllipse.h"
#include "HCircle.h"
#include "HSpline.h"
#include "strconv.h"
#include "Geom.h"

CApp theApp;

CApp::CApp() :ObjList()
{
	m_geom_tol = 0.000001;
	m_sketch_reorder_tol = 0.01;
	current_color = HeeksColor(0, 0, 0);
}


static void WriteDXFEntity(HeeksObj* object, CDxfWrite& dxf_file, const std::wstring parent_layer_name)
{
	std::wstring layer_name;

	if (parent_layer_name.size() == 0)
	{
		wchar_t s[1024];
		swprintf(s, 1024, L"%d", object->m_id);
		layer_name.assign(s);
	}
	else
	{
		layer_name = parent_layer_name;
	}

	switch (object->GetType())
	{
	case LineType:
	{
		HLine* l = (HLine*)object;
		double s[3], e[3];
		extract(l->A, s);
		extract(l->B, e);
		dxf_file.WriteLine(s, e, ws2s(layer_name).c_str(), l->m_thickness, l->m_extrusion_vector);
	}
	break;
	case PointType:
	{
		HPoint* p = (HPoint*)object;
		double s[3];
		extract(p->m_p, s);
		dxf_file.WritePoint(s, ws2s(layer_name).c_str());
	}
	break;
	case ArcType:
	{
		HArc* a = (HArc*)object;
		double s[3], e[3], c[3];
		extract(a->A, s);
		extract(a->B, e);
		extract(a->C, c);
		bool dir = a->m_axis.Direction().Z() > 0;
		dxf_file.WriteArc(s, e, c, dir, ws2s(layer_name).c_str(), a->m_thickness, a->m_extrusion_vector);
	}
	break;
	case EllipseType:
	{
		HEllipse* e = (HEllipse*)object;
		double c[3];
		extract(e->C, c);
		bool dir = e->m_zdir.Z() > 0;
		double maj_r = e->m_majr;
		double min_r = e->m_minr;
		double rot = e->GetRotation();
		dxf_file.WriteEllipse(c, maj_r, min_r, rot, 0, 2 * M_PI, dir, ws2s(layer_name).c_str(), 0.0);
	}
	break;
	case CircleType:
	{
		HCircle* cir = (HCircle*)object;
		double c[3];
		extract(cir->m_axis.Location(), c);
		double radius = cir->m_radius;
		dxf_file.WriteCircle(c, radius, ws2s(layer_name).c_str(), cir->m_thickness, cir->m_extrusion_vector);
	}
	break;
	default:
	{
		if (parent_layer_name.size() == 0)
		{
			layer_name.clear();
			if ((object->GetShortString() != NULL) && (std::wstring(object->GetTypeString()) != std::wstring(object->GetShortString())))
			{
				layer_name.assign(object->GetShortString());
			}
			else
			{
				wchar_t s[1024];
				swprintf(s, 1024, L"%d", object->m_id);
				layer_name.assign(s);   // Use the ID as a layer name so that it's unique.
			}
		}
		else
		{
			layer_name = parent_layer_name;
		}

		for (HeeksObj* child = object->GetFirstChild(); child; child = object->GetNextChild())
		{

			// recursive
			WriteDXFEntity(child, dxf_file, layer_name);
		}
	}
	}
}

void CApp::SaveDXFFile(const std::list<HeeksObj*>& objects, const wchar_t *filepath)
{
	CDxfWrite dxf_file(ws2s(filepath).c_str());
	if (dxf_file.Failed())
	{
		std::wstring str = std::wstring(L"couldn't open file") + filepath;
		//wxMessageBox(str);
		return;
	}

	// write all the objects
	for (std::list<HeeksObj*>::const_iterator It = objects.begin(); It != objects.end(); It++)
	{
		HeeksObj* object = *It;
		// At this level, don't assign each element to its own layer.  We only want sketch objects
		// to be located on their own layer.  This will be done from within the WriteDXFEntity() method.
		WriteDXFEntity(object, dxf_file, _T(""));
	}

	// when dxf_file goes out of scope it writes the file, see ~CDxfWrite
}

static double tolerance_for_SplinesToBiarcs = 0.001;
static void SplinesToBiarcs(ObjList* objlist)
{
	std::list<HeeksObj*> new_objects;

	// loop through all the objects converting splines to biarcs
	for (std::list<HeeksObj*>::const_iterator It = objlist->m_objects.begin(); It != objlist->m_objects.end(); It++)
	{
		HeeksObj* object = *It;
		if (object->GetType() == SplineType)
		{
			std::list<HeeksObj*> new_spans;
			((HSpline*)object)->ToBiarcs(new_spans, tolerance_for_SplinesToBiarcs);
			for (std::list<HeeksObj*>::iterator ItS = new_spans.begin(); ItS != new_spans.end(); ItS++)
			{
				new_objects.push_back(*ItS);
			}
		}
		else
		{
			if (object->GetType() == SketchType)
			{
				SplinesToBiarcs((CSketch*)object);
			}
		
			new_objects.push_back(object);
		}
	}

	objlist->m_objects = new_objects;
}

void CApp::SplinesToBiarcs(double tolerance)
{
	tolerance_for_SplinesToBiarcs = tolerance;

	::SplinesToBiarcs(this);
}