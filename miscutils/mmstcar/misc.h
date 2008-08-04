/****************************************************************
 * $ID: misc.h         Tue, 02 Sep 2008 17:31:14 +0800  mhfan $ *
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
#ifndef MISC_H
#define MISC_H

enum alarm_type {
    ALARM_NONE,
    ALARM_GUARD,
    ALARM_OVERSPEED,
    ALARM_POWER,
    ALARM_REGION,

    ALARM_START,
    ALARM_DOOR,
    ALARM_SHAKE,
};

struct misc_priv {
    unsigned short hcnt, hint;
    char mn[16];    struct timeval gtime;
    unsigned guard:1, power:1, acc:1, gprs:1, alarm:4, acnt:9, gcnt:5, door:1;
    struct { struct {	float lon, lat; } tl, br; } rg;
    struct { unsigned inte, maxt, cnts, intv; } gpsr;
    struct { float os;	unsigned short dl, ct; } oa;
};

#endif//MISC_H
// vim:sts=4:ts=8:
