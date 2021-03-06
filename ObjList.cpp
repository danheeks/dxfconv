// ObjList.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "ObjList.h"
#include <algorithm>


ObjList::ObjList(const ObjList& objlist): HeeksObj(objlist), m_index_list_valid(true) {operator=(objlist);}

void ObjList::Clear()
{
	std::list<HeeksObj*>::iterator It;
	for(It=m_objects.begin(); It!=m_objects.end() ;It++)
	{
		HeeksObj* object = *It;
		(*It)->m_owner = NULL;
		if(!object->NeverDelete())delete *It;
	}
	m_objects.clear();
	m_index_list.clear();
	m_index_list_valid = true;
}

void ObjList::Clear(std::set<HeeksObj*> &to_delete)
{
	std::list<HeeksObj*>::iterator It;
	for(It=m_objects.begin(); It!=m_objects.end();)
	{
		if(to_delete.find(*It) != to_delete.end())
		{
			(*It)->m_owner = NULL;
			It = m_objects.erase(It);
		}
		else
			It++;
	}
	m_index_list.clear();
	m_index_list_valid = false;
}

const ObjList& ObjList::operator=(const ObjList& objlist)
{
	HeeksObj::operator=(objlist);
	Clear();
	std::list<HeeksObj*>::const_iterator It;
	for (It=objlist.m_objects.begin();It!=objlist.m_objects.end();It++)
	{
		if(!(*It)->OneOfAKind())
		{
			HeeksObj* new_op = (*It)->MakeACopy();
			if(new_op)Add(new_op, NULL);
		}
	}
	return *this;
}

HeeksObj* ObjList::MakeACopy(void) const { return new ObjList(*this); }

void ObjList::GetBox(CBox &box)
{
	std::list<HeeksObj*>::iterator It;
	for(It=m_objects.begin(); It!=m_objects.end() ;It++)
	{
		HeeksObj* object = *It;
		if(object->OnVisibleLayer() && object->m_visible)
		{
			object->GetBox(box);
		}
	}
}

HeeksObj* ObjList::GetFirstChild()
{
	if (m_objects.size()==0) return NULL;
	LoopIt = m_objects.begin();
	return *LoopIt;
}

HeeksObj* ObjList::GetNextChild()
{
	if (m_objects.size()==0 || LoopIt==m_objects.end()) return NULL;
	LoopIt++;
	if (LoopIt==m_objects.end()) return NULL;
	return *LoopIt;
}

void ObjList::recalculate_index_list()
{
	m_index_list.clear();
	m_index_list.resize(m_objects.size());
	int i = 0;
	for(std::list<HeeksObj*>::iterator It=m_objects.begin(); It!=m_objects.end() ;It++, i++)
	{
		HeeksObj* object = *It;
		m_index_list[i] = object;
	}
	m_index_list_valid = true;
}

HeeksObj* ObjList::GetAtIndex(int index)
{
	if(!m_index_list_valid)
	{
		recalculate_index_list();
	}

	if(index < 0 || index >= (int)(m_index_list.size()))return NULL;
	return m_index_list[index];
}

int ObjList::GetNumChildren()
{
	return m_objects.size();
}

void ObjList::Add(std::list<HeeksObj*> objects)
{
	std::list<HeeksObj*>::iterator it;
	for(it = objects.begin(); it != objects.end(); it++)
	{
		Add(*it,NULL);
	}
}

void ObjList::Remove(std::list<HeeksObj*> objects)
{
	std::list<HeeksObj*>::iterator it;
	for(it = objects.begin(); it != objects.end(); it++)
	{
		Remove(*it);
	}
}

bool ObjList::Add(HeeksObj* object, HeeksObj* prev_object)
{
	if (object==NULL) return false;
	if (!CanAdd(object)) return false;
	if (std::find(m_objects.begin(), m_objects.end(), object) != m_objects.end()) return true; // It's already here.

	if (m_objects.size()==0 || prev_object==NULL)
	{
		m_objects.push_back(object);
		LoopIt = m_objects.end();
		LoopIt--;
	}
	else
	{
		for(LoopIt = m_objects.begin(); LoopIt != m_objects.end(); LoopIt++) { if (*LoopIt==prev_object) break; }
		m_objects.insert(LoopIt, object);
	}
	m_index_list_valid = false;
	HeeksObj::Add(object, prev_object);

	//if (((!theApp.m_in_OpenFile || theApp.m_file_open_or_import_type != FileOpenTypeHeeks || theApp.m_inPaste) && object->UsesID() && (object->m_id == 0 || (theApp.m_file_open_or_import_type == FileImportTypeHeeks && theApp.m_in_OpenFile))))
	//{
	//	object->SetID(theApp.GetNextID(object->GetIDGroupType()));
	//}

	return true;
}


