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
#include <vector>
#include <boost/version.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <vpr/vpr.h>
#include <vpr/System.h>
#include <vpr/Util/FileUtils.h>

#include <vrkit/Exception.h>
#include <vrkit/Status.h>
#include <vrkit/plugin/Data.h>
#include <vrkit/plugin/DataFactory.h>


namespace fs = boost::filesystem;

namespace vrkit
{

namespace plugin
{

const vpr::GUID DataFactory::type_guid(
   "1B0B4334-809C-4FEF-8D21-92376AF541C7"
);

DataFactory::DataFactory()
   : vrkit::SceneData()
{
#if defined(VPR_OS_Windows)
   const std::string split_chars(";");
#else
   const std::string split_chars(":");
#endif

   std::string path_str;
   vpr::System::getenv("VRKIT_DATA_TYPE_PATH", path_str);

   if ( path_str.empty() )
   {
      path_str =
         vpr::replaceEnvVars("${VRKIT_DATA_DIR}/plugin/datatypes");
   }

   std::vector<std::string> path_dirs;
   boost::split(path_dirs, path_str, boost::is_any_of(split_chars));

   std::vector<std::string> file_list;
   fs::directory_iterator end_itr;
   const std::string ext(".vdata");

   // Load types.
   typedef std::vector<std::string>::iterator iter_type;
   for ( iter_type dir = path_dirs.begin(); dir != path_dirs.end(); ++dir )
   {
      for ( fs::directory_iterator file(*dir); file != end_itr; ++file )
      {
#if BOOST_VERSION < 103400
         // Ignore directories.  Normal files and symlinks are fine.
         if ( ! fs::is_directory(*file) )
         {
            // Construct a substring of file->leaf() that contains only the
            // file extension. We require that the file we will match have
            // names that end with ext.
            const std::string::size_type pos =
               file->leaf().size() - ext.size();
            const std::string file_ext = file->leaf().substr(pos);

            if ( file_ext == ext )
            {
               file_list.push_back(file->native_file_string());
            }
         }
#else
         // Ignore directories.  Normal files and symlinks are fine.
         if ( ! fs::is_directory(file->status()) )
         {
            // Construct a substring of file->path.leaf() that contains only
            // the file extension. We require that the file we will match have
            // names that end with ext.
            const std::string::size_type pos =
               file->path().leaf().size() - ext.size();
            const std::string file_ext = file->path().leaf().substr(pos);

            if ( file_ext == ext )
            {
               file_list.push_back(file->path().native_file_string());
            }
         }
#endif
      }
   }

   cppdom::ContextPtr ctx(new cppdom::Context());

   for ( iter_type f = file_list.begin(); f != file_list.end(); ++f )
   {
      VRKIT_STATUS << "Loading plug-in data type declaration from " << *f
                   << std::endl;
      cppdom::DocumentPtr doc(new cppdom::Document(ctx));
      doc->loadFile(*f);
      registerType(doc->getChild("data"));
   }
}

void DataFactory::registerType(const std::string& typeDecl)
{
   std::istringstream xml_stream(typeDecl);
   cppdom::ContextPtr ctx(new cppdom::Context());
   cppdom::DocumentPtr doc(new cppdom::Document(ctx));
   doc->load(xml_stream, ctx);

   registerType(doc->getChild("data"));
}

DataPtr DataFactory::createInstance(const vpr::GUID& typeID,
                                    const std::string& objName)
{
   if ( mTypes.count(typeID) == 0 )
   {
      std::ostringstream msg_stream;
      msg_stream << "Unknown type identifier " << typeID.toString();
      throw vrkit::Exception(msg_stream.str(), VRKIT_LOCATION);
   }

   return Data::create((*mTypes.find(typeID)).second, objName);
}

DataPtr DataFactory::findInstance(const vpr::GUID& typeID,
                                  const std::string& objName) const
{
   return Data::sInstanceStore.getByName(typeID, objName);
}

}

}
