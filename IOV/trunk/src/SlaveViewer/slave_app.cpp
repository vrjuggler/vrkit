// Copyright (C) Infiscape Corporation 2005-2007

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <exception>
#include <typeinfo>
#include <string>
#include <vector>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

#include <vrj/Kernel/Kernel.h>

#include <IOV/ExitCodes.h>
#include <IOV/Slave/SlaveViewer.h>

#define IOSV_VERSION_MAJOR 0
#define IOSV_VERSION_MINOR 1
#define IOSV_VERSION_PATCH 0


namespace po = boost::program_options;

template<typename T>
T fromString(const std::string& str, std::ios_base& (*f)(std::ios_base&))
{
   std::istringstream stream(str);
   T result;

   if ( (stream >> f >> result).fail() )
   {
      std::ostringstream msg_stream;
      msg_stream << "Failed to convert '" << str << "' to type "
                 << typeid(T).name() << std::endl;
      throw std::runtime_error(msg_stream.str());
   }

   return result;
}

int main(int argc, char* argv[])
{
   std::string master_addr;
   std::string root_name;
   std::string mask_str;

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
      ("mask,m",
       po::value<std::string>(&mask_str)->default_value("0xffffffff"),
       "Render action traversal mask in base-8, base-10, or base-16 form")
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
         return inf::EXIT_ERR_MISSING_ADDR;
      }

      vrj::Kernel* kernel  = vrj::Kernel::instance();

      if ( vm.count("jconf") == 0 )
      {
         std::cout << "No VR Juggler configuration files given!" << std::endl;
         return inf::EXIT_ERR_MISSING_JCONF;
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

      OSG::UInt32 trav_mask(0xffffffff);

      if ( ! mask_str.empty() )
      {
         try
         {
            std::ios_base& (*formatter)(std::ios_base&);

            // The mask was given as a hexadecimal value.
            if ( boost::istarts_with(mask_str, "0x") )
            {
               formatter = std::hex;
            }
            // The mask was given as an octal value.
            else if ( mask_str != std::string("0") &&
                      boost::starts_with(mask_str, "0") )
            {
               formatter = std::oct;
            }
            // The mask was given as a decimal value.
            else
            {
               formatter = std::dec;
            }

            trav_mask = fromString<OSG::UInt32>(mask_str, formatter);
         }
         catch (std::exception& ex)
         {
            std::cerr << "Could not set traversal mask from user input:\n"
                      << ex.what() << std::endl;
         }
      }

      inf::SlaveViewer* app = new inf::SlaveViewer(master_addr, root_name,
                                                   trav_mask);

      kernel->start();
      kernel->setApplication(app);
      kernel->waitForKernelStop();

      delete app;
   }
   catch (std::exception& ex)
   {
      std::cout << ex.what() << std::endl;
      std::cout << visible << std::endl;
      return inf::EXIT_ERR_EXCEPTION;
   }

   return EXIT_SUCCESS;
}
