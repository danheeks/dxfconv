// HLine.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "EndedObject.h"

class HLine: public EndedObject{
public:
	~HLine(void);
	HLine(const gp_Pnt &a, const gp_Pnt &b, const HeeksColor* col);
	HLine(const HLine &line);

	const HLine& operator=(const HLine &b);

	// HeeksObj's virtual functions
	int GetType()const{return LineType;}
	long GetMarkingMask()const{return MARKING_FILTER_LINE;}
	void GetBox(CBox &box);
	const wchar_t* GetTypeString(void)const{return L"Line";}
	HeeksObj *MakeACopy(void)const;
	bool GetMidPoint(double* pos);
	bool FindNearPoint(const double* ray_start, const double* ray_direction, double *point);
	bool FindPossTangentPoint(const double* ray_start, const double* ray_direction, double *point);
	void GetSegments(void(*callbackfunc)(const double *p), double pixels_per_mm, bool want_start_point = true)const;
	int Intersects(const HeeksObj *object, std::list< double > *rl)const;
	void CopyFrom(const HeeksObj* object){operator=(*((HLine*)object));}

	bool UsesID(){return true;} 
	gp_Lin GetLine()const;
	bool Intersects(const gp_Pnt &pnt)const;
	gp_Vec GetSegmentVector(double fraction);
	void Reverse();
};
