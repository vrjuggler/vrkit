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

#include <sstream>

#include <OpenSG/OSGSimpleGeometry.h>
#include <OpenSG/OSGSimpleMaterial.h>
#if OSG_MAJOR_VERSION < 2
#  include <OpenSG/OSGGeoPropPTypes.h>
#else
#  include <OpenSG/OSGGeoProperties.h>
#endif

#include <gmtl/Math.h>
#include <gmtl/External/OpenSGConvert.h>

#include <vpr/Util/Assert.h>
#include <jccl/Config/ConfigElement.h>

#include <vrkit/exceptions/PluginException.h>

#include "Grid.h"


namespace vrkit
{

GridPtr Grid::init(jccl::ConfigElementPtr cfgElt)
{
   vprASSERT(cfgElt->getID() == std::string("grid"));

   const unsigned int req_cfg_version(1);

   // Check for correct version of plugin configuration.
   if ( cfgElt->getVersion() < req_cfg_version )
   {
      std::ostringstream msg;
      msg << "Failed to configure grid '" << cfgElt->getName()
          << "'. Required config element version is " << req_cfg_version
          << ", but this element is version " << cfgElt->getVersion();
      throw PluginException(msg.str(), VRKIT_LOCATION);
   }

   mName = cfgElt->getName();

   const std::string size_prop("size");
   const std::string granularity_prop("granularity");
   const std::string color_prop("color");
   const std::string corner_prop("corner");
   const std::string corner_pos_prop("corner_position");
   const std::string orient_prop("orientation");

   const OSG::Real32 width  = cfgElt->getProperty<OSG::Real32>(size_prop, 0);
   const OSG::Real32 height = cfgElt->getProperty<OSG::Real32>(size_prop, 1);

   if ( width <= 0.0f || height <= 0.0f )
   {
      std::ostringstream msg_stream;
      msg_stream << "Invalid grid dimensions " << width << "x" << height;
      throw PluginException(msg_stream.str(), VRKIT_LOCATION);
   }

   const float granularity = cfgElt->getProperty<float>(granularity_prop);

   if ( granularity <= 0.0f )
   {
      std::ostringstream msg_stream;
      msg_stream << "Invalid grid cell granularity " << granularity;
      throw PluginException(msg_stream.str(), VRKIT_LOCATION);
   }

   OSG::Real32 red   = cfgElt->getProperty<OSG::Real32>(color_prop, 0);
   OSG::Real32 green = cfgElt->getProperty<OSG::Real32>(color_prop, 1);
   OSG::Real32 blue  = cfgElt->getProperty<OSG::Real32>(color_prop, 2);

   if ( red < 0.0f || red > 1.0f )
   {
      std::cerr << "WARNING: Invalid red color value " << red
                << " in config element '" << cfgElt->getName() << "'"
                << std::endl;
      red = 0.0f;
   }

   if ( green < 0.0f || green > 1.0f )
   {
      std::cerr << "WARNING: Invalid green color value " << green
                << " in config element '" << cfgElt->getName() << "'"
                << std::endl;
      green = 0.0f;
   }

   if ( blue < 0.0f || blue > 1.0f )
   {
      std::cerr << "WARNING: Invalid blue color value " << blue
                << " in config element '" << cfgElt->getName() << "'"
                << std::endl;
      blue = 0.0f;
   }

   unsigned int corner_val = cfgElt->getProperty<unsigned int>(corner_prop);
   Corner corner;

   if ( 0 <= corner_val && corner_val <= 3 )
   {
      corner = static_cast<Corner>(corner_val);
   }
   else
   {
      std::ostringstream msg_stream;
      msg_stream << "Invalid corner value " << corner_val
                 << "; must be one of 0, 1, 2, or 3";
      throw PluginException(msg_stream.str(), VRKIT_LOCATION);
   }

   const OSG::Vec3f corner_pos(
      cfgElt->getProperty<float>(corner_pos_prop, 0),
      cfgElt->getProperty<float>(corner_pos_prop, 1),
      cfgElt->getProperty<float>(corner_pos_prop, 2)
   );
   OSG::Quaternion rot;
   rot.setValue(
      gmtl::Math::deg2Rad(cfgElt->getProperty<float>(orient_prop, 0)),
      gmtl::Math::deg2Rad(cfgElt->getProperty<float>(orient_prop, 1)),
      gmtl::Math::deg2Rad(cfgElt->getProperty<float>(orient_prop, 2))
   );

   initGeometry(width, height, granularity, corner, corner_pos, rot,
                OSG::Color3f(red, green, blue));

   return shared_from_this();
}

OSG::NodeRefPtr Grid::getRoot() const
{
   return OSG::NodeRefPtr(mRoot.node());
}

bool Grid::isVisible()
{
#if OSG_MAJOR_VERSION < 2
   return mRoot.node()->getActive();
#else
   return mRoot.node()->getTravMask() != 0;
#endif
}

void Grid::setVisible(const bool visible)
{
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor re(mRoot.node(), OSG::Node::TravMaskFieldMask);
   mRoot.node()->setActive(visible);
#else
   mRoot.node()->setTravMask(visible);
#endif
}

void Grid::setSelected(const bool selected)
{
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor pne(mPlaneNode.node(), OSG::Node::TravMaskFieldMask);
   mPlaneNode.node()->setActive(selected);
#else
   mPlaneNode.node()->setTravMask(selected);
#endif
}

void Grid::move(const OSG::Matrix& xform)
{
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor rce(mRoot.core(), OSG::Transform::MatrixFieldMask);
#endif
   mRoot->setMatrix(xform);
}

void Grid::move(const gmtl::Matrix44f& xform)
{
   OSG::Matrix xform_osg;
   gmtl::set(xform_osg, xform);
   move(xform_osg);
}

void Grid::reset()
{
   move(mInitXform);
}

OSG::Matrix Grid::getCurrentXform()
{
   return mRoot->getMatrix();
}

void Grid::initGeometry(const OSG::Real32 width, const OSG::Real32 height,
                        const OSG::Real32 granularity,
                        const Grid::Corner corner,
                        const OSG::Vec3f& cornerPos,
                        const OSG::Quaternion& rot,
                        const OSG::Color3f& color)
{
   OSG::SimpleMaterialPtr plane_mat = OSG::SimpleMaterial::create();

#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor pme(plane_mat,
                     OSG::SimpleMaterial::AmbientFieldMask         |
                        OSG::SimpleMaterial::DiffuseFieldMask      |
                        OSG::SimpleMaterial::LitFieldMask          |
                        OSG::SimpleMaterial::TransparencyFieldMask);
#endif
   plane_mat->setLit(false);
   plane_mat->setAmbient(color);
   plane_mat->setDiffuse(color);
   plane_mat->setTransparency(0.90f);

