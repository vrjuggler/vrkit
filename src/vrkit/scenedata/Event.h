// vrkit is (C) Copyright 2005-2007
//    by Allen Bierbaum, Aron Bierbuam, Patrick Hartling, and Daniel Shipton
//
// This file is part of vrkit.
//
// vrkit is free software; you can redistribute it and/or modify it under the
// terms of the GNU Lesser General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// vrkit is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
// more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef _VRKIT_EVENT_H_
#define _VRKIT_EVENT_H_

namespace vrkit
{

/**
 * Provides event signaling types.
 */
namespace event
{

/**
 * The return type to be used for slots that receive emitted signals.
 */
enum ResultType
{
   DONE     = 0,     /**< Indicates that signal processing must stop */
   CONTINUE = 1      /**< Indicates that signal processing should continue */
};

/**
 * Combiner that evaluate an InputIterator sequence until it gets a return
 * value of DONE. This allows a signal to be consumed.
 */
struct ResultOperator
{
   typedef ResultType result_type;

   template<typename InputIterator>
   result_type operator()(InputIterator first, InputIterator last) const
   {
      while ( first != last )
      {
         //result_type result = *first;
         if ( DONE == *first )
         {
            return DONE;
         }
         ++first;
      }
      return DONE;
   }
};

}

}


#endif /* _VRKIT_EVENT_H_ */
