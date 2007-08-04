// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_GRID_PLUGIN_H_
#define _INF_GRID_PLUGIN_H_

#include <IOV/Plugin/PluginConfig.h>

#include <string>
#include <vector>
#include <boost/enable_shared_from_this.hpp>

#include <jccl/Config/ConfigElementPtr.h>

#include <IOV/Plugin.h>
#include <IOV/WandInterfacePtr.h>
#include <IOV/ViewerPtr.h>
#include <IOV/Util/DigitalCommand.h>
#include <IOV/Util/Exceptions.h>

#include "GridPtr.h"


namespace inf
{

class GridPlugin
   : public inf::Plugin
   , public boost::enable_shared_from_this<GridPlugin>
{
protected:
   GridPlugin(const inf::plugin::Info& info);

public:
   static inf::PluginPtr create(const inf::plugin::Info& info)
   {
      return inf::PluginPtr(new GridPlugin(info));
   }

   virtual ~GridPlugin()
   {
      /* Do nothing. */ ;
   }

   virtual std::string getDescription()
   {
      return std::string("Grid");
   }

   virtual PluginPtr init(inf::ViewerPtr viewer);

   /**
    * Updates the state of the grids based on input from the wand interface.
    *
    * @post The state of each grid is updated to reflect the digtial and/or
    *       analog input from the user via the wand interface.
    *
    * @param viewer The VR Juggler application object.
    */
   virtual void update(inf::ViewerPtr viewer);

   /**
    * Invokes the global scope delete operator.  This is required for proper
    * releasing of memory in DLLs on Win32.
    */
   void operator delete(void* p)
   {
      ::operator delete(p);
   }

protected:
   /**
    * Deletes this object.  This is an implementation of the pure virtual
    * inf::Plugin::destroy() method.
    */
   virtual void destroy()
   {
      delete this;
   }

   /**
    * Updates the state of the status panel (if it is in use) based on the
    * new focused state of this plug-in.
    *
    * @post If this plug-in has lost focus, \c mSelectedGridIndex will be -1.
    *       This means that no grid will be selected the next time that this
    *       plug-in is activated. If one or more grids are visible when focus
    *       is lost, they remain visible, but none are selected.
    *
    * @param viewer The VR Juggler application object.
    */
   void focusChanged(inf::ViewerPtr viewer);

private:
   static std::string getElementType()
   {
      return std::string("iov_grid_plugin");
   }

   /**
    * Configures this plug-in.
    *
    * @pre The type of \p elt matches the value returned by getElementType().
    *
    * @param elt The config element for this plug-in.
    *
    * @throw inf::Exception is thrown if the given config element contains
    *        bad data, is out of date, or is otherwise unusable.
    */
   void configure(jccl::ConfigElementPtr elt);

   inf::WandInterfacePtr mWandInterface;

   /** @name Grid State */
   //@{
   std::vector<inf::GridPtr> mGrids;
   int mSelectedGridIndex;
   bool mGridsVisible;
   //@}

   /** @name Grid Sliding */
   //@{
   int   mAnalogNum;    /**< Wand interface analog index for sliding */
   float mForwardVal;   /**< "Forward" analog value (0.0 or 1.0) */
   //@}

   /** @name Digital Buttons */
   //@{
   inf::DigitalCommand mActivateBtn;    /**< Activate/deactivate all button */
   inf::DigitalCommand mCycleBtn;       /**< Cycle grid selection button */
   inf::DigitalCommand mHideBtn;        /**< Show/hide grid button */
   inf::DigitalCommand mResetBtn;       /**< Reset grid position button */
   //@}

   /** @name Digital Button Command Descriptions */
   //@{
   const std::string mActivateText;     /**< Activation command description */
   const std::string mCycleText;        /**< Grid cycle command description */
   const std::string mHideText;         /**< Grid hide command description */
   const std::string mResetText;        /**< Grid reset command description */
   //@}
};

}


#endif /* _INF_GRID_PLUGIN_H_ */
