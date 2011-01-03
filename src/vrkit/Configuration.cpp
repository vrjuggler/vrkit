// vrkit is (C) Copyright 2005-2011
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

#include <vrkit/Config.h>

#include <sstream>

#include <vrkit/Configuration.h>
#include <vrkit/exceptions/ConfigurationException.h>


namespace vrkit
{

void Configuration::loadConfigEltFile(const std::string& filename)
{
   bool cfg_loaded(false);
   jccl::Configuration cfg;

   if ( ! filename.empty() )
   {
      cfg_loaded = cfg.load(filename);
   }

   if ( ! cfg_loaded )
   {
      std::ostringstream msg_stream;
      msg_stream << "WARNING: Failed to load '" << filename << "'!";
      throw ConfigurationException(msg_stream.str(), VRKIT_LOCATION);
   }
   else
   {
      mElementLists.push_back(cfg);
   }
}

jccl::ConfigElementPtr
Configuration::getConfigElement(const std::string& eltName)
{
   jccl::ConfigElementPtr ret_elt;

   unsigned int cur_list(0);

   // While still have more lists to check and we haven't found one
   while ( cur_list < mElementLists.size()  && ret_elt.get() == NULL )
   {
      std::vector<jccl::ConfigElementPtr> found_elts;
      mElementLists[cur_list].getByType(eltName, found_elts);

      if ( ! found_elts.empty() )
      {
         ret_elt = found_elts.front();
      }

      ++cur_list;
   }

   return ret_elt;
}

std::vector<jccl::ConfigElementPtr>
Configuration::getAllConfigElements(const std::string& eltName)
{
   std::vector<jccl::ConfigElementPtr> ret_elts;

   if ( eltName.empty() )
   {
      // Concat all of them
      for ( unsigned int i = 0; i < mElementLists.size(); ++i )
      {
         const std::vector<jccl::ConfigElementPtr>& cur_elts =
            mElementLists[i].vec();
         ret_elts.insert(ret_elts.end(), cur_elts.begin(), cur_elts.end());
      }
   }
   else
   {
      for ( unsigned int i = 0; i < mElementLists.size(); ++i )
      {
         std::vector<jccl::ConfigElementPtr> cur_elts;
         mElementLists[i].getByType(eltName, cur_elts);
         ret_elts.insert(ret_elts.end(), cur_elts.begin(), cur_elts.end());
      }
   }

   return ret_elts;
}

}
