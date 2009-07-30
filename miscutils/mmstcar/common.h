/****************************************************************
 * $ID: common.h       Mon, 27 Mar 2006 21:26:41 +0800  mhfan $ *
 *                                                              *
 * Description:                                                 *
 *                                                              *
 * Maintainer:  ∑∂√¿ª‘(Meihui Fan)  <mhfan@ustc.edu>            *
 *                                                              *
 * CopyRight (c)  2006~2009  M.H.Fan                            *
 *   All rights reserved.                                       *
 ****************************************************************/
#ifndef COMMON_H
#define COMMON_H

#define	DEBUG				1
#define	SANITY_WRAP			1
#define	STREAMING_READ			1

#include "errmsg.h"

#define	MAX(a, b)			((a < b) ? b : a)
#define	MIN(a, b)			((a < b) ? a : b)

#define	NORMAL_FILE_MODE		(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define	DATA_PATH_DIR_MODE		(0755)

#define	CUBIC_BEZIER			1 // for cairo/twin render

#define BITSOF(t)	(sizeof(t) << 3)

#endif//COMMON_H
// vim:sts=4:ts=8:
