#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <exception>
#include <string>
#include <vector>
#include <boost/program_options.hpp>

#include <vrj/Kernel/Kernel.h>

#include <OpenSG/VRJ/SlaveViewer/SlaveViewer.h>

#define IOSV_VERSION_MAJOR 0
#define IOSV_VERSION_MINOR 1
#define IOSV_VERSION_PATCH 0


namespace po = boost::program_options;

int main(int argc, char* argv[])
{
   const int EXIT_ERR_MISSING_JCONF(1);
   const int EXIT_ERR_MISSING_ADDR(2);
   const int EXIT_ERR_EXCEPTION(-1);

   std::string master_addr;
   std::string root_name;

   po::options_description generic("Generic options");
   generic.add_options()
      ("version,v", "print version string")
      ("help", "produce help message")
      ;

   po::options_description config("Configuration");
   config.add_options()
      ("jconf,j", po::value< std::vector<std::string> >()->composing(),
       "VR Juggler config file")
      ("addr,a", po::value<std::string>(&master_addr),
       "Master address and port number in the form address:port")
      ("root,r",
       po::value<std::string>(&root_name)->default_value("RootNode"),
       "Name of the root node being shared by the master")
      ;

   po::options_description cmdline_options;
   cmdline_options.add(generic).add(config);

   po::options_description config_file_options;
   config_file_options.add(config);

   po::options_description visible("Allowed options");
   visible.add(generic).add(config);

   try
   {
      po::variables_map vm;
      store(po::parse_command_line(argc, argv, cmdline_options), vm);

      std::ifstream cfg_file("slave_viewer.cfg");
      store(parse_config_file(cfg_file, config_file_options), vm);
      notify(vm);

      if ( vm.count("help") > 0 )
      {
         std::cout << visible << std::endl;
         return EXIT_SUCCESS;
      }

      if ( vm.count("version") > 0 )
      {
         std::cout << "Infiscape Slave Viewer Application v"
                   << IOSV_VERSION_MAJOR << "." << IOSV_VERSION_MINOR << "."
                   << IOSV_VERSION_PATCH << std::endl
                   << "\tCopyright (c) 2005 Infiscape Corporation"
                   << std::endl;
         return EXIT_SUCCESS;
      }

      if ( vm.count("addr") == 0 )
      {
         std::cout << "No address for master node given!" << std::endl;
         return EXIT_ERR_MISSING_ADDR;
      }

      vrj::Kernel* kernel  = vrj::Kernel::instance();

      if ( vm.count("jconf") == 0 )
      {
         std::cout << "No VR Juggler configuration files given!" << std::endl;
         return EXIT_ERR_MISSING_JCONF;
      }
      else
      {
         std::vector<std::string> jconfs =
            vm["jconf"].as< std::vector<std::string> >();

         std::vector<std::string>::iterator i;
         for ( i = jconfs.begin(); i != jconfs.end(); ++i )
         {
            kernel->loadConfigFile(*i);
         }
      }

      inf::SlaveViewer* app = new inf::SlaveViewer(master_addr, root_name);

      kernel->start();
      kernel->setApplication(app);
      kernel->waitForKernelStop();

      delete app;
   }
   catch (std::exception& ex)
   {
      std::cout << ex.what() << std::endl;
      std::cout << visible << std::endl;
      return EXIT_ERR_EXCEPTION;
   }

   return EXIT_SUCCESS;
}
