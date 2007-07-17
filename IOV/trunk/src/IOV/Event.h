// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_EVENT_H_
#define _INF_EVENT_H_

namespace inf
{

/**
 * Provides event signaling types.
 */
class IOV_CLASS_API Event
{
public:
   /**
    * The return type to be used for slots that receive emitted signals.
    */
   enum ResultType
   {
      DONE     = 0,     /**< Indicates that signal processing must stop */
      CONTINUE = 1      /**< Indicates that signal processing should continue */
   };

   /**
    * Combiner that evaluate an InputIterator sequence until it gets a
    * return value of DONE. This allows a signal to be consumed.
    */
   struct ResultOperator
   {
      typedef ResultType result_type;

      template<typename InputIterator>
      result_type operator()(InputIterator first, InputIterator last) const
      {
         while (first != last)
         {
            //result_type result = *first;
            if (DONE == *first)
            {
               return DONE;
            }
            ++first;
         }
         return DONE;
      }
   };
};

}


#endif
