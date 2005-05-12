#ifndef _INF_VIEWPOINTS_PLUGIN_H_
#define _INF_VIEWPOINTS_PLUGIN_H_

#include <OpenSG/VRJ/Viewer/plugins/PluginConfig.h>

#include <string>
#include <boost/enable_shared_from_this.hpp>
#include <gmtl/Matrix.h>
#include <vector>

#include <OpenSG/VRJ/Viewer/IOV/Plugin.h>
#include <OpenSG/VRJ/Viewer/IOV/WandInterfacePtr.h>

namespace inf
{

/** Plugin for moving between preset viewpoints.
 *
 */
class ViewpointsPlugin
   : public inf::Plugin, public boost::enable_shared_from_this<ViewpointsPlugin>
{
public:
   static PluginPtr create();

   virtual ~ViewpointsPlugin()
   {;}

   virtual std::string getDescription()
   {
      return std::string("Viewpoints Plugin");
   }

   virtual void init(inf::ViewerPtr viewer);

   virtual void updateState(inf::ViewerPtr viewer);

   virtual void run(inf::ViewerPtr viewer);

   virtual bool canHandleElement(jccl::ConfigElementPtr elt);

   virtual bool config(jccl::ConfigElementPtr elt);

   /**
    * Invokes the global scope delete operator.  This is required for proper
    * releasing of memory in DLLs on Win32.
    */
   void operator delete(void* p)
   {
      ::operator delete(p);
   }

protected:
   static std::string getElementType();

   /**
    * Deletes this object.  This is an implementation of the pure virtual
    * inf::Plugin::destroy() method.
    */
   virtual void destroy()
   {
      delete this;
   }

   ViewpointsPlugin()
      : mControlBtnNum(0)
      , mNextViewpoint(0)
   {;}

protected:
   struct Viewpoint
   {
      gmtl::Matrix44f   mXform;
      std::string       mName;
   };

private:
   WandInterfacePtr        mWandInterface;      /**< Ptr to the wand interface to use. */
   unsigned                mControlBtnNum;      /**< The index of the control button. */
   unsigned                mNextViewpoint;      /**< The next viewpoint to switch to. */
   std::vector<Viewpoint>  mViewpoints;         /**< The predefined viewpoints. */
};


} // namespace inf

#endif

