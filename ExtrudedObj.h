#pragma once

#include "HeeksObj.h"

template<typename T>
class ExtrudedObj : public T

{
public:
	double m_thickness;
	double m_extrusion_vector[3];

	~ExtrudedObj(void);
	ExtrudedObj(void)
	{
		m_thickness = 0.0;
		m_extrusion_vector[0] = 0.0;
		m_extrusion_vector[1] = 0.0;
		m_extrusion_vector[2] = 0.0;
	}
	ExtrudedObj(const ExtrudedObj& e);

	const ExtrudedObj& operator=(const ExtrudedObj &b);

	// HeeksObj's virtual functions
	void ModifyByMatrix(const double* m);
	void CopyFrom(const HeeksObj* object){ operator=(*((ExtrudedObj*)object)); }
	HeeksObj* MakeACopyWithID();
	bool IsDifferent(HeeksObj* other);
};

template<typename T>
ExtrudedObj<T>::ExtrudedObj(const ExtrudedObj& e)
{
	operator=(e);
}

template < typename T >
ExtrudedObj<T>::~ExtrudedObj(){
}

template < typename T >
const ExtrudedObj<T>& ExtrudedObj<T>::operator=(const ExtrudedObj<T> &b){
	T::operator = (b);
	m_thickness = b.m_thickness;
	m_extrusion_vector[0] = b.m_extrusion_vector[0];
	m_extrusion_vector[1] = b.m_extrusion_vector[1];
	m_extrusion_vector[2] = b.m_extrusion_vector[2];
	return *this;
}

template < typename T > HeeksObj* ExtrudedObj<T>::MakeACopyWithID()
{
	ExtrudedObj<T>* pnew = (ExtrudedObj<T>*)T::MakeACopyWithID();
	pnew->m_thickness = m_thickness;
	pnew->m_extrusion_vector[0] = m_extrusion_vector[0];
	pnew->m_extrusion_vector[1] = m_extrusion_vector[1];
	pnew->m_extrusion_vector[2] = m_extrusion_vector[2];
	return (HeeksObj*)pnew;
}

template < typename T >
bool ExtrudedObj<T>::IsDifferent(HeeksObj *other)
{
	ExtrudedObj<T>* eobj = (ExtrudedObj<T>*)other;
	if (fabs(eobj->m_thickness - m_thickness) > theApp.m_geom_tol)
		return true;

	for (int i = 0; i<3; i++)
	{
		if (fabs(eobj->m_extrusion_vector[i] - m_extrusion_vector[i]) > 0.000000000001)
			return true;
	}

	return T::IsDifferent(other);
}

template < typename T > void ExtrudedObj<T>::ModifyByMatrix(const double* m){
	gp_Trsf mat = make_matrix(m);
	gp_Vec v(m_extrusion_vector[0], m_extrusion_vector[1], m_extrusion_vector[2]);
	v.Transform(mat);
	extract(v, m_extrusion_vector);
}
