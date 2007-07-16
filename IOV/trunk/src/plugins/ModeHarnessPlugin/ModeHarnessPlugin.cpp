// Copyright (C) Infiscape Corporation 2005-2007

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

#include <IOV/SignalRepository.h>
#include <IOV/ModeComponent.h>
#include <IOV/PluginCreator.h>
#include <IOV/PluginRegistry.h>
#include <IOV/Status.h>
#include <IOV/TypedInitRegistryEntry.h>
#include <IOV/Viewer.h>
#include <IOV/Version.h>
#include <IOV/Plugin/Info.h>
#include <IOV/Plugin/Helpers.h>

#include "ModeHarnessPlugin.h"


using namespace boost::assign;
namespace fs = boost::filesystem;

static const inf::plugin::Info sInfo(
   "com.infiscape", "ModeHarnessPlugin",
   list_of(IOV_VERSION_MAJOR)(IOV_VERSION_MINOR)(IOV_VERSION_PATCH)
);
static inf::PluginCreator<inf::Plugin> sPluginCreator(
   boost::bind(&inf::ModeHarnessPlugin::create, sInfo)
);

extern "C"
{

/** @name Plug-in Entry Points */
//@{
IOV_PLUGIN_API(const inf::plugin::Info*) getPluginInfo()
{
   return &sInfo;
}

IOV_PLUGIN_API(void) getPluginInterfaceVersion(vpr::Uint32& majorVer,
                                               vpr::Uint32& minorVer)
{
   majorVer = INF_PLUGIN_API_MAJOR;
   minorVer = INF_PLUGIN_API_MINOR;
}

IOV_PLUGIN_API(inf::PluginCreatorBase*) getCreator()
{
   return &sPluginCreator;
}
//@}

}

namespace inf
{

ModeHarnessPlugin::ModeHarnessPlugin(const inf::plugin::Info& info)
   : Plugin(info)
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

   std::map<std::string, inf::ModeComponentPtr>::iterator c;
   for ( c = mComponents.begin(); c != mComponents.end(); ++c )
   {
      (*c).second->exit(mViewer);
   }
}

inf::PluginPtr ModeHarnessPlugin::init(inf::ViewerPtr viewer)
{
   mViewer = viewer;

   inf::SignalRepositoryPtr signal_data =
      viewer->getSceneObj()->getSceneData<SignalRepository>();

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

      inf::ModeComponentPtr component;

      if ( mComponents.count(name) == 0 )
      {
         try
         {
            component = makeComponent(plugin, viewer);
            mComponents[name] = component;
         }
         catch (std::runtime_error& ex)
         {
            IOV_STATUS << "[Mode Harness] ERROR: Failed to load mode "
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
            inf::ModeComponentPtr component = mComponents[comp_name];

            IOV_STATUS << "[Mode Harness] Connecting mode component '"
                       << component->getDescription() << "'\n"
                       << "               to signal '" << sig_name << "'"
                       << std::endl;

            typedef boost::signal<void (const std::string&)> signal_type;
            typedef SignalContainer<signal_type> signal_container_type;

            // Ensure that sig_name is a known signal.
            if ( ! signal_data->hasSignal(sig_name) )
            {
               signal_data->addSignal(sig_name,
                                      signal_container_type::create());
            }

            // Connect the newly instantiated component with its signal.
            mConnections.push_back(
               signal_data->getSignal<signal_type>(sig_name)->connect(
                  boost::bind(&inf::ModeHarnessPlugin::prepComponentSwitch,
                              this, component)
               )
            );

            registered_signals.push_back(sig_name);
         }
         else
         {
            IOV_STATUS << "[Mode Harness] ERROR: No component '" << comp_name
                       << "'to connect to signal '" << sig_name << "'"
                       << std::endl;
         }
      }
      else
      {
         IOV_STATUS << "[Mode Harness] ERROR: Component already registered "
                    << "for signal '" << sig_name << "'" << std::endl;
      }
   }

   inf::ModeComponentPtr default_component;

   // Set the default component.
   if ( ! mDefaultComponentName.empty() )
   {
      if ( mComponents.count(mDefaultComponentName) == 0 )
      {
         IOV_STATUS << "[Mode Harness] ERROR: Unknown or invalid component '"
                    << mDefaultComponentName << "' used for default component!"
                    << std::endl;
      }
      else
      {
         default_component = mComponents[mDefaultComponentName];
      }
   }
   else
   {
      IOV_STATUS << "[Mode Harness] WARNING: No default mode component "
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
      IOV_STATUS << "WARNING: There is no default mode component."
                 << std::endl;
   }

   return shared_from_this();
}

void ModeHarnessPlugin::update(inf::ViewerPtr viewer)
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
            IOV_STATUS << "Deactivating '" << mCurComponent->getDescription()
                       << "'" << std::endl;
            mCurComponent->deactivate(viewer);
         }

         // Do the switch.
         mCurComponent  = mNextComponent;
         mNextComponent = inf::ModeComponentPtr();

         // If there is a new current component, activate it.
         if ( mCurComponent )
         {
            IOV_STATUS << "Activating '" << mCurComponent->getDescription()
                       << "'" << std::endl;

            // If activation fails, then we will not have an active component.
            if ( ! mCurComponent->activate(viewer) )
            {
               mCurComponent = inf::ModeComponentPtr();
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
      throw PluginException(msg.str(), IOV_LOCATION);
   }

   const std::string component_path_prop("component_path");
   const std::string default_component_prop("default_component");
   const std::string component_prop("component");
   const std::string plugin_prop("plugin");
   const std::string signal_prop("signal");
   const std::string active_component_prop("active_component");

   // Set up two default search paths:
   //    1. Relative path to './plugins/mode'
   //    2. IOV_BASE_DIR/lib/IOV/plugins/mode
   //
   // In all of the above cases, the 'debug' subdirectory is searched first if
   // this is a debug build (i.e., when IOV_DEBUG is defined and _DEBUG is
   // not).
   std::vector<std::string> component_path =
      inf::plugin::getDefaultSearchPath("mode");

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

   std::vector<vpr::LibraryPtr> modules =
      inf::plugin::findModules(component_path);
   std::for_each(modules.begin(), modules.end(),
                 boost::bind(&ModeHarnessPlugin::registerModule, this, _1));
}

void ModeHarnessPlugin::registerModule(vpr::LibraryPtr module)
{
   mViewer->getPluginRegistry()->addEntry(
      inf::TypedRegistryEntry<inf::ModeComponent>::create(
         module, &inf::ModeComponent::validatePluginLib
      )
   );
}

inf::ModeComponentPtr
ModeHarnessPlugin::makeComponent(const std::string& pluginType,
                                 inf::ViewerPtr viewer)
{
   IOV_STATUS << "   Instantiating mode component '" << pluginType << "' ..."
              << std::flush;
   std::vector<AbstractPluginPtr> deps;
   AbstractPluginPtr m = viewer->getPluginRegistry()->makeInstance(pluginType,
                                                                   deps);
   inf::ModeComponentPtr component =
      boost::dynamic_pointer_cast<inf::ModeComponent>(m);
   vprASSERT(component.get() != NULL);
   IOV_STATUS << " [OK]" << std::endl;

   return component->init(viewer);
}

void ModeHarnessPlugin::prepComponentSwitch(inf::ModeComponentPtr newComponent)
{
   // Update mNextComponent in a thread-safe manner. The actual component
   // swapping happens in run().
   vpr::Guard<vpr::Mutex> guard(mNextComponentMutex);
   mNextComponent = newComponent;
}

}
