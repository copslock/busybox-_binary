/****************************************************************
 * $ID: hscp.h         Fri, 25 Jul 2008 23:00:11 +0800  mhfan $ *
 *                                                              *
 * Description:                                                 *
 *                                                              *
 * Maintainer:  ∑∂√¿ª‘(Meihui Fan)  <mhfan@ustc.edu>            *
 *                                                              *
 * CopyRight (c)  2008~2009  M.H.Fan                            *
 *   All rights reserved.                                       *
 ****************************************************************/
#ifndef HSCP_H
#define HSCP_H	"Hand Set Control Protocol"

struct __attribute__((packed)) hscp_cmsg {
    unsigned char stx;
#if 1
#define HSCP_BAUD_RATE	19200	// 8n1
#define HSCP_CMSG_STX	0x24	// '$'
#define HSCP_CMSG_CEND	0x0a
#define HSCP_CMSG_SEND	0xff
    unsigned char cmd, len, buf[UCHAR_MAX - 3/* - 2*/];
    // NOTE: sum = buf[len - 2], end = buf[len - 1];
#else// XXX: TongDu
#define HSCP_BAUD_RATE	9600	// 8n1
#define HSCP_CMSG_STX	0xaa
    unsigned char len, buf[UCHAR_MAX - 2 + 1];
    // NOTE: cmd = buf[0], sum = buf[len];
#endif
};

typedef unsigned char bcd;

struct __attribute__((packed)) hscp_selftest {
    unsigned char gprs, gsm, gps, host;
};

struct __attribute__((packed)) hscp_gps_status {
    struct __attribute__((packed)) { bcd hour, min, sec; } time;

    bcd lat[4], lon[4], speed[2], dir[2];
    
    unsigned char gps;
    union __attribute__((packed)) {
	unsigned char _;
	struct __attribute__((packed)) {
	    unsigned char gprs:1, type:2, X:1, net:3, alert:1;
	};
    };

    unsigned char overspeed;
    unsigned char reserved, dummy;
    unsigned char csq, watch, acc;

    struct __attribute__((packed)) { bcd year, mon, day; } date;
};

enum hscp_cmsg_code {
    HSCP_CMSG_NULL,

    SEND_SMS_SERVER,
    SEND_SMS_MOBILE,
    SET_SERVER_IP,
    SET_SELF_PN,

    SEND_CMD_DIAL = 0x07,
    SEND_CMD_HANG,

    SEND_HANDSET_VER = 0x0a,
    SET_APN,
    SEND_CMD_SELFTEST,
    SEND_ACK,
    SEND_CMD_ACCEPT,
    HANDSET_FREE,
    SET_USER_PN,

    REQUEST_HOST = 0x11,
    HSCP_SET_GUARD,
    HSCP_RELEASE_GUARD,

    SEND_GPS_STATUS = 0xa0,

    SEND_SMS_FAILED = 0xa3,
    SEND_SMS_SUCCEED,
    SEND_NEW_CALL,
    SEND_HANG,
    SEND_ACCEPTED,
    SEND_SELFTEST = 0xa8,

    SEND_HOST_STATUS = 0xae,
    SEND_NEW_SMS_PDU,
    SEND_NEW_SMS_TXT,
    SET_SUCCESS,
};

struct hscp_priv {
    //int fd;
    //speed_t speed;
    //const char* devp;
    struct termios tio;
    struct hscp_cmsg smsg;
    unsigned pending;
#ifdef	EV_H__
    ev_timer* repeat;
#endif
    void* data;
};

#endif//HSCP_H
// vim:sts=4:ts=8:
