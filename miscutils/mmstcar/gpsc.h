/****************************************************************
 * $ID: gpsc.h         Sun, 03 Aug 2008 23:14:43 +0800  mhfan $ *
 *                                                              *
 * Description:                                                 *
 *                                                              *
 * Maintainer:  ∑∂√¿ª‘(MeiHui FAN)  <mhfan@ustc.edu>            *
 *                                                              *
 * CopyLeft (c)  2008  M.H.Fan                                  *
 *   All rights reserved.                                       *
 *                                                              *
 * This file is free software;                                  *
 *   you are free to modify and/or redistribute it   	        *
 *   under the terms of the GNU General Public Licence (GPL).   *
 ****************************************************************/
#ifndef GPSC_H
#define GPSC_H

#include <nmea/nmea.h>

struct gpsc_priv {
    //int fd;
    //speed_t speed;
    //const char* devp;
    struct termios tio;
    nmeaPARSER parser;
    nmeaINFO info;
    //char* nmea[];
    //void* data;
};

#endif//GPSC_H
// vim:sts=4:ts=8:
