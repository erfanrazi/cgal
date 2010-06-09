// Copyright (c) 2006  Tel-Aviv University (Israel).
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
// Author(s)     : Ron Wein   <wein@post.tau.ac.il>

#ifndef CGAL_MINKOWSKI_SUM_DECOMP_2_H
#define CGAL_MINKOWSKI_SUM_DECOMP_2_H

#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Minkowski_sum_2/Labels.h>
#include <CGAL/Minkowski_sum_2/Arr_labeled_traits_2.h>
#include <CGAL/Minkowski_sum_2/Union_of_segment_cycles_2.h>

namespace CGAL {

/*! \class
 * A class for computing the Minkowski sum of two simple polygons based on
 * their decomposition two convex sub-polygons, taking the pairwise sums and
 * computing the union of the sub-sums.
 */
template <class DecompStrategy_>
class Minkowski_sum_by_decomposition_2
{
public:
  
  typedef DecompStrategy_                             Decomposition_strategy;
  typedef typename Decomposition_strategy::Polygon_2  Polygon_2;

private:

  // Kernel types:
  typedef typename Decomposition_strategy::Kernel        Kernel;
  typedef typename Kernel::Point_2                       Point_2;
  typedef typename Kernel::Vector_2                      Vector_2;
  typedef typename Kernel::Direction_2                   Direction_2;
  
  // Kernel functors:
  typedef typename Kernel::Equal_2                       Equal_2;
  typedef typename Kernel::Compare_angle_with_x_axis_2   Compare_angle_2;
  typedef typename Kernel::Construct_translated_point_2  Translate_point_2;
  typedef typename Kernel::Construct_vector_2            Construct_vector_2;
  typedef typename Kernel::Construct_direction_2         Construct_direction_2;
  typedef typename Kernel::Orientation_2                 Compute_orientation_2;
  typedef typename Kernel::Compare_x_2                   Compare_x_2;
  typedef typename Kernel::Compare_y_2                   Compare_y_2;
  typedef typename Kernel::Compare_xy_2                  Compare_xy_2;

  // Polygon-related types:
  typedef typename Polygon_2::Vertex_circulator          Vertex_circulator;
  typedef std::list<Polygon_2>                           Polygons_list;
  typedef typename Polygons_list::iterator               Polygons_iterator;

  // Traits-related types:
  typedef Arr_segment_traits_2<Kernel>                    Segment_traits_2;
  typedef Arr_labeled_traits_2<Segment_traits_2>          Traits_2; 

  typedef typename Segment_traits_2::X_monotone_curve_2   Segment_2;
  typedef typename Traits_2::X_monotone_curve_2           Labeled_segment_2;
  typedef std::list<Labeled_segment_2>                    Segments_list;

  typedef Union_of_segment_cycles_2<Traits_2, Polygon_2>  Union_2;

  // Data members:
  Equal_2                 f_equal;
  Compare_angle_2         f_compare_angle;
  Translate_point_2       f_add;
  Construct_vector_2      f_vector;
  Construct_direction_2   f_direction;
  Compute_orientation_2   f_orientation;
  Compare_x_2             f_compare_x;
  Compare_y_2             f_compare_y;
  Compare_xy_2            f_compare_xy;

public:

  /*! Default constructor. */
  Minkowski_sum_by_decomposition_2 ()
  {
    // Obtain kernel functors.
    Kernel                ker;

    f_equal = ker.equal_2_object();
    f_compare_angle = ker.compare_angle_with_x_axis_2_object();
    f_add = ker.construct_translated_point_2_object(); 
    f_vector = ker.construct_vector_2_object();
    f_direction = ker.construct_direction_2_object();
    f_orientation = ker.orientation_2_object();
    f_compare_x = ker.compare_x_2_object();
    f_compare_y = ker.compare_y_2_object();
    f_compare_xy = ker.compare_xy_2_object();
  }

  /*!
   * Compute the Minkowski sum of two simple polygons.
   * \param pgn1 The first polygon.
   * \param pgn2 The second polygon.
   * \param sum_bound Output: A polygon respresenting the outer boundary
   *                          of the Minkowski sum.
   * \param sum_holes Output: An output iterator for the holes in the sum,
   *                          represented as simple polygons.
   * \pre Both input polygons are simple.
   * \return A past-the-end iterator for the holes in the sum.
   */
  template <class OutputIterator>
  OutputIterator operator() (const Polygon_2& pgn1,
                             const Polygon_2& pgn2,
                             Polygon_2& sum_bound,
                             OutputIterator sum_holes) const
  {
    CGAL_precondition (pgn1.is_simple());
    CGAL_precondition (pgn2.is_simple());

    // Decompose both input polygons to convex sub-polygons.
    Decomposition_strategy  decomp_strat;
    Polygons_list           sub_pgns1;
    Polygons_list           sub_pgns2;

    decomp_strat (pgn1, std::back_inserter(sub_pgns1));
    decomp_strat (pgn2, std::back_inserter(sub_pgns2));

    // Compute the sub-sums of all pairs of sub-polygons.
    Polygons_iterator       end1 = sub_pgns1.end();
    Polygons_iterator       end2 = sub_pgns2.end();
    unsigned int            sum_index = 0;
    Polygons_iterator       curr1, curr2;
    Segments_list           boundary_segments;

    for (curr1 = sub_pgns1.begin(); curr1 != end1; ++curr1)
    {
      for (curr2 = sub_pgns2.begin(); curr2 != end2; ++curr2)
      {
        // Compute the sum of the current pair of convex sub-polygons.
	sum_index++;
        _compute_sum_of_convex (sum_index,
				*curr1,
                                *curr2,
                                std::back_inserter (boundary_segments));
      }
    }

    // Compute the union of the polygons that represent the Minkowski sums
    // of all sub-polygon pairs.
    Union_2     unite;

    sum_holes = unite (boundary_segments.begin(), boundary_segments.end(),
                       sum_bound, sum_holes);

    return (sum_holes);
  }

private:

