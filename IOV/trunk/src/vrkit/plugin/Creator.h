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

#ifndef _VRKIT_PLUGIN_CREATOR_H_
#define _VRKIT_PLUGIN_CREATOR_H_

#include <vrkit/Config.h>

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include <vrkit/plugin/CreatorBase.h>


namespace vrkit
{

namespace plugin
{

template<typename PLUGIN_TYPE>
class Creator : public CreatorBase
{
public:
   typedef boost::shared_ptr<PLUGIN_TYPE> plugin_ptr_t;
   typedef boost::function<plugin_ptr_t ()> creator_t;

   Creator(creator_t creator)
      : mCreator(creator)
   {
      /* Do nothing. */ ;
   }

   virtual ~Creator()
   {
      /* Do nothing. */ ;
   }

   plugin_ptr_t createPlugin() const
   {
      return mCreator();
   }

private:
   creator_t mCreator;
};

}

}


#endif /* _VRKIT_PLUGIN_CREATOR_H_ */
