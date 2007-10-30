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

#ifndef _VRKIT_SLAVE_EXIT_CODES_H_
#define _VRKIT_SLAVE_EXIT_CODES_H_


namespace vrkit
{

/** @name Command Line Processing Exit Codes */
//@{
const int EXIT_ERR_MISSING_JCONF(1);
const int EXIT_ERR_MISSING_ADDR(2);
const int EXIT_ERR_EXCEPTION(-1);
//@}

/** @name Run-Time Error Exit Codes */
//@{
const int EXIT_ERR_CONNECT_FAIL(3);
const int EXIT_ERR_COMM(4);
//@}

}


#endif /* _VRKIT_SLAVE_EXIT_CODES_H_ */
