// HeeksObj.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "HeeksObj.h"
#include "ObjList.h"
#include "Sketch.h"

HeeksObj::HeeksObj(void): m_owner(NULL), m_skip_for_undo(false), m_id(0), m_layer(0), m_visible(true), m_preserving_id(false), m_index(0)
{
}

HeeksObj::HeeksObj(const HeeksObj& ho): m_owner(NULL), m_skip_for_undo(false), m_id(0), m_layer(0), m_visible(true),m_preserving_id(false), m_index(0)
{
	operator=(ho);
}

const HeeksObj& HeeksObj::operator=(const HeeksObj &ho)
{
	// don't copy the ID or the owner
	m_layer = ho.m_layer;
	m_visible = ho.m_visible;
	m_skip_for_undo = ho.m_skip_for_undo;

	if(ho.m_preserving_id)
		m_id = ho.m_id;

	return *this;
}

HeeksObj::~HeeksObj()
{
	if(m_owner)m_owner->Remove(this);

//	if (m_index) theApp.ReleaseIndex(m_index);
}

HeeksObj* HeeksObj::MakeACopyWithID()
{
	m_preserving_id = true;
	HeeksObj* ret = MakeACopy();
	m_preserving_id = false;
	return ret;
}

bool HeeksObj::GetScaleAboutMatrix(double *m)
{
	// return the bottom left corner of the box
	CBox box;
	GetBox(box);
	if(!box.m_valid)return false;
	gp_Trsf mat;
	mat.SetTranslationPart(gp_Vec(box.m_x[0], box.m_x[1], box.m_x[2]));
	extract(mat, m);
	return true;
}

bool HeeksObj::Add(HeeksObj* object, HeeksObj* prev_object)
{
	object->m_owner = this;
	object->OnAdd();
	return true;
}

void HeeksObj::OnRemove()
{
	if(m_owner == NULL)KillGLLists();
}

void HeeksObj::SetID(int id)
{
	//theApp.SetObjectID(this, id);
}

bool HeeksObj::OnVisibleLayer()
{
	// to do, support multiple layers.
	return true;
}

HeeksObj *HeeksObj::Find( const int type, const unsigned int id )
{
	if ((type == this->GetType()) && (this->m_id == id)) return(this);
	return(NULL);
}

#ifdef WIN32
#define snprintf _snprintf
#endif

void HeeksObj::ToString(char *str, unsigned int* rlen, unsigned int len)
{
	unsigned int printed;
	*rlen = 0;

	printed = snprintf(str,len,"ID: 0x%X, Type: 0x%X, MarkingMask: 0x%X, IDGroup: 0x%X\n",GetID(),GetType(),(unsigned int)GetMarkingMask(),GetIDGroupType());
	if(printed >= len)
		goto abort;
	*rlen += printed; len -= printed;

abort:
	*rlen = 0;
}

unsigned int HeeksObj::GetIndex() {
//	if (!m_index) m_index = theApp.GetIndex(this);
	return m_index;
}
