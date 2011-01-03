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

#ifndef _VRKIT_SIMPLE_SLIDE_MOVE_STRATEGY_H_
#define _VRKIT_SIMPLE_SLIDE_MOVE_STRATEGY_H_

#include <string>
#include <boost/enable_shared_from_this.hpp>

#include <jccl/Config/ConfigElementPtr.h>

#include <vrkit/move/Strategy.h>


namespace vrkit
{

class SimpleSlideMoveStrategy
   : public move::Strategy
   , public boost::enable_shared_from_this<SimpleSlideMoveStrategy>
{
protected:
   SimpleSlideMoveStrategy(const plugin::Info& info);

public:
   static std::string getId()
   {
      return "SimpleSlideMove";
   }

   static move::StrategyPtr create(const plugin::Info& info)
   {
      return move::StrategyPtr(new SimpleSlideMoveStrategy(info));
   }

   virtual ~SimpleSlideMoveStrategy()
   {
      /* Do nothing. */ ;
   }

   virtual move::StrategyPtr init(ViewerPtr viewer);

   virtual void objectsGrabbed(ViewerPtr viewer,
                               const std::vector<SceneObjectPtr>& obj,
                               const gmtl::Point3f& intersectPoint,
                               const gmtl::Matrix44f& vp_M_wand);

   virtual void objectsReleased(ViewerPtr viewer,
                                const std::vector<SceneObjectPtr>& obj);

   // Note: Use the curObjPos instead of obj->getMatrix()
   virtual gmtl::Matrix44f computeMove(ViewerPtr viewer, SceneObjectPtr obj,
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
    * @throw vrkit::PluginException
    *           Thrown if the version of the given config element is too old.
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


#endif /* _VRKIT_BASIC_MOVE_STRATEGY_H_ */
