// HText.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "HText.h"

HText::HText(const gp_Trsf &trsf, const std::wstring &text, const HeeksColor* col,
#ifndef WIN32
			 VectorFont *pFont,
#endif
			 int hj, int vj):m_color(*col),  m_trsf(trsf), m_text(text),
#ifndef WIN32
			 m_pFont(pFont),
#endif
			 m_h_justification(hj), m_v_justification(vj)
{
}

HText::HText(const HText &b)
{
	operator=(b);
}

HText::~HText(void)
{
}

const HText& HText::operator=(const HText &b)
{
    if (this != &b)
    {
        ObjList::operator=(b);
        m_trsf = b.m_trsf;
        m_text = b.m_text;
        m_color = b.m_color;
#ifndef WIN32
        m_pFont = b.m_pFont;
#endif
		m_v_justification = b.m_v_justification;
		m_h_justification = b.m_h_justification;
    }

	return *this;
}

bool HText::GetTextSize( const std::wstring & text, float *pWidth, float *pHeight ) const
{
	return false;
} // End GetTextSize() method

void HText::GetBoxPoints(std::list<gp_Pnt> &pnts)
{
	gp_Pnt vt(0, 0, 0);
	vt.Transform(m_trsf);

	float width, height;
	if (! GetTextSize( m_text, &width, &height ))
	{
		pnts.push_back(vt);
		return;
	}

	gp_Pnt point[4];
	point[0] = gp_Pnt(0, 0, 0);
	point[1] = gp_Pnt(width, 0, 0);
	point[2] = gp_Pnt(0, -height, 0);
	point[3] = gp_Pnt(width, -height, 0);

	double x = 0;
	double y = 0;

	switch(m_h_justification)
	{
	case 0:// Left
		break;
	case 1:// Center
		x = -width * 0.5;
		break;
	case 2:// Right
		x = -width;
		break;
	default:
		break;
	}
	switch(m_v_justification)//0 = Baseline; 1 = Bottom; 2 = Middle; 3 = Top
	{
	case 0:// Baseline
		y = height * 0.85;
		break;
	case 1:// Bottom
		y = height;
		break;
	case 2:// Middle
		y= height * 0.5;
		break;
	case 3:// Top
		break;
	default:
		break;
	}

#ifndef WIN32
	if (m_pFont != NULL)
	{
		// We're using the vector fonts.  These have the opposite meanings for the Y axis values.
            for (::size_t i=0; i<sizeof(point)/sizeof(point[0]); i++)
            {
                point[i].SetY( point[i].Y() * -1.0 );
            }
	} // End if - then
#endif

	gp_Trsf shift;
	shift.SetTranslationPart(gp_Vec(x, y, 0));

	for(int i = 0; i<4; i++)
	{
		point[i].Transform(shift);
		point[i].Transform(m_trsf);
		pnts.push_back(point[i]);
	}
}

void HText::GetBox(CBox &box)
{
	std::list<gp_Pnt> pnts;
	GetBoxPoints(pnts);
	double p[3];

	for(std::list<gp_Pnt>::iterator It = pnts.begin(); It != pnts.end(); It++)
	{
		gp_Pnt &point = *It;
		extract(point, p);
		box.Insert(p);
	}
}

HeeksObj *HText::MakeACopy(void)const
{
	return new HText(*this);
}

void HText::ModifyByMatrix(const double *m)
{
	gp_Trsf mat = make_matrix(m);
	m_trsf = mat * m_trsf;
}

bool HText::Stretch(const double *p, const double* shift, void* data)
{
	return false;
}

void HText::OnEditString(const wchar_t* str){
	m_text.assign(str);
	// to do, use undoable property changes
}

bool HText::CanAdd(HeeksObj* object)
{
	if (object == NULL) return(false);
	if (GetNumChildren() > 0)
	{
		//wxMessageBox(_("Only a single orientation modifier is supported"));
		return(false);
	}

	if (object->GetType() == OrientationModifierType) return(true);
	return(false);
}
