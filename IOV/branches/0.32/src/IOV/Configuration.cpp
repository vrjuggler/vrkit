// Copyright (C) Infiscape Corporation 2005-2007

#include <IOV/Configuration.h>

#include <boost/format.hpp>

namespace inf
{

void Configuration::loadConfigEltFile(const std::string& filename)
{
   bool cfg_loaded(false);
   jccl::Configuration cfg;

   if (!filename.empty())
   {
      cfg_loaded = cfg.load(filename);
   }

   if (!cfg_loaded)
   {
      std::cerr << "WARNING: Failed to load our configuration!" << std::endl;
   }
   else
   {
      mElementLists.push_back(cfg);
   }
}

jccl::ConfigElementPtr Configuration::getConfigElement(const std::string& eltName)
{
   jccl::ConfigElementPtr ret_elt;

   unsigned cur_list(0);

   // While still have more lists to check and we haven't found one
   while((cur_list < mElementLists.size()) &&
         (ret_elt.get() == NULL))
   {
      std::vector<jccl::ConfigElementPtr> found_elts;
      mElementLists[cur_list].getByType(eltName, found_elts);
      if(!found_elts.empty())
      {
         ret_elt = found_elts.front();
      }
     cur_list++;
   }

   return ret_elt;
}

std::vector<jccl::ConfigElementPtr> Configuration::getAllConfigElements(const std::string& eltName)
{
   std::vector<jccl::ConfigElementPtr> ret_elts;

   if(eltName.empty())
   {
      // Concat all of them
      for(unsigned i=0;i<mElementLists.size();i++)
      {
         std::vector<jccl::ConfigElementPtr> cur_elts = mElementLists[i].vec();
         ret_elts.insert(ret_elts.end(), cur_elts.begin(), cur_elts.end());
      }
   }
   else
   {
      for(unsigned i=0;i<mElementLists.size();i++)
      {
         std::vector<jccl::ConfigElementPtr> cur_elts;
         mElementLists[i].getByType(eltName, cur_elts);
         ret_elts.insert(ret_elts.end(), cur_elts.begin(), cur_elts.end());
      }
   }

   return ret_elts;
}

}
