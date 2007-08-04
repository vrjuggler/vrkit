// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_BASIC_HIGHLIGHTER_PTR_H_
#define _INF_BASIC_HIGHLIGHTER_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class BasicHighlighter;
   typedef boost::shared_ptr<BasicHighlighter> BasicHighlighterPtr;
   typedef boost::weak_ptr<BasicHighlighter> BasicHighlighterWeakPtr;
}

#endif
