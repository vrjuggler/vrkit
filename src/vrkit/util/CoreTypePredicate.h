// vrkit is (C) Copyright 2005-2011
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

#ifndef _VRKIT_UTIL_CORE_TYPE_PREDICATE_H_
#define _VRKIT_UTIL_CORE_TYPE_PREDICATE_H_

#include <vrkit/Config.h>

#include <vector>

#include <OpenSG/OSGNode.h>


namespace vrkit
{

namespace util
{

/** \class CoreTypePredicate CoreTypePredicate.h vrkit/util/CoreTypePredicate.h
 *
 * A predicate designed for use with vrkit::DynamicSceneObject but that is
 * surely usable in other contexts. This predicate identifies node
 * relationships based on the type of the node core. The recognized core types
 * are provided to the constructor of this class in an STL vector.
 *
 * In general, the types must be queried from the OpenSG Field Container
 * Factory (OSG::FieldContainerFactory) at run time. This means that this
 * class is most suitable for cases when the types to be queried are \em not
 * known at compile time. For example, this predicate could be used in
 * conjuction with configurable code that gets its types of interest at run
 * time. In the case when the types of interest are known statically at
 * compile time, use vrkit::util::CoreTypeSeqPredicate<T> instead.
 *
 * @see vrkit::DynamicSceneObject
 * @see vrkit::util::CoreTypeSeqPredicate
 *
 * @note This class was moved into the vrkit::util namespace in version 0.47.
 *
 * @since 0.46.0
 */
class VRKIT_CLASS_API CoreTypePredicate
{
public:
   /**
    * Constructs a predicate that can identify nodes with cores of the given
    * types.
    *
    * @param coreTypes The field container types for node cores that are used
    *                  to identify OpenSG scene graph nodes of interest.
    *
    * @post \c mCoreTypes contains all OSG::FieldContainerType pointers in
    *       \p coreTypes that are for node cores.
    *
    * @note The awkwardness of using OSG::FieldContainerType* instead of
    *       OSG::FieldContainerType is due to the fact that
    *       OSG::FieldContainerType::operator= is private. That prevents it
    *       from getting along well with std::vector.
    */
   CoreTypePredicate(const std::vector<OSG::FieldContainerType*>& coreTypes);

   /**
    * Indicates whether the given node is of interest. "Interest" is
    * determined in this case by examining the type of the core of the given
    * node. If it is one of the field container types 
    */
   bool operator()(OSG::NodePtr node);

private:
   std::vector<OSG::FieldContainerType*> mCoreTypes;
};

}

}


#endif /* _VRKIT_UTIL_CORE_TYPE_PREDICATE_H_ */
