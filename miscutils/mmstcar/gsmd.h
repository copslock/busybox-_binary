/****************************************************************
 * $ID: gsmd.h         Sun, 03 Aug 2008 23:14:54 +0800  mhfan $ *
 *                                                              *
 * Description:                                                 *
 *                                                              *
 * Maintainer:  ∑∂√¿ª‘(MeiHui FAN)  <mhfan@ustc.edu>            *
 *                                                              *
 * CopyRight (c)  2008~2009  M.H.Fan                            *
 *   All rights reserved.                                       *
 ****************************************************************/
#ifndef GSMD_H
#define GSMD_H

/* XXX: simple notes
 *		AT+IPR=115200;&W
 *		AT+CFUN=1,1
 *		AT+CLIP=1
 *		AT+COLP=1
 *		AT+CLVL=99
 *		AT+CMIC=0,15
 *		AT+CMIC=1,15
 *		AT+CMGF=0
 *		AT+ECHO=30500,4,10,0
 *		ATE0
 *
 * Save params: AT&W
 *
 * Repeat:	A/
 * Check SIM:	AT+CCID
 *
 * Dialing:	ATD15855199006;
 * Hangup:	ATH
 * Accept:	ATA
 *
 *		AT+CSCA="+8613800551500"
 *
 * SMS format:	AT+CMGF=1
 * Send SMS:	AT+CMGS="+8615855199006"
 *		>hello~
 *
 *		AT+CSCS=?
 *
 * List SMS:	AT+CMGL="ALL"
 * Read SMS:	AT+CMGR=2
 * Delete:	AT+CMGD=2
 * SIM card:	AT+CPIN?
 *
 * self NO:	AT+CPBS="ON"
 *		AT+CPBW=1,"13866162440"
 *		AT&W
 *		AT+CNUM
 *
 * SignalQ:	AT+CSQ
 *		AT+CREG=?
 *		AT+COPS=?
 *		AT+COPS?
 *
 * GPRS:	AT+CGREG?
 *		AT+CGACT=1,1
 * Version:	AT+CGMR
 *
 * XXX:		AT+CGCLASS="B"
 *		AT+CGDCONT=1,"IP","CMNET"
 *		AT+CIPCSGP=1,"CMNET"
 *
 *		AT+CIPSTART="TCP","218.22.26.94","5678"
 *		AT+CIPSEND
 *		>hello~
 *
 *		AT+CIPSTATUS
 *
 *		AT+CIPCLOSE
 *		AT+CIPSHUT
 *
 *		AT+CLPORT="TCP","2020"
 *		AT+CIPSERVER
 *		AT+CIFSR
 */

struct __attribute__((packed)) gprs_cpkt {
#define GPRS_CPKT_VER	1
#define GPRS_CPKT_STX	0xaa
    unsigned char stx, ver;
    char tid[16];
    unsigned char cmd, xor, buf[80];
};

enum gprs_cpkt_code {
    GPRS_REGISTER,
    GPRS_GET_GPS,
    GPRS_CPKT_ACK,

    GPRS_REPEAT_GPS = 3,

    GPRS_QUERY_VERS = 5,
    GPRS_QUERY_STATUS,
    GPRS_QUERY_SMS,
    GPRS_SET_SMS,
    GPRS_QUERY_SERVER,
    GPRS_SET_SERVER,
    GPRS_RESET,
    GPRS_FREE_ALARM,
    GPRS_SUSPEND,
    GPRS_RESUME,
    GPRS_SET_OVERS,
    GPRS_SET_REGION,
    GPRS_GUARD_ALARM,
    GPRS_POWER_ALARM,

    GPRS_START_ALARM,
    GPRS_DOOR_ALARM,
    GPRS_SHAKE_ALARM,

    GPRS_MONITORING = 21,

    GPRS_SHUT_OILC = 23,
    GPRS_OPEN_OILC,
    GPRS_SERVER_ACK,

    GPRS_OVERS_ALARM = 25,
    GPRS_REGION_ALARM
};

#define gprs_recv	gsmd_recv
#define gprs_priv	gsmd_priv

struct gsmd_priv {
    //int fd;
    //speed_t speed;
    //const char* devp;
    struct termios tio;
    char sip[16], sport[6], apn[6];
    struct gprs_cpkt spkt;
    unsigned pending;
    void* data;
};

#endif//GSMD_H
// vim:sts=4:ts=8:
