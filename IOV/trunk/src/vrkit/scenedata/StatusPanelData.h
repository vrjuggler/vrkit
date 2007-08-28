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

#ifndef _VRKIT_STATUS_PANEL_DATA_H_
#define _VRKIT_STATUS_PANEL_DATA_H_

#include <vrkit/Config.h>

#include <vector>
#include <string>
#include <boost/signal.hpp>

#include <vpr/vpr.h>
#include <vpr/Util/GUID.h>

#include <vrkit/SceneData.h>
#include <vrkit/scenedata/StatusPanelDataPtr.h>


namespace vrkit
{

class VRKIT_CLASS_API StatusPanelData : public SceneData
{
public:
   static const vpr::GUID type_guid;

   static StatusPanelDataPtr create()
   {
      return StatusPanelDataPtr(new StatusPanelData);
   }

   virtual ~StatusPanelData();

   typedef boost::signal<void (const std::string&)> set_string_t;
   typedef boost::signal<
      void (const std::string&, const std::string&)
   > set_cmd_string_t;
   typedef boost::signal<
      void (const std::string&, const std::string&, const unsigned int)
   > set_cmd_string_uint_t;
   typedef boost::signal<
      void (const float, const float, const float)
   > set_float3_t;
   typedef boost::signal<void (const unsigned int)> set_uint_t;

   set_string_t setHeaderTitle;
   set_string_t setCenterTitle;
   set_string_t setBottomTitle;

   set_string_t setHeaderText;

   set_cmd_string_t setControlText;

   set_cmd_string_uint_t addControlText;

   set_cmd_string_t removeControlText;

   set_string_t addStatusMessage;

   set_float3_t setWidthHeight;

   set_uint_t setStatusHistorySize;

protected:
   StatusPanelData()
   {
      /* Do nothing. */;
   }
};

} // end vrkit


#endif /* _VRKIT_STATUS_PANEL_DATA_H_ */
