// Copyright (c) 2009 INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you may redistribute it under
// the terms of the Q Public License version 1.0.
// See the file LICENSE.QPL distributed with CGAL.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL$
// $Id$
//
//
// Author(s)     : Stephane Tayeb
//
//******************************************************************************
// File Description : 
//******************************************************************************

#ifndef CGAL_MESH_OPTIMIZATION_RETURN_CODE_H
#define CGAL_MESH_OPTIMIZATION_RETURN_CODE_H

namespace CGAL {

enum Mesh_optimization_return_code
{
  BOUND_REACHED=0,
  TIME_LIMIT_REACHED,
  CANT_IMPROVE_ANYMORE,
  CONVERGENCE_REACHED,
  MAX_ITERATION_NUMBER_REACHED
};


} //namespace CGAL

#endif // CGAL_MESH_OPTIMIZATION_RETURN_CODE_H
