// Copyright (C) Infiscape Corporation 2005

#ifndef _INF_SLAVE_EXIT_CODES_H_
#define _INF_SLAVE_EXIT_CODES_H_


namespace inf
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


#endif
