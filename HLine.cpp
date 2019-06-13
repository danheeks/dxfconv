// HLine.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"

#include "HLine.h"
#include "HILine.h"
#include "HCircle.h"
#include "HArc.h"
#include "Sketch.h"
#include "HPoint.h"

HLine::HLine(const HLine &line):EndedObject(){
	operator=(line);
}

HLine::HLine(const gp_Pnt &a, const gp_Pnt &b, const HeeksColor* col):EndedObject(){
	A = a;
	B = b;
	SetColor(*col);
}

HLine::~HLine(){
}

const HLine& HLine::operator=(const HLine &b){
	EndedObject::operator=(b);
	return *this;
}

HeeksObj *HLine::MakeACopy(void)const{
	HLine *new_object = new HLine(*this);
	return new_object;
}

bool HLine::GetMidPoint(double* pos)
{
	extract((A.XYZ() + B.XYZ()) / 2, pos);
	return true;
}

void HLine::GetBox(CBox &box){
	box.Insert(A.X(), A.Y(), A.Z());
	box.Insert(B.X(), B.Y(), B.Z());
}

bool HLine::FindNearPoint(const double* ray_start, const double* ray_direction, double *point){
	// The OpenCascade libraries throw an exception when one tries to
	// create a gp_Lin() object using a vector that doesn't point
	// anywhere.  If this is a zero-length line then we're in
	// trouble.  Don't bother with it.
	if ((A.X() == B.X()) &&
	    (A.Y() == B.Y()) &&
	    (A.Z() == B.Z())) return(false);

	gp_Lin ray(make_point(ray_start), make_vector(ray_direction));
	gp_Pnt p1, p2;
	ClosestPointsOnLines(GetLine(), ray, p1, p2);

	if(!Intersects(p1))
		return false;

	extract(p1, point);
	return true;
}

bool HLine::FindPossTangentPoint(const double* ray_start, const double* ray_direction, double *point){
	// any point on this line is a possible tangent point
	return FindNearPoint(ray_start, ray_direction, point);
}

gp_Lin HLine::GetLine()const{
	gp_Vec v(A, B);
	return gp_Lin(A, v);
}

int HLine::Intersects(const HeeksObj *object, std::list< double > *rl)const{
	int numi = 0;

	switch(object->GetType())
	{
    case SketchType:
        return( ((CSketch *)object)->Intersects( this, rl ));

	case LineType:
		{
			// The OpenCascade libraries throw an exception when one tries to
			// create a gp_Lin() object using a vector that doesn't point
			// anywhere.  If this is a zero-length line then we're in
			// trouble.  Don't bother with it.
			if ((A.X() == B.X()) &&
			    (A.Y() == B.Y()) &&
			    (A.Z() == B.Z())) break;

			gp_Pnt pnt;
			if(intersect(GetLine(), ((HLine*)object)->GetLine(), pnt))
			{
				if(Intersects(pnt) && ((HLine*)object)->Intersects(pnt)){
					if(rl)add_pnt_to_doubles(pnt, *rl);
					numi++;
				}
			}
		}
		break;

	case ILineType:
		{
			gp_Pnt pnt;
			if(intersect(GetLine(), ((HILine*)object)->GetLine(), pnt))
			{
				if(Intersects(pnt)){
					if(rl)add_pnt_to_doubles(pnt, *rl);
					numi++;
				}
			}
		}
		break;

	case ArcType:
		{
			std::list<gp_Pnt> plist;
			intersect(GetLine(), ((HArc*)object)->GetCircle(), plist);
			for(std::list<gp_Pnt>::iterator It = plist.begin(); It != plist.end(); It++)
			{
				gp_Pnt& pnt = *It;
				if(Intersects(pnt) && ((HArc*)object)->Intersects(pnt))
				{
					if(rl)add_pnt_to_doubles(pnt, *rl);
					numi++;
				}
			}
		}
		break;

	case CircleType:
		{
			std::list<gp_Pnt> plist;
			intersect(GetLine(), ((HCircle*)object)->GetCircle(), plist);
			for(std::list<gp_Pnt>::iterator It = plist.begin(); It != plist.end(); It++)
			{
				gp_Pnt& pnt = *It;
				if(Intersects(pnt))
				{
					if(rl)add_pnt_to_doubles(pnt, *rl);
					numi++;
				}
			}
		}
		break;
	}

	return numi;
}

bool HLine::Intersects(const gp_Pnt &pnt)const
{
	gp_Lin this_line = GetLine();
	if(!intersect(pnt, this_line))return false;

	// check it lies between A and B
	gp_Vec v = this_line.Direction();
	double dpA = gp_Vec(A.XYZ()) * v;
	double dpB = gp_Vec(B.XYZ()) * v;
	double dp = gp_Vec(pnt.XYZ()) * v;
	return dp >= dpA - theApp.m_geom_tol && dp <= dpB + theApp.m_geom_tol;
}

void HLine::GetSegments(void(*callbackfunc)(const double *p), double pixels_per_mm, bool want_start_point)const{
	if(want_start_point)
	{
		double p[3];
		extract(A, p);
		(*callbackfunc)(p);
	}

	double p[3];
	extract(B, p);
	(*callbackfunc)(p);
}

gp_Vec HLine::GetSegmentVector(double fraction)
{
	gp_Vec line_vector(A, B);
	if(line_vector.Magnitude() < 0.000000001)return gp_Vec(0, 0, 0);
	return gp_Vec(A, B).Normalized();
}

void HLine::Reverse()
{
	gp_Pnt temp = A;
	A = B;
	B = temp;
}

