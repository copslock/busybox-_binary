/****************************************************************
 * $ID: version.h      Thu, 30 Mar 2006 13:23:19 +0800  mhfan $ *
 *                                                              *
 * Description:                                                 *
 *                                                              *
 * Maintainer:  ∑∂√¿ª‘(MeiHui FAN)  <mhfan@ustc.edu>            *
 *                                                              *
 * CopyRight (c)  2006~2009  M.H.Fan                            *
 *   All rights reserved.                                       *
 ****************************************************************/
#ifndef VERSION_H
#define VERSION_H

#define AUTHOR_EMAIL			"<mhfan@ustc.edu>"
//#define AUTHOR_EMAIL			"<meihui.fan@gmail.com>"
#define AUTHOR_NAME			"MeiHui FAN"

#define VERSION_STRING_CONCAT_REAL(major, minor, reviz) \
	"v" #major "." #minor "." #reviz

#define VERSION_STRING_CONCAT(major, minor, reviz) \
	VERSION_STRING_CONCAT_REAL(major, minor, reviz)

#if 1//def	__STDC__
#define VERSION_NUMBER  \
	(VERSION_MAJOR << 16 | VERSION_MINOR <<  8 | VERSION_REVIZ)

#define VERSION_STRING  \
	VERSION_STRING_CONCAT(VERSION_MAJOR, VERSION_MINOR, VERSION_REVIZ)

#define AUTHOR_STRING			AUTHOR_NAME " " AUTHOR_EMAIL
#else
const unsigned VERSION_NUMBER = 
	(VERSION_MAJOR << 16 | VERSION_MINOR <<  8 | VERSION_REVIZ)

const char VERSION_STRING[] =
	VERSION_STRING_CONCAT(VERSION_MAJOR, VERSION_MINOR, VERSION_REVIZ);
const char  AUTHOR_STRING[] = AUTHOR_NAME " " AUTHOR_EMAIL
#endif//__STDC__

#endif//VERSION_H
// vim:sts=4:ts=8:
