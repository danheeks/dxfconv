// EndedObject.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "EndedObject.h"
#include "HPoint.h"

EndedObject::EndedObject(void){
}

EndedObject::EndedObject(const EndedObject& e)
{
	operator=(e);
}

EndedObject::~EndedObject(){
}

const EndedObject& EndedObject::operator=(const EndedObject &b){
	ExtrudedObj<HeeksObj>::operator = (b);
	A = b.A;
	B = b.B;
	color = b.color;
	return *this;
}

HeeksObj* EndedObject::MakeACopyWithID()
{
	EndedObject* pnew = (EndedObject*)ExtrudedObj<HeeksObj>::MakeACopyWithID();
	return pnew;
}

bool EndedObject::IsDifferent(HeeksObj *other)
{
	EndedObject* eobj = (EndedObject*)other;
	if(eobj->A.Distance(A) > theApp.m_geom_tol)
		return true;

	if(eobj->B.Distance(B) > theApp.m_geom_tol)
		return true;

	if(color.COLORREF_color() != eobj->color.COLORREF_color())
		return true;

	return ExtrudedObj<HeeksObj>::IsDifferent(other);
}

void EndedObject::ModifyByMatrix(const double* m){
	gp_Trsf mat = make_matrix(m);
	A.Transform(mat);
	B.Transform(mat);
}

bool EndedObject::Stretch(const double *p, const double* shift, void* data){
	gp_Pnt vp = make_point(p);
	gp_Vec vshift = make_vector(shift);

	if(data == &A){
		A = vp.XYZ() + vshift.XYZ();
	}
	else if(data == &B){
		B = vp.XYZ() + vshift.XYZ();
	}
	return false;
}

bool EndedObject::GetStartPoint(double* pos)
{
	extract(A, pos);
	return true;
}

bool EndedObject::GetEndPoint(double* pos)
{
	extract(B, pos);
	return true;
}
