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

#ifndef _VRKIT_BAD_RECORDING_DATA_H_
#define _VRKIT_BAD_RECORDING_DATA_H_

#include <vrkit/Config.h>

#include <vrkit/exceptions/RecordingException.h>


namespace vrkit
{

/**
 * Exception thrown if an attempt is made to record OpenGL frames with bad
 * data.
 *
 * @see vrkit::VideoCamera
 * @see vrkit::VideoEncoder
 *
 * @since 0.45.1
 */
class VRKIT_CLASS_API BadRecordingData : public RecordingException
{
public:
   BadRecordingData(const std::string& msg, const std::string& location = "")
      throw ();

   virtual ~BadRecordingData() throw ();

   std::string getExceptionName()
   {
      return "vrkit::BadRecordingData";
   }
};

}


#endif /* _VRKIT_BAD_RECORDING_DATA_H_ */
