#pragma once

#include "ObjList.h"
#include "HeeksColor.h"

class CApp : public ObjList
{
public:
	double m_geom_tol;
	double m_sketch_reorder_tol;
	HeeksColor current_color;
	unsigned int m_number_of_splines_converted;

	CApp();

	void SaveDXFFile(const std::list<HeeksObj*>& objects, const wchar_t *filepath);
	void SplinesToBiarcs(double tolerance);
};

extern CApp theApp;