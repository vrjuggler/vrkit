// vrkit is (C) Copyright 2005-2008
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

#include <stdexcept>
#include <algorithm>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/bind.hpp>
#include <boost/assign/list_of.hpp>

#include <OpenSG/OSGConfig.h>

#include <vpr/vpr.h>
#include <vpr/System.h>
#include <vpr/Sync/Guard.h>
#include <vpr/Util/Assert.h>
#include <vpr/Util/FileUtils.h>

#include <jccl/Config/ConfigElement.h>

#include <vrkit/Scene.h>
#include <vrkit/Status.h>
#include <vrkit/Viewer.h>
#include <vrkit/Version.h>
#include <vrkit/signal/Repository.h>
#include <vrkit/mode/Component.h>
#include <vrkit/plugin/Creator.h>
#include <vrkit/plugin/Registry.h>
#include <vrkit/plugin/TypedInitRegistryEntry.h>
#include <vrkit/plugin/Info.h>
#include <vrkit/plugin/Helpers.h>

#include "ModeHarnessPlugin.h"


using namespace boost::assign;
namespace fs = boost::filesystem;

static const vrkit::plugin::Info sInfo(
   "com.infiscape", "ModeHarnessPlugin",
   list_of(VRKIT_VERSION_MAJOR)(VRKIT_VERSION_MINOR)(VRKIT_VERSION_PATCH)
);
static vrkit::plugin::Creator<vrkit::viewer::Plugin> sPluginCreator(
   boost::bind(&vrkit::ModeHarnessPlugin::create, sInfo)
);

extern "C"
{

/** @name Plug-in Entry Points */
//@{
VRKIT_PLUGIN_API(const vrkit::plugin::Info*) getPluginInfo()
{
   return &sInfo;
}

VRKIT_PLUGIN_API(void) getPluginInterfaceVersion(vpr::Uint32& majorVer,
                                                 vpr::Uint32& minorVer)
{
   majorVer = VRKIT_PLUGIN_API_MAJOR;
   minorVer = VRKIT_PLUGIN_API_MINOR;
}

VRKIT_PLUGIN_API(vrkit::plugin::CreatorBase*) getCreator()
{
   return &sPluginCreator;
}
//@}

}

namespace vrkit
{

ModeHarnessPlugin::ModeHarnessPlugin(const plugin::Info& info)
   : viewer::Plugin(info)
{
   /* Do nothing. */ ;
}

ModeHarnessPlugin::~ModeHarnessPlugin()
{
   // Disconnect all slots before we shut down the components. This is just
   // to be safe.
   std::for_each(mConnections.begin(), mConnections.end(),
                 boost::bind(&boost::signals::connection::disconnect, _1));
   mConnections.clear();

   if ( mCurComponent && mCurComponent->isActive() )
   {
      mCurComponent->deactivate(mViewer);
   }

   mNextComponent.reset();
   mCurComponent.reset();

   typedef std::map<std::string, mode::ComponentPtr>::iterator iter_type;
   for ( iter_type c = mComponents.begin(); c != mComponents.end(); ++c )
   {
      (*c).second->exit(mViewer);
   }
}

viewer::PluginPtr ModeHarnessPlugin::init(ViewerPtr viewer)
{
   mViewer = viewer;

   signal::RepositoryPtr signal_data =
      viewer->getSceneObj()->getSceneData<signal::Repository>();

   // Configuration.
   const std::string elt_type_name(getElementType());
   jccl::ConfigElementPtr cfg_elt =
      viewer->getConfiguration().getConfigElement(elt_type_name);

   if ( cfg_elt )
   {
      // Configure ourselves.
      configure(cfg_elt);
   }

   typedef std::vector<ComponentInfo>::iterator citer_type;
   for ( citer_type i = mComponentInfo.begin(); i != mComponentInfo.end(); ++i )
   {
      const std::string& name((*i).name);
      const std::string& plugin((*i).plugin);

      mode::ComponentPtr component;

      if ( mComponents.count(name) == 0 )
      {
         try
         {
            component = makeComponent(plugin, viewer);
            mComponents[name] = component;
         }
         catch (std::runtime_error& ex)
         {
            VRKIT_STATUS << "[Mode Harness] ERROR: Failed to load mode "
                         << "component '" << name << "':\n" << ex.what()
                         << std::endl;
         }
      }
      // If the named compnent is already known, reuse the existing instance.
      // XXX: Should component reuse be configurable? It matters for the case
      // of one component being activated by multiple mode activation signals.
      else
      {
         component = mComponents[name];
      }
   }

   std::vector<std::string> registered_signals;

   typedef std::vector<SignalDef>::iterator siter_type;
   for ( siter_type i = mSignalDefs.begin(); i != mSignalDefs.end(); ++i )
   {
      const std::string& sig_name((*i).name);
      const std::string& comp_name((*i).componentName);

      // Determine whether a component has already been registered to respond
      // to signal_id.
      std::vector<std::string>::iterator ei =
         std::find(registered_signals.begin(), registered_signals.end(),
                   sig_name);

      // If sig_name was not found in registered_signals, then we can go ahead
      // and hook up the mode component with sig_name.
      if ( ei == registered_signals.end() )
      {
         if ( mComponents.count(comp_name) > 0 )
         {
            mode::ComponentPtr component = mComponents[comp_name];

            VRKIT_STATUS << "[Mode Harness] Connecting mode component '"
                         << component->getDescription() << "'\n"
                         << "               to signal '" << sig_name << "'"
                         << std::endl;

            typedef boost::signal<void ()> signal_type;
            typedef signal::Container<signal_type> signal_container_type;

            // Ensure that sig_name is a known signal.
            if ( ! signal_data->hasSignal(sig_name) )
            {
               signal_data->addSignal(sig_name,
                                      signal_container_type::create());
            }

            // Connect the newly instantiated component with its signal.
            mConnections.push_back(
               signal_data->getSignal<signal_type>(sig_name)->connect(
                  boost::bind(&ModeHarnessPlugin::prepComponentSwitch,
                              this, component)
               )
            );

            registered_signals.push_back(sig_name);
         }
         else
         {
            VRKIT_STATUS << "[Mode Harness] ERROR: No component '"
                         << comp_name << "' to connect to signal '"
                         << sig_name << "'" << std::endl;
         }
      }
      else
      {
         VRKIT_STATUS << "[Mode Harness] ERROR: Component already registered "
                      << "for signal '" << sig_name << "'" << std::endl;
      }
   }

   mode::ComponentPtr default_component;

   // Set the default component.
   if ( ! mDefaultComponentName.empty() )
   {
      if ( mComponents.count(mDefaultComponentName) == 0 )
      {
         VRKIT_STATUS << "[Mode Harness] ERROR: Unknown or invalid component '"
                      << mDefaultComponentName
                      << "' used for default component!" << std::endl;
      }
      else
      {
         default_component = mComponents[mDefaultComponentName];
      }
   }
   else
   {
      VRKIT_STATUS << "[Mode Harness] WARNING: No default mode component "
                   << "has been identified!" << std::endl;
   }

   // If we have a default component, assign it mNextComponent. The default
   // component will be activated on the first pass through the application
   // frame loop when run() is invoked. This ensures that data initialization
   // has completed before the component is activated.
   if ( default_component )
   {
      mNextComponent = default_component;
   }
   // Not having a default componnt could be a bad thing, but it is not
   // considered to be a fatal error.
   else
   {
      VRKIT_STATUS << "WARNING: There is no default mode component."
                   << std::endl;
   }

   return shared_from_this();
}

void ModeHarnessPlugin::update(ViewerPtr viewer)
{
   // Sanity check.
   vprASSERT(mViewer == viewer && "Our viewer changed!");

   // If mNextComponent is not a NULL pointer, then we perform the mode
   // component switching.
   {
      vpr::Guard<vpr::Mutex> guard(mNextComponentMutex);

      if ( mNextComponent )
      {
         // NOTE: If mNextComponent == mCurComponent, the component will be
         // deactivated and reactivated. This is probably the safest behavior
         // in general.

         // If there is an active component, deactivate it.
         if ( mCurComponent )
         {
            VRKIT_STATUS << "Deactivating '"
                         << mCurComponent->getDescription() << "'"
                         << std::endl;
            mCurComponent->deactivate(viewer);
         }

         // Do the switch.
         mCurComponent  = mNextComponent;
         mNextComponent = mode::ComponentPtr();

         // If there is a new current component, activate it.
         if ( mCurComponent )
         {
            VRKIT_STATUS << "Activating '" << mCurComponent->getDescription()
                         << "'" << std::endl;

            // If activation fails, then we will not have an active component.
            if ( ! mCurComponent->activate(viewer) )
            {
               mCurComponent = mode::ComponentPtr();
            }
         }
      }
   }

   if ( mCurComponent )
   {
      mCurComponent->update(viewer);
   }
}

void ModeHarnessPlugin::configure(jccl::ConfigElementPtr elt)
{
   vprASSERT(elt->getID() == getElementType());

   const unsigned int req_cfg_version(2);

   // Check for correct version of plugin configuration.
   if ( elt->getVersion() < req_cfg_version )
   {
      std::stringstream msg;
      msg << "Configuration of ModeHarnessPlugin failed.  Required config "
          << "element version is " << req_cfg_version << ", but element '"
          << elt->getName() << "' is version " << elt->getVersion();
      throw PluginException(msg.str(), VRKIT_LOCATION);
   }

   const std::string component_path_prop("component_path");
   const std::string default_component_prop("default_component");
   const std::string component_prop("component");
   const std::string plugin_prop("plugin");
   const std::string signal_prop("signal");
   const std::string active_component_prop("active_component");

   // Set up two default search paths:
   //    1. Relative path to './plugins/mode'
   //    2. VRKIT_BASE_DIR/lib/vrkit/plugins/mode
   //
   // In all of the above cases, the 'debug' subdirectory is searched first if
   // this is a debug build (i.e., when VRKIT_DEBUG is defined and _DEBUG is
   // not).
   std::vector<std::string> component_path =
      plugin::getDefaultSearchPath("mode");

   const unsigned int num_plugin_paths(elt->getNum(component_path_prop));
   for ( unsigned int i = 0; i < num_plugin_paths; ++i )
   {
      std::string dir = elt->getProperty<std::string>(component_path_prop, i);
      component_path.push_back(vpr::replaceEnvVars(dir));
   }

   mDefaultComponentName =
      elt->getProperty<std::string>(default_component_prop);

   const unsigned int num_comps(elt->getNum(component_prop));
   for ( unsigned int i = 0; i < num_comps; ++i )
   {
      jccl::ConfigElementPtr comp_elt =
         elt->getProperty<jccl::ConfigElementPtr>(component_prop, i);
      mComponentInfo.push_back(
         ComponentInfo(comp_elt->getName(),
                       comp_elt->getProperty<std::string>(plugin_prop))
      );
   }

   const unsigned int num_signals(elt->getNum(signal_prop));
   for ( unsigned int i = 0; i < num_signals; ++i )
   {
      jccl::ConfigElementPtr signal_elt =
         elt->getProperty<jccl::ConfigElementPtr>(signal_prop, i);
      mSignalDefs.push_back(
         SignalDef(signal_elt->getName(),
                   signal_elt->getProperty<std::string>(active_component_prop))
      );
   }

   std::vector<vpr::LibraryPtr> modules = plugin::findModules(component_path);
   std::for_each(modules.begin(), modules.end(),
                 boost::bind(&ModeHarnessPlugin::registerModule, this, _1));
}

void ModeHarnessPlugin::registerModule(vpr::LibraryPtr module)
{
   mViewer->getPluginRegistry()->addEntry(
      plugin::TypedRegistryEntry<mode::Component>::create(
         module, &mode::Component::validatePluginLib
      )
   );
}

mode::ComponentPtr
ModeHarnessPlugin::makeComponent(const std::string& pluginType,
                                 ViewerPtr viewer)
{
   VRKIT_STATUS << "   Instantiating mode component '" << pluginType
                << "' ..." << std::flush;
   std::vector<AbstractPluginPtr> deps;
   AbstractPluginPtr m = viewer->getPluginRegistry()->makeInstance(pluginType,
                                                                   deps);
   mode::ComponentPtr component =
      boost::dynamic_pointer_cast<mode::Component>(m);
   vprASSERT(component.get() != NULL);
   VRKIT_STATUS << " [OK]" << std::endl;

   return component->init(viewer);
}

void ModeHarnessPlugin::prepComponentSwitch(mode::ComponentPtr newComponent)
{
   // Update mNextComponent in a thread-safe manner. The actual component
   // swapping happens in run().
   vpr::Guard<vpr::Mutex> guard(mNextComponentMutex);
   mNextComponent = newComponent;
}

}
