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

#ifndef _VRKIT_RECORDING_CONFIG_ERROR_H_
#define _VRKIT_RECORDING_CONFIG_ERROR_H_

#include <vrkit/Config.h>

#include <vrkit/exceptions/RecordingException.h>


namespace vrkit
{

/** \class RecordingConfigError RecordingConfigError.h vrkit/exceptions/RecordingConfigError.h
 *
 * Exception thrown if an attempt is made to record OpenGL frames with an
 * invalid encoder configuration.
 *
 * @see vrkit::VideoCamera
 * @see vrkit::VideoEncoder
 *
 * @since 0.45.1
 */
class VRKIT_CLASS_API RecordingConfigError : public RecordingException
{
public:
   RecordingConfigError(const std::string& msg,
                        const std::string& location = "") throw ();

   virtual ~RecordingConfigError() throw ();

   std::string getExceptionName()
   {
      return "vrkit::RecordingConfigError";
   }
};

}


#endif /* _VRKIT_RECORDING_CONFIG_ERROR_H_ */
