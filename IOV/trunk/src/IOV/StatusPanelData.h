// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_STATUS_PANEL_DATA_H_
#define _INF_STATUS_PANEL_DATA_H_

#include <IOV/Config.h>
#include <IOV/StatusPanelDataPtr.h>
#include <IOV/SceneData.h>

#include <vpr/vpr.h>
#include <vpr/Util/GUID.h>

#include <vector>
#include <string>

#include <boost/signal.hpp>


namespace inf
{

class IOV_CLASS_API StatusPanelData : public inf::SceneData
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
      bool (const std::string&, const std::string&)
   > has_cmd_string_t;
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

   has_cmd_string_t hasControlText;

   set_string_t addStatusMessage;

   set_float3_t setWidthHeight;

   set_uint_t setStatusHistorySize;

protected:
   StatusPanelData()
   {
      /* Do nothing. */;
   }
};

} // end inf


#endif /* _INF_STATUS_PANEL_DATA_H_ */
