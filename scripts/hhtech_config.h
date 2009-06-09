/****************************************************************
 * $Id: hhtech_config.h		     2007-05-24 15:48:43Z svn $ *
 *                                                              *
 * Description:							*
 *                                                              *
 * Maintainer:  ∑∂√¿ª‘(Meihui Fan)  <mhfan@hhcn.com>            *
 *                                                              *
 * CopyRight (c)  2006~2009  HHTech                             *
 *   www.hhcn.com, www.hhcn.org                                 *
 *   All rights reserved.                                       *
 *                                                              *
 * This file is free software;                                  *
 *   you are free to modify and/or redistribute it   	        *
 *   under the terms of the GNU General Public Licence (GPL).   *
 ****************************************************************/
#ifndef HHTECH_CONFIG_H
#define HHTECH_CONFIG_H

// configurable macros:

// HHTech's common stuff must be in CONFIG_HHTECH block:
#define CONFIG_HHTECH			1

// HHTech's miniPMP related stuff:
//#define CONFIG_HHTECH_CE		1
//#define CONFIG_HHTECH_MINIPMP		1

// HHTech miniPMP's specific stuff:
// e.g.: customer/manufacturer(s), product series
//#define	CONFIG_HHTECH_RAMOS		1
//#define	CONFIG_HHTECH_SMARTQ_T5		1
//#define	CONFIG_HHTECH_OPPO_S5		1

#include "local_config.h"
#endif//HHTECH_CONFIG_H
// vim:sts=4:ts=8:
