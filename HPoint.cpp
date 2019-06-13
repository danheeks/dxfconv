// HPoint.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "HPoint.h"

static unsigned char cross16[32] = {0x80, 0x01, 0x40, 0x02, 0x20, 0x04, 0x10, 0x08, 0x08, 0x10, 0x04, 0x20, 0x02, 0x40, 0x01, 0x80, 0x01, 0x80, 0x02, 0x40, 0x04, 0x20, 0x08, 0x10, 0x10, 0x08, 0x20, 0x04, 0x40, 0x02, 0x80, 0x01};
static unsigned char cross16_selected[32] = {0xc0, 0x03, 0xe0, 0x07, 0x70, 0x0e, 0x38, 0x1c, 0x1c, 0x38, 0x0e, 0x70, 0x07, 0xe0, 0x03, 0xc0, 0x03, 0xc0, 0x07, 0xe0, 0x0e, 0x70, 0x1c, 0x38, 0x38, 0x1c, 0x70, 0x0e, 0xe0, 0x07, 0xc0, 0x03};

HPoint::~HPoint(void)
{
}

HPoint::HPoint(const gp_Pnt &p, const HeeksColor* col)
{
	m_p = p;
	color = *col;
	m_draw_unselected=true;
}

HPoint::HPoint(const HPoint &p)
{
	operator=(p);
}

const HPoint& HPoint::operator=(const HPoint &b)
{
	HeeksObj::operator =(b);

	m_p = b.m_p;
	color = b.color;
	m_draw_unselected = b.m_draw_unselected;
	return *this;
}

bool HPoint::IsDifferent(HeeksObj* o)
{
	HPoint* other = (HPoint*)o;
	if(m_p.Distance(other->m_p) > theApp.m_geom_tol)
		return true;

	return HeeksObj::IsDifferent(o);
}

void HPoint::GetBox(CBox &box)
{
	box.Insert(m_p.X(), m_p.Y(), m_p.Z());
}

HeeksObj *HPoint::MakeACopy(void)const
{
	return new HPoint(*this);
}

void HPoint::ModifyByMatrix(const double *m)
{
	gp_Trsf mat = make_matrix(m);
	m_p.Transform(mat);
}

bool HPoint::GetStartPoint(double* pos)
{
	extract(m_p, pos);
	return true;
}

bool HPoint::GetEndPoint(double* pos)
{
	extract(m_p, pos);
	return true;
}
