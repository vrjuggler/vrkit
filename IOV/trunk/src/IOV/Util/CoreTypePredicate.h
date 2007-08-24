// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _IOV_UTIL_CORE_TYPE_PREDICATE_H_
#define _IOV_UTIL_CORE_TYPE_PREDICATE_H_

#include <IOV/Config.h>

#include <vector>

#include <OpenSG/OSGNode.h>


namespace inf
{

/**
 * A predicate designed for use with inf::DynamicSceneObject but surely usable
 * in other contexts. This predicate identifies node relationships based on
 * the type of the node core. The recognized core types are provided to the
 * constructor of this class in an STL vector.
 *
 * In general, the types must be queried from the OpenSG Field Container
 * Factory (OSG::FieldContainerFactory) at run time. This means that this
 * class is most suitable for cases when the types to be queried are \em not
 * known at compile time. For example, this predicate could be used in
 * conjuction with configurable code that gets its types of interest at run
 * time. In the case when the types of interest are known statically at
 * compile time, use inf::CoreTypeSeqPredicate<T> instead.
 *
 * @see inf::DynamicSceneObject
 * @see inf::CoreTypeSeqPredicate
 *
 * @since 0.46.0
 */
class IOV_CLASS_API CoreTypePredicate
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


#endif /* _IOV_UTIL_CORE_TYPE_PREDICATE_H_ */
