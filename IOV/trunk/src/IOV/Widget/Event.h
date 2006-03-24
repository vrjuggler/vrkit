// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_WAND_EVENT_H_
#define _INF_WAND_EVENT_H_

#include <IOV/Config.h>
#include <IOV/Widget/WidgetPtr.h>

#include <gmtl/Matrix.h>
#include <vector>

namespace inf
{

class IOV_CLASS_API Event
{
public:
   typedef int ResultType;
   static const ResultType DONE = 0;
   static const ResultType CONTINUE = 1;

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
public:
   enum Type
   {
      WandEnter = 0,
      WandExit = 1,
      WandButtonPress = 2,
      WandButtonRelease = 3,
      WandMove = 4
   };

   Event(WidgetPtr src, Type type) : mSource(src), mType(type)
   {;}

   virtual ~Event();

   Type getType()
   {
      return mType;
   }

   WidgetPtr getSource()
   {
      return mSource;
   }
protected:
   WidgetPtr            mSource;
   Type                 mType;
   gmtl::Matrix44f      mWandPos;
   std::vector<int>     mButtons;
};

}

#endif

