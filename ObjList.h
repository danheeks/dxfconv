// ObjList.h
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#pragma once

#include "HeeksObj.h"

#include <list>
#include <vector>
#include <set>

class ObjList : public HeeksObj
{
	friend class ReorderTool;

public:
	std::list<HeeksObj*> m_objects;
protected:
	std::list<HeeksObj*>::iterator LoopIt;
	std::list<std::list<HeeksObj*>::iterator> LoopItStack;
	std::vector<HeeksObj*> m_index_list; // for quick performance of GetAtIndex();
	bool m_index_list_valid;

	void recalculate_index_list();

public:
	ObjList():m_index_list_valid(true){}
	ObjList(const ObjList& objlist);
	virtual ~ObjList(){}

	const ObjList& operator=(const ObjList& objlist);

	bool operator==( const ObjList & rhs ) const;
	bool operator!=( const ObjList & rhs ) const { return(! (*this == rhs)); }
	bool IsDifferent(HeeksObj *other) { return( *this != (*(ObjList *)other) ); }

	void ClearUndoably(void);
	void Clear(std::set<HeeksObj*> &to_delete);

	HeeksObj* MakeACopy(void) const;
	void GetBox(CBox &box);
	HeeksObj* GetFirstChild();
	HeeksObj* GetNextChild();
	HeeksObj* GetAtIndex(int index);
	int GetNumChildren();
	std::list<HeeksObj *> GetChildren() const;
	bool CanAdd(HeeksObj* object){return true;}
	virtual bool Add(HeeksObj* object, HeeksObj* prev_object);
	virtual void Add(std::list<HeeksObj*> objects);
	virtual void Remove(HeeksObj* object);
	virtual void Remove(std::list<HeeksObj*> objects);
	void Clear();
	void KillGLLists(void);
	void ModifyByMatrix(const double *m);
	void GetTriangles(void(*callbackfunc)(const double* x, const double* n), double cusp, bool just_one_average_normal = true);
	bool IsList(){return true;}
	void ReloadPointers();
	void OnChangeViewUnits(const double units);

	HeeksObj *Find( const int type, const unsigned int id );	// Search for an object by type/id from this or any child objects.
	/* virtual */ void SetIdPreservation(const bool flag);
};

