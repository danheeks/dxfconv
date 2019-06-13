// HPoint.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "IdNamedObj.h"
#include "HeeksColor.h"

class HPoint: public IdNamedObj{
private:
	HeeksColor color;

public:
	gp_Pnt m_p;
	bool m_draw_unselected;
	double mx,my;

	~HPoint(void);
	HPoint(const gp_Pnt &p, const HeeksColor* col);
	HPoint(const HPoint &p);

	const HPoint& operator=(const HPoint &b);

	// HeeksObj's virtual functions
	int GetType()const{return PointType;}
	long GetMarkingMask()const{return MARKING_FILTER_POINT;}
	void GetBox(CBox &box);
	const wchar_t* GetTypeString(void)const{return L"Point";}
	HeeksObj *MakeACopy(void)const;
	void ModifyByMatrix(const double *mat);
	void SetColor(const HeeksColor &col){color = col;}
	const HeeksColor* GetColor()const{return &color;}
	bool GetStartPoint(double* pos);
	bool GetEndPoint(double* pos);
	void CopyFrom(const HeeksObj* object){operator=(*((HPoint*)object));}
	bool IsDifferent(HeeksObj* other);
};
