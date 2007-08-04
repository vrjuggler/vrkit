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
      typedef boost::signal<void (const std::string&, const std::string&)> set_cmd_string_t;
      typedef boost::signal<void (const std::string&, const std::string&, const unsigned int)> set_cmd_string_uint_t;
      typedef boost::signal<void (const std::string&, const std::string&, bool&)> has_cmd_string_t;
      typedef boost::signal<void (const float, const float, const float)> set_float3_t;
      typedef boost::signal<void (const unsigned int)> set_uint_t;

      set_string_t mSetHeaderTitle;
      set_string_t mSetCenterTitle;
      set_string_t mSetBottomTitle;

      set_string_t mSetHeaderText;

      set_cmd_string_t mSetControlText;

      set_cmd_string_uint_t mAddControlText;

      set_cmd_string_t mRemoveControlText;

      has_cmd_string_t mHasControlText;

      set_string_t mAddStatusMessage;

      set_float3_t mSetWidthHeight;

      set_uint_t mSetStatusHistorySize;

      void test()
      {
         std::cerr << "test";
      }

   protected:
      StatusPanelData()
      {
         /* Do nothing. */;
      }

};

} // end inf


#endif /* _INF_STATUS_PANEL_DATA_H_ */