std::list<HeeksObj *> ObjList::GetChildren() const
{
	std::list<HeeksObj *> children;
	std::copy( m_objects.begin(), m_objects.end(), std::inserter(children, children.begin()) );
	return(children);
}

void ObjList::Remove(HeeksObj* object)
{
	if (object==NULL) return;
	for(LoopIt = m_objects.begin(); LoopIt != m_objects.end(); LoopIt++){
		if(*LoopIt==object)break;
	}
	if(LoopIt != m_objects.end())
	{
		m_objects.erase(LoopIt);
	}
	m_index_list_valid = false;
	HeeksObj::Remove(object);

	std::list<HeeksObj*> parents;
	parents.push_back(this);
	object->Disconnect(parents);

	//if( (!theApp.m_in_OpenFile || theApp.m_file_open_or_import_type != FileOpenTypeHeeks) &&
	//	object->UsesID() &&
	//	(object->m_id == 0 || (theApp.m_file_open_or_import_type == FileImportTypeHeeks && theApp.m_in_OpenFile))
	//	)
	//{
	//	theApp.RemoveID(object);
	//}
}

void ObjList::KillGLLists(void)
{
	std::list<HeeksObj*>::iterator It;
	for(It=m_objects.begin(); It!=m_objects.end() ;It++) (*It)->KillGLLists();
}

void ObjList::ModifyByMatrix(const double *m)
{
	std::list<HeeksObj*> copy_list = m_objects;
	for(std::list<HeeksObj*>::iterator It=copy_list.begin(); It!=copy_list.end() ;It++)
	{
		(*It)->ModifyByMatrix(m);
	}
}

void ObjList::GetTriangles(void(*callbackfunc)(const double* x, const double* n), double cusp, bool just_one_average_normal)
{
	for(std::list<HeeksObj*>::iterator It=m_objects.begin(); It!=m_objects.end() ;It++) (*It)->GetTriangles(callbackfunc, cusp, just_one_average_normal);
}

/**
	This is the overload for the corresponding method in the HeeksObj class.  It looks for an existing
	object anywhere in this or the child elements (or their children's children.... or their
	children's children's children....you get the idea)

	The idea is to search for pre-existing objects when we're importing a new set of data.  If none
	can be found then a NULL should be returned.
 */
HeeksObj *ObjList::Find( const int type, const unsigned int id )
{
	if ((type == this->GetType()) && (this->m_id == id)) return(this);
	for(std::list<HeeksObj*>::const_iterator It=m_objects.begin(); It!=m_objects.end() ;It++)
	{
		HeeksObj *object = (*It)->Find( type, id );
		if (object != NULL) return(object);
	} // End for
	return(NULL);
}

/* virtual */ void ObjList::SetIdPreservation(const bool flag)
{
	for(std::list<HeeksObj*>::const_iterator It=m_objects.begin(); It!=m_objects.end() ;It++)
	{
		(*It)->SetIdPreservation(flag);
	}

	HeeksObj::SetIdPreservation(flag);
}

void ObjList::ReloadPointers()
{
	for (std::list<HeeksObj*>::iterator itObject = m_objects.begin(); itObject != m_objects.end(); itObject++)
	{
		(*itObject)->ReloadPointers();
	}

	HeeksObj::ReloadPointers();
}

bool ObjList::operator==( const ObjList & rhs ) const
{
	return(true);


	if (m_objects.size() != rhs.m_objects.size()) return(false);

	std::list<const HeeksObj *> sorted_lhs;
	std::copy( m_objects.begin(), m_objects.end(), std::inserter( sorted_lhs, sorted_lhs.begin() ) );

	std::list<const HeeksObj *> sorted_rhs;
	std::copy( rhs.m_objects.begin(), rhs.m_objects.end(), std::inserter( sorted_rhs, sorted_rhs.begin() ) );

	sorted_lhs.sort();
	sorted_rhs.sort();

	std::list<const HeeksObj *> difference;

	// std::set_difference( sorted_lhs.begin(), sorted_lhs.end(), sorted_rhs.begin(), sorted_rhs.begin(), difference.begin() );
	// return(difference.size() == 0);

	return(sorted_lhs == sorted_rhs);
}

void ObjList::OnChangeViewUnits(const double units)
{
	for (std::list<HeeksObj*>::iterator itObject = m_objects.begin(); itObject != m_objects.end(); itObject++)
	{
		(*itObject)->OnChangeViewUnits(units);
	}

	HeeksObj::OnChangeViewUnits(units);
}
