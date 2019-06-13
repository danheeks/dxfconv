// HILine.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "EndedObject.h"

class HILine: public EndedObject{
public:
	~HILine(void);
	HILine(const gp_Pnt &a, const gp_Pnt &b, const HeeksColor* col);
	HILine(const HILine &line);

	const HILine& operator=(const HILine &b);

	// HeeksObj's virtual functions
	int GetType()const{return ILineType;}
	long GetMarkingMask()const{return MARKING_FILTER_ILINE;}
	void GetBox(CBox &box);
	const wchar_t* GetTypeString(void)const{return L"Infinite Line";}
	HeeksObj *MakeACopy(void)const;
	bool FindNearPoint(const double* ray_start, const double* ray_direction, double *point);
	bool FindPossTangentPoint(const double* ray_start, const double* ray_direction, double *point);
	int Intersects(const HeeksObj *object, std::list< double > *rl)const;
	void CopyFrom(const HeeksObj* object){operator=(*((HILine*)object));}
	bool GetEndPoint(double* pos);
	bool GetStartPoint(double* pos);

	gp_Lin GetLine()const;
};