  /*!
   * Compute the Minkowski sum of two convex polygons.
   * \param sum_index The index of the sub-sum.
   * \param pgn1 The first convex polygon.
   * \param pgn2 The second convex polygon.
   * \param soi Output: An output iterator for the labeled segments along
   *                    the counterclockwise-oriented Minkowski sum.
   * \return A past-the-end iterator for the output segments.
   */
  template <class SegmentsOutputIter>
  SegmentsOutputIter _compute_sum_of_convex (unsigned int sum_index,
					     const Polygon_2& pgn1,
                                             const Polygon_2& pgn2,
                                             SegmentsOutputIter soi) const
  {
    // Find the bottom-left vertex in both polygons.
    Vertex_circulator         first1, curr1, next1;
    Vertex_circulator         bottom_left1;
    Comparison_result         res;
    
    bottom_left1 = curr1 = first1 = pgn1.vertices_circulator();
    ++curr1;
    while (curr1 != first1)
    {
      res = f_compare_y (*curr1, *bottom_left1);
      if (res == SMALLER ||
          (res == EQUAL && f_compare_x (*curr1, *bottom_left1) == SMALLER))
      {
        bottom_left1 = curr1;
      }
      ++curr1;
    }

    // Find the bottom-left vertex in both polygons.
    Vertex_circulator         first2, curr2, next2;
    Vertex_circulator         bottom_left2;
    
    bottom_left2 = curr2 = first2 = pgn2.vertices_circulator();
    ++curr2;
    while (curr2 != first2)
    {
      res = f_compare_y (*curr2, *bottom_left2);
      if (res == SMALLER ||
          (res == EQUAL && f_compare_x (*curr2, *bottom_left2) == SMALLER))
      {
        bottom_left2 = curr2;
      }
      ++curr2;
    }

    // Start from the bottom-left vertices.
    next1 = curr1 = bottom_left1;
    ++next1;
    next2 = curr2 = bottom_left2;
    ++next2;
 
    // Compute the Minkowski sum.
    unsigned int              seg_index = 0;
    Point_2                   first_pt;
    Point_2                   curr_pt;
    Point_2                   prev_pt;
    bool                      is_first = true;
    bool                      inc1, inc2;
    bool                      moved_on1 = false;
    bool                      moved_on2 = false;

    do
    {
      // Compute the sum of the two current vertices (we actually translate
      // the point curr1 by a vector equivalent to the point curr2).
      curr_pt = f_add (*curr1, f_vector(CGAL::ORIGIN, *curr2));

      if (is_first)
      {
        // This is the first point we have computed.
        first_pt = prev_pt = curr_pt;
        is_first = false;
      }
      else
      {
        // Add a segment from the previously summed point to the current one.
        res = f_compare_xy (prev_pt, curr_pt);
        CGAL_assertion (res != EQUAL);
        *soi = Labeled_segment_2 (Segment_2 (prev_pt, curr_pt),
                                  X_curve_label ((res == SMALLER),
						 sum_index,
						 seg_index, false)); 
        ++soi;
        seg_index++;
        prev_pt = curr_pt;
      }

      // Compare the angles the current edges form with the x-axis.
      res = f_compare_angle (f_direction (f_vector (*curr1, *next1)), 
                             f_direction (f_vector (*curr2, *next2)));

      // Proceed to the next vertex according to the result.
      inc1 = (res != LARGER);
      inc2 = (res != SMALLER);

      if (inc1 && moved_on1 && curr1 == bottom_left1)
      {
        inc1 = false;
        inc2 = true;
      }

      if (inc2 && moved_on2 && curr2 == bottom_left2)
      {
        inc1 = true;
        inc2 = false;
      }

      if (inc1)
      {
        curr1 = next1;
        ++next1;
        moved_on1 = true;
      }      
      
      if (inc2)
      {
        curr2 = next2;
        ++next2;
         moved_on2 = true;
     }

    } while (curr1 != bottom_left1 || curr2 != bottom_left2);

    // Add the final segment to close the outer boundary.
    res = f_compare_xy (prev_pt, first_pt);
    CGAL_assertion (res != EQUAL);
    *soi = Labeled_segment_2 (Segment_2 (prev_pt, first_pt),
                              X_curve_label ((res == SMALLER),
					     sum_index,
					     seg_index, true)); 
    ++soi;

    return (soi);
  }
};

} //namespace CGAL

#endif
