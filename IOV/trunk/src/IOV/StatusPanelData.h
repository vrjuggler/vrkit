#include <IOV/StatusPanelDataPtr.h>

#include <IOV/SceneData.h>
#include <IOV/StatusPanelPlugin.h>

#include <vpr/vpr.h>
#include <vpr/Util/GUID.h>

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

      StatusPanelData();

      virtual ~StatusPanelData();

   public:
      StatusPanelPluginPtr    mStatusPanelPlugin;  /**< Pointer to the plugin (if registered). */
};

} // end inf
