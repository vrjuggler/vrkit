#include <IOV/StatusPanelDataPtr.h>

#include <IOV/SceneData.h>
#include <IOV/StatusPanelPlugin.h>

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
      
      virtual ~StatusPanelData()
      {
         /* Do nothing. */;
      }
      
      typedef boost::signal<void (const std::string&)> set_string_t;
      typedef boost::signal<void (const int, const std::string&)> set_int_string_t;
      typedef boost::signal<void (const std::vector<int>&, const std::string&)> set_ints_string_t;
      typedef boost::signal<void (const int, const std::string&, const unsigned int)> set_int_string_uint_t;
      typedef boost::signal<void (const std::vector<int>&, const std::string&, const unsigned int)> set_ints_string_uint_t;
      typedef boost::signal<void (const int, const std::string&, bool&)> has_int_string_t;
      typedef boost::signal<void (const std::vector<int>&, const std::string&, bool&)> has_ints_string_t;
      typedef boost::signal<void (const float, const float, const float)> set_float3_t;
      typedef boost::signal<void (const unsigned int)> set_uint_t;
      
      set_string_t mSetHeaderTitle;
      set_string_t mSetCenterTitle;
      set_string_t mSetBottomTitle;

      set_string_t mSetHeaderText;

      set_int_string_t mSetControlText;
      set_ints_string_t mSetControlTexts;

      set_int_string_uint_t mAddControlText;
      set_ints_string_uint_t mAddControlTexts;
      
      set_int_string_t mRemoveControlText;
      set_ints_string_t mRemoveControlTexts;

      has_int_string_t mHasControlText;
      has_ints_string_t mHasControlTexts;

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
