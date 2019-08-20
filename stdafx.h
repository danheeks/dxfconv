// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>


#include <algorithm>
#include <list>
#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <sstream>
#include <ctime>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <math.h>

//Following is required to be defined on Ubuntu with OCC 6.3.1
#ifndef HAVE_IOSTREAM
#define HAVE_IOSTREAM
#endif

#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif

#ifndef INT_MAX
#define INT_MAX 2147483647
#endif
#ifndef INT_MIN
#define INT_MIN  (-2147483647 - 1)
#endif



#include <Standard.hxx>
#include <Standard_TypeDef.hxx>

//#include <Bnd_Box.hxx>
//#include <BRepAdaptor_Curve.hxx>
//#include <BRepAdaptor_Surface.hxx>
//#include <BRepAlgoAPI_Common.hxx>
//#include <BRepAlgoAPI_Cut.hxx>
//#include <BRepAlgoAPI_Fuse.hxx>
//#include <BRepAlgo_Fuse.hxx>
//#include <BRepBndLib.hxx>
//#include <BRep_Tool.hxx>
//#include <BRepTools.hxx>
//#include <BRepTools_WireExplorer.hxx>
//#include <GCPnts_AbscissaPoint.hxx>
//#include <GC_MakeSegment.hxx>
//#include <GC_MakeArcOfCircle.hxx>
//#include <Geom_Axis1Placement.hxx>
//#include <Geom_BezierCurve.hxx>
//#include <Geom_BezierSurface.hxx>
#include <Geom_BSplineCurve.hxx>
//#include <Geom_BSplineSurface.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Line.hxx>
#include <Geom_Plane.hxx>
//#include <GeomAPI_IntCS.hxx>
#include <GeomAPI_Interpolate.hxx>
#include <GeomAPI_IntSS.hxx>
//#include <GeomAPI_PointsToBSpline.hxx>
//#include <GeomAPI_ProjectPointOnCurve.hxx>
//#include <GeomAPI_ProjectPointOnSurf.hxx>
//#include <GeomConvert_CompCurveToBSplineCurve.hxx>
//#include <GeomLProp_SLProps.hxx>
//#include <GProp_GProps.hxx>
#include <gp.hxx>
#include <gp_Ax1.hxx>
#include <gp_Circ.hxx>
//#include <gp_Cone.hxx>
//#include <gp_Cylinder.hxx>
#include <gp_Dir.hxx>
#include <gp_Elips.hxx>
#include <gp_GTrsf.hxx>
#include <gp_Lin.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
//#include <gp_Sphere.hxx>
//#include <gp_Torus.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
//#include <Handle_Geom_TrimmedCurve.hxx>
//#include <IntTools_FaceFace.hxx>
//#include "math_BFGS.hxx"
//#include "math_MultipleVarFunctionWithGradient.hxx"
//#include <Poly_Connect.hxx>
//#include <Poly_Polygon3D.hxx>
//#include <Poly_PolygonOnTriangulation.hxx>
//#include <Poly_Triangulation.hxx>
#include <Precision.hxx>
//#include <ShapeFix_Wire.hxx>
#include <Standard_ErrorHandler.hxx>
#include <Standard_Version.hxx>
//#include <StdPrs_ToolShadedShape.hxx>
//#include <STEPControl_Controller.hxx>
//#include <STEPControl_Reader.hxx>
//#include <STEPControl_Writer.hxx>
//#include <TColgp_Array1OfDir.hxx>
#if OCC_VERSION_HEX < 0x070000
#include <TColgp_Array1OfPnt.hxx>
#endif
//#include <TColgp_Array2OfPnt.hxx>
#if OCC_VERSION_HEX < 0x070000
#include <TColgp_HArray1OfPnt.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <TColStd_Array1OfReal.hxx>
#endif
//#include <TopExp.hxx>
//#include <TopExp_Explorer.hxx>
//#include <TopoDS.hxx>
//#include <TopoDS_Face.hxx>
//#include <TopoDS_Shape.hxx>
//#include <TopoDS_Solid.hxx>
//#include <TopoDS_Vertex.hxx>
//#include <TopoDS_Wire.hxx>
//#include <TopOpeBRep_FacesIntersector.hxx>
//#include <TopOpeBRepBuild_FuseFace.hxx>
//#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
//#include <TopTools_ListIteratorOfListOfShape.hxx>
//#include <TopTools_MapIteratorOfMapOfShape.hxx>
//#include <TopTools_MapOfShape.hxx>
//#include <UnitsAPI.hxx>



#include "strconv.h"
#include "Geom.h"
#include "HeeksObj.h"
#include "HeeksColor.h"
#include "Sketch.h"
#include "App.h"