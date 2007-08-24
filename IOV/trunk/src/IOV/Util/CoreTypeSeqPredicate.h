// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _IOV_UTIL_CORE_TYPE_SEQ_PREDICATE_H_
#define _IOV_UTIL_CORE_TYPE_SEQ_PREDICATE_H_

#include <IOV/Config.h>

#include <boost/mpl/for_each.hpp>

#include <OpenSG/OSGNodeCore.h>
#include <OpenSG/OSGFieldContainerType.h>


namespace inf
{

/**
 * A predicate designed for use with inf::DynamicSceneObject but surely usable
 * in other contexts. This predicate identifies node relationships based on
 * the type of the node core. The recognized core types are provided in a
 * Boost MPL type sequence as the template parameter of this class. This means
 * that this class only suitable for cases when the types or interest are
 * known at compile time. In the case when the types of interest are not known
 * statically at compile time, use inf::CoreTypePredicate instead.
 *
 * @see inf::DynamicSceneObject
 * @see inf::CoreTypePredicate
 *
 * @since 0.46.1
 */
template<typename CoreTypeSeqT>
class CoreTypeSeqPredicate
{
public:
   /**
    * The sequence of field container types. This must be a Boost MPL sequence
    * (vector, list, etc.) containing subclasses of OSG::FieldContainer. More
    * abstractly, each type must have a public static method named
    * getClassType() that returns a reference to an OSG::FieldContainerType
    * object.
    */
   typedef CoreTypeSeqT type_seq_type;

private:
   /**
    * This exists solely to allow OpenSG field container types (which do not
    * have a public constructor or destructor) to be used in the type
    * sequence that is used for iteration in the call to
    * boost::mpl::for_each<>().
    */
   template<typename T>
   struct wrap { };

   /**
    * Performs a run-time comparision between two or more field container
    * types to determine if the type of interest derives from the other type.
    */
   struct Comparator
   {
      /**
       * @note \p result is passed in by reference and stored as a reference
       *       type because boost::mpl::for_each<>() takes its paramter by
       *       copy. Thus, to be able to retrieve the result of the sequence
       *       iteration, we have to use a reference (or a pointer) type.
       */
      Comparator(OSG::NodeCorePtr core, bool& result)
         : mCoreType(core->getType())
         , mResult(result)
      {
         // Ensure that we start with the correct state.
         mResult = false;
      }

      /**
       * This is what gets invoked at run time by boost::mpl::for_each<>().
       * An object of type \c wrap<CoreType> is passed in, and we ignore it
       * because what we really care about is \c CoreType and its static
       * getClassType() method.
       */
      template<typename CoreType>
      void operator()(wrap<CoreType>)
      {
         // Accumulate the result by leveraging the nature of the Boolean OR
         // operation and logical operation short cirtcuiting.
         mResult =
            mResult || mCoreType.isDerivedFrom(CoreType::getClassType());
      }

      OSG::FieldContainerType& mCoreType;
      bool& mResult;
   };

public:
   /**
    * This is what gets invoked at run time as the predicate interface. An
    * OSG::NodePtr is provided, and we check its core type against the
    * sequence of types provide as the template paramter to this class.
    */
   bool operator()(OSG::NodePtr node)
   {
      using namespace boost::mpl;

      bool result(false);
      Comparator c(node->getCore(), result);
      boost::mpl::for_each<type_seq_type, wrap<_1> >(c);

      return result;
   }
};

}


#endif /* _IOV_UTIL_CORE_TYPE_SEQ_PREDICATE_H_ */
