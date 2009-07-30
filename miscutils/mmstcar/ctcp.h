/****************************************************************
 * $ID: ctcp.h         Sat, 26 Jul 2008 19:59:08 +0800  mhfan $ *
 *                                                              *
 * Description:                                                 *
 *                                                              *
 * Maintainer:  ∑∂√¿ª‘(Meihui Fan)  <mhfan@ustc.edu>            *
 *                                                              *
 * Copyright (C)  2008~2009  M.H.Fan                            *
 *   All rights reserved.                                       *
 ****************************************************************/
#ifndef CTCP_H
#define CTCP_H

struct ctcp_cmsg {
#define CTCP_CMSG_STX	0xaa
#define CTCP_CMSG_SDATA	20
#define CTCP_CMSG_CDATA 80
    unsigned char stx, ver, tar[16], cmd, xor, buf[CTCP_CMSG_CDATA];
};

enum ctcp_cmsg_code {
    CTCP_CMSG_NULL,
};

#endif//CTCP_H
// vim:sts=4:ts=8:
