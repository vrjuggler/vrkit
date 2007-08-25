// vrkit is (C) Copyright 2005-2007
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

#ifndef _INF_SIMPLE_SLIDE_MOVE_STRATEGY_H_
#define _INF_SIMPLE_SLIDE_MOVE_STRATEGY_H_

#include <string>
#include <boost/enable_shared_from_this.hpp>

#include <jccl/Config/ConfigElementPtr.h>

#include <IOV/Grab/MoveStrategy.h>


namespace inf
{

class SimpleSlideMoveStrategy
   : public inf::MoveStrategy
   , public boost::enable_shared_from_this<SimpleSlideMoveStrategy>
{
protected:
   SimpleSlideMoveStrategy(const inf::plugin::Info& info);

public:
   static std::string getId()
   {
      return "SimpleSlideMove";
   }

   static inf::MoveStrategyPtr create(const inf::plugin::Info& info)
   {
      return inf::MoveStrategyPtr(new SimpleSlideMoveStrategy(info));
   }

   virtual ~SimpleSlideMoveStrategy()
   {
      /* Do nothing. */ ;
   }

   virtual inf::MoveStrategyPtr init(inf::ViewerPtr viewer);

   virtual void objectsGrabbed(inf::ViewerPtr viewer,
                               const std::vector<SceneObjectPtr>& obj,
                               const gmtl::Point3f& intersectPoint,
                               const gmtl::Matrix44f& vp_M_wand);

   virtual void objectsReleased(inf::ViewerPtr viewer,
                                const std::vector<SceneObjectPtr>& obj);

   // Note: Use the curObjPos instead of obj->getMatrix()
   virtual gmtl::Matrix44f computeMove(inf::ViewerPtr viewer,
                                       SceneObjectPtr obj,
                                       const gmtl::Matrix44f& vp_M_wand,
                                       const gmtl::Matrix44f& curObjPos);

private:
   static std::string getElementType()
   {
      return "simple_slide_move_strategy";
   }

   /**
    * Configures this move strategy.
    *
    * @pre The type of the given config element matches the identifier
    *      returned by getElementType().
    *
    * @param cfgElt The config element to use for configuring this object.
    *
    * @throw inf::PluginException is thrown if the version of the given
    *        config element is too old.
    */
   void configure(jccl::ConfigElementPtr cfgElt);

   /** @name Slide Behavior Properties */
   //@{
   float mTransValue;           /**< Accumulated translation */
   int   mAnalogNum;            /**< The wand interface analog index */
   float mForwardValue;         /**< Value for foroward sliing (0 or 1) */
   float mSlideEpsilon;         /**< The slide start threshold */
   float mSlideMultiplier;      /**< Slide acceleration multiplier */
   //@}
};

}


#endif /* _INF_BASIC_MOVE_STRATEGY_H_ */