   OSG::SimpleMaterialPtr line_mat =
#if OSG_MAJOR_VERSION < 2
      OSG::SimpleMaterialPtr::dcast(OSG::deepClone(plane_mat));
#else
      OSG::cast_dynamic<OSG::SimpleMaterialPtr>(OSG::deepClone(plane_mat));
#endif

#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor lme(line_mat, OSG::SimpleMaterial::TransparencyFieldMask);
#endif
   line_mat->setTransparency(0.0f);

   mPlaneNode = OSG::makePlaneGeo(width, height, 1, 1);
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor pne(mPlaneNode.core(), OSG::Geometry::MaterialFieldMask);
#endif
   mPlaneNode->setMaterial(plane_mat);

   const float half_width(width / 2.0f);
   const float half_height(height / 2.0f);

   std::cout << "Corner = " << cornerPos << std::endl;
   OSG::Vec3f center_pt;

   switch ( corner )
   {
      case LOWER_LEFT:
         center_pt[0] = cornerPos[0] + half_width;
         center_pt[1] = cornerPos[1] + half_height;
         center_pt[2] = cornerPos[2];
         break;
      case LOWER_RIGHT:
         center_pt[0] = cornerPos[0] - half_width;
         center_pt[1] = cornerPos[1] + half_height;
         center_pt[2] = cornerPos[2];
         break;
      case UPPER_RIGHT:
         center_pt[0] = cornerPos[0] - half_width;
         center_pt[1] = cornerPos[1] - half_height;
         center_pt[2] = cornerPos[2];
         break;
      case UPPER_LEFT:
         center_pt[0] = cornerPos[0] + half_width;
         center_pt[1] = cornerPos[1] - half_height;
         center_pt[2] = cornerPos[2];
         break;
   }

   std::cout << "Center = " << center_pt << std::endl;

   mRoot = OSG::Transform::create();

   OSG::Matrix xform;
   xform.setTransform(center_pt, rot, OSG::Vec3f(1.0f, 1.0f, 1.0f),
                      OSG::Quaternion(), cornerPos - center_pt);

   move(xform);

   OSG::GeoPTypesPtr type = OSG::GeoPTypesUI8::create();
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor te(type, OSG::GeoPTypesUI8::GeoPropDataFieldMask);
#endif
   type->addValue(GL_LINES);

   unsigned int vertex_count(0);

   OSG::GeoPositions3fPtr pos = OSG::GeoPositions3f::create();
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor pe(pos, OSG::GeoPositions3f::GeoPropDataFieldMask);
#endif
   for ( float i = -half_width; i <= half_width; i += granularity )
   {
      pos->addValue(OSG::Pnt3f(i, -half_height, 0.0f));
      pos->addValue(OSG::Pnt3f(i, half_height, 0.0f));
      vertex_count += 2;
   }
   for ( float i = -half_height; i <= half_height; i += granularity )
   {
      pos->addValue(OSG::Pnt3f(-half_width, i, 0.0f));
      pos->addValue(OSG::Pnt3f(half_width, i, 0.0f));
      vertex_count += 2;
   }

//   std::cout << "vertex_count = " << vertex_count << std::endl;

   OSG::GeoPLengthsPtr length = OSG::GeoPLengthsUI32::create();
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor le(length, OSG::GeoPLengthsUI32::GeoPropDataFieldMask);
#endif
   length->addValue(vertex_count);

   OSG::GeoNormals3fPtr norms = OSG::GeoNormals3f::create();
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor ne(norms, OSG::GeoNormals3f::GeoPropDataFieldMask);
#endif
   for ( unsigned int i = 0; i < vertex_count; ++i )
   {
      norms->addValue(OSG::Vec3f(0.0f, 0.0f, 1.0f));
   }

   mGridNode = OSG::Geometry::create();

#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor gnce(mGridNode.core(),
                      OSG::Geometry::TypesFieldMask        |
                         OSG::Geometry::LengthsFieldMask   |
                         OSG::Geometry::PositionsFieldMask |
                         OSG::Geometry::NormalsFieldMask   |
                         OSG::Geometry::MaterialFieldMask);
#endif
   mGridNode->setTypes(type);
   mGridNode->setLengths(length);
   mGridNode->setPositions(pos);
   mGridNode->setNormals(norms);
   mGridNode->setMaterial(line_mat);

#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor re(mRoot.node(), OSG::Node::ChildrenFieldMask);
#endif
   mRoot.node()->addChild(mGridNode);
   mRoot.node()->addChild(mPlaneNode);

   setSelected(false);
}

}
