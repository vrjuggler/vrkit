// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_EVENT_H_
#define _INF_EVENT_H_


namespace inf
{

class Event
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
