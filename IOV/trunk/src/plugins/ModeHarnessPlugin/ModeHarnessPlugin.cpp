// Copyright (C) Infiscape Corporation 2005-2007

#include <stdexcept>
#include <algorithm>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/bind.hpp>

#include <OpenSG/OSGConfig.h>

#include <vpr/vpr.h>
#include <vpr/System.h>
#include <vpr/Sync/Guard.h>
#include <vpr/Util/Assert.h>
#include <vpr/Util/FileUtils.h>

#include <jccl/Config/ConfigElement.h>

#include <IOV/SignalRepository.h>
#include <IOV/ModeComponent.h>
#include <IOV/PluginFactory.h>
#include <IOV/Status.h>
#include <IOV/Viewer.h>

#include "ModeHarnessPlugin.h"


namespace fs = boost::filesystem;

static inf::PluginCreator<inf::Plugin> sPluginCreator(
   &inf::ModeHarnessPlugin::create, "Mode Harness Plug-in"
);

extern "C"
{

/** @name Plug-in Entry Points */
//@{
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

typedef boost::signal<void (const std::string&)> signal_type;
typedef SignalContainer<signal_type> signal_container_type;
typedef signal_container_type::ptr_type signal_container_ptr;

ModeHarnessPlugin::ModeHarnessPlugin()
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

   mPluginFactory = viewer->getPluginFactory();
   mPluginFactory->addScanPath(mComponentPath);

   std::vector<std::string> registered_signals;

   std::vector<ComponentInfo>::iterator i;
   for ( i = mComponentInfo.begin(); i != mComponentInfo.end(); ++i )
   {
      const std::string& name((*i).name);
      const std::string& signal_id((*i).signalID);

      // Determine whether a component has already been registered to respond
      // to signal_id.
      std::vector<std::string>::iterator ei =
         std::find(registered_signals.begin(), registered_signals.end(),
                   signal_id);

      // If signal_id was not found in registered_signals, then we can go
      // ahead and hook up the mode component with signal_id.
      if ( ei == registered_signals.end() )
      {
         inf::ModeComponentPtr component;

         if ( mComponents.count(name) == 0 )
         {
            try
            {
               component = loadComponent(name, viewer);
               mComponents[name] = component;
            }
            catch (std::runtime_error& ex)
            {
               IOV_STATUS << "[Mode Harness] ERROR: Failed to load mode "
                          << "component '" << name << "':\n" << ex.what()
                          << std::endl;
            }
         }
         // If the named compnent is already known, reuse the existing
         // instance.
         // XXX: Should component reuse be configurable? It matters for
         // the case of one component being activated by multiple mode
         // activation signals.
         else
         {
            component = mComponents[name];
         }

         if ( component )
         {
            IOV_STATUS << "[Mode Harness] Connecting mode component '"
                       << component->getDescription() << "'\n"
                       << "               (from " << name << ") to signal '"
                       << signal_id << "'" << std::endl;

            // Ensure that signal_id is a known signal.
            if ( ! signal_data->hasSignal(signal_id) )
            {
               signal_data->addSignal(signal_id,
                                      signal_container_type::create());
            }

            // Connect the newly instantiated component with its signal.
            mConnections.push_back(
               signal_data->getSignal<signal_type>(signal_id)->connect(
                  boost::bind(&inf::ModeHarnessPlugin::prepComponentSwitch,
                              this, component)
               )
            );

            registered_signals.push_back(signal_id);
         }
         else
         {
            IOV_STATUS << "[Mode Harness] ERROR: No component to connect to "
                       << "signal '" << signal_id << "'" << std::endl;
         }
      }
      else
      {
         IOV_STATUS << "[Mode Harness] ERROR: Component already registered "
                    << "for signal '" << signal_id << "'" << std::endl;
      }
   }

   inf::ModeComponentPtr default_component;

   // Set the default component.
   if ( ! mDefaultComponentName.empty() )
   {
      if ( mComponents.count(mDefaultComponentName) == 0 )
      {
         try
         {
            inf::ModeComponentPtr component =
               loadComponent(mDefaultComponentName, viewer);
            mComponents[mDefaultComponentName] = component;
            default_component = component;
         }
         catch (std::runtime_error& ex)
         {
            IOV_STATUS << "[Mode Harness] ERROR: Failed to load default "
                       << "mode component '" << mDefaultComponentName
                       << "':\n" << ex.what() << std::endl;
         }
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

   const std::string component_path_prop("component_path");
   const std::string default_component_prop("default_component");
   const std::string component_prop("component");
   const std::string component_name_prop("component_name");
   const std::string signal_id_prop("signal_id");

   // Set up two default search paths:
   //    1. Relative path to './plugins/mode'
   //    2. IOV_BASE_DIR/lib/IOV/plugins/mode
   mComponentPath.push_back("plugins/mode");

   std::string iov_base_dir;
   vpr::System::getenv("IOV_BASE_DIR", iov_base_dir);

   if ( ! iov_base_dir.empty() )
   {
      fs::path iov_base_path(iov_base_dir, fs::native);
      fs::path def_component_path = iov_base_path / "lib/IOV/plugins/mode";

      if ( fs::exists(def_component_path) )
      {
         std::string def_search_path =
            def_component_path.native_directory_string();
         std::cout << "Setting default IOV mode component path: "
                   << def_search_path << std::endl;
         mComponentPath.push_back(def_search_path);
      }
      else
      {
         std::cerr << "Default IOV mode component path does not exist: "
                   << def_component_path.native_directory_string()
                   << std::endl;
      }
   }

   const unsigned int num_plugin_paths(elt->getNum(component_path_prop));
   for ( unsigned int i = 0; i < num_plugin_paths; ++i )
   {
      std::string dir = elt->getProperty<std::string>(component_path_prop, i);
      mComponentPath.push_back(vpr::replaceEnvVars(dir));
   }

   mDefaultComponentName =
      elt->getProperty<std::string>(default_component_prop);

   const unsigned int num_comps(elt->getNum(component_prop));
   for ( unsigned int i = 0; i < num_comps; ++i )
   {
      jccl::ConfigElementPtr comp_elt =
         elt->getProperty<jccl::ConfigElementPtr>(component_prop, i);
      mComponentInfo.push_back(
         ComponentInfo(comp_elt->getProperty<std::string>(component_name_prop),
                       comp_elt->getProperty<std::string>(signal_id_prop))
      );
   }
}

inf::ModeComponentPtr
ModeHarnessPlugin::loadComponent(const std::string& name,
                                 inf::ViewerPtr viewer)
{
   inf::ModeComponentPtr component;

   IOV_STATUS << "   Loading mode component '" << name << "' ..."
              << std::flush;
   inf::PluginCreator<inf::ModeComponent>* creator(
      mPluginFactory->getPluginCreator<inf::ModeComponent>(name)
   );

   if ( NULL != creator )
   {
      component = creator->createPlugin()->init(viewer);
   }
   else
   {
      std::ostringstream msg_stream;
      msg_stream << "Failed to get creator for mode component '" << name
                 << "'";
      throw inf::Exception(msg_stream.str(), IOV_LOCATION);
   }

   return component;
}

void ModeHarnessPlugin::prepComponentSwitch(inf::ModeComponentPtr newComponent)
{
   // Update mNextComponent in a thread-safe manner. The actual component
   // swapping happens in run().
   vpr::Guard<vpr::Mutex> guard(mNextComponentMutex);
   mNextComponent = newComponent;
}

}
