//#!/usr/bin/tcc -run
/****************************************************************
 * $ID: mmstcar.c      Thu, 31 Jul 2008 09:28:21 +0800  mhfan $ *
 *                                                              *
 * Description: The main control program for Handset,		*
 *		GSM/GPRS, GPS, BTNI and LEDO modules.		*
 *                                                              *
 * Maintainer:  范美辉(MeiHui FAN)  <mhfan@ustc.edu>            *
 *                                                              *
 * CopyRight (c)  2008~2009  M.H.Fan                            *
 *   All rights reserved.                                       *
 ****************************************************************/

//kbuild:lib-y :=
//kbuild:lib-$(CONFIG_MMSTCAR) += mmstcar.o
//kbuild:EXTRA_CFLAGS += -Wno-unused -Wstrict-aliasing=2
//kbuild:
//config:config MMSTCAR
//config:       bool "mmstcar"
//config:       default n
//config:       help
//config:         Multi Media Smart Terminal in Car, by mhfan
//config:
//applet:IF_MMSTCAR(APPLET(mmstcar, _BB_DIR_USR_BIN, _BB_SUID_DROP))
//applet:
//usage:#define mmstcar_trivial_usage
//usage:       "[-g DEVICE:SPEED] [-p DEVICE:SPEED] "
//usage:       "[-s DEVICE:SPEED] [-i IP:PORT]"
//usage:#define mmstcar_full_usage "\n\n"
//usage:       "Daemon for Multi Media Smart Terminal in Car\n"
//usage:     "\nOptions:"
//usage:     "\n       -g      specify DEVICE and SPEED for GSM/GPRS"
//usage:     "\n       -s      specify DEVICE and SPEED for Handset"
//usage:     "\n       -p      specify DEVICE and SPEED for GPS"
//usage:     "\n       -i      specify IP and PORT for server"
//usage:

//#include <ev.h>
#ifdef	CONFIG_PROPERTY
#include "property.h"
//#include "errmsg.h"
#else// XXX:
#include <libbb.h>
#endif//CONFIG_PROPERTY

#include "hscp.h"
#include "gpsc.h"
#include "gsmd.h"
#include "btni.h"
#include "misc.h"

//#include "bench.h"

#define PACKAGE_NAME	"Multi Media Smart Terminal in Car (MMSTCar)"
#define VERSION_MAJOR	0
#define VERSION_MINOR	1
#define VERSION_REVIZ	0
#include "version.h"


void btni_recv(int fd, struct btni_priv* priv);
void gpsc_recv(int fd, struct gpsc_priv* priv);
void gsmd_recv(int fd, struct gsmd_priv* priv);
void stdi_recv(int fd, struct gsmd_priv* priv);

void gprs_conn(int fd, struct gprs_priv* priv);
void gprs_set_gpsi(unsigned char* buf, nmeaINFO* info);
void gprs_send_alarm(int fd, struct gprs_priv* priv, unsigned cmd);
void gprs_send(int fd, struct gprs_priv* priv, struct gprs_cpkt* cpkt);

void hscp_recv(int fd, struct hscp_priv* priv);
void hscp_send(int fd, struct hscp_priv* priv, struct hscp_cmsg* smsg);
void hscp_selftest_status(int fd, struct hscp_priv* priv,
	struct hscp_cmsg* smsg);
void hscp_update_status(int fd, struct hscp_priv* priv,
	nmeaINFO* info, struct hscp_cmsg* smsg);
int  hscp_resend(struct hscp_priv* priv);

void send_alarm_sms(int fd, char* mn, const char* ucs);

unsigned char checksum_xor(unsigned char* buf, unsigned char len);

int raw_seriel_init(const char* path, speed_t speed, struct termios* tio);

#undef  inline
#define inline	inline __attribute__((gnu_inline))

inline unsigned short bcd2bin(unsigned char  val);
inline unsigned char  bin2bcd(unsigned short val);


int poll_time = -1;
struct gsmd_priv gsmd_priv;
struct hscp_priv hscp_priv;
struct gpsc_priv gpsc_priv;
struct misc_priv misc_priv;

volatile smallint signalled = 0;
static void signal_handler(int signo) { signalled = signo; }

char leds_path[] = "/sys/class/leds/led-2/brightness";

#ifdef	EV_H__
struct ev_loop* evlp = NULL;

static void hscp_callback(EV_P_ struct ev_io* w, int revents)
{
    hscp_recv(w->fd, (struct hscp_priv*)w->data);
}

static void gpsc_callback(EV_P_ struct ev_io* w, int revents)
{
    gpsc_recv(w->fd, (struct gpsc_priv*)w->data);
}

static void gsmd_callback(EV_P_ struct ev_io* w, int revents)
{
    gsmd_recv(w->fd, (struct gsmd_priv*)w->data);
}

static void btni_callback(EV_P_ struct ev_io* w, int revents)
{
    btni_recv(w->fd, (struct btni_priv*)w->data);
}

static void stdi_callback(EV_P_ struct ev_io* w, int revents)
{
    //ev_io_stop(EV_A_ w);
    stdi_recv(w->fd, &gsmd_priv);
    //ev_unloop(EV_A_ EVUNLOOP_ALL);
}

static void timer_callback(EV_P_ struct ev_timer* w, int revents)
{
    if (!hscp_resend(&hscp_priv)) ev_timer_stop(EV_A_ w);
return;
    ev_unloop(EV_A_ EVUNLOOP_ONE);
}
#endif//EV_H__


inline unsigned short bcd2bin(unsigned char  val)
{
    return (val & 0x0f) + (val >> 4) * 10;
}

inline unsigned char  bin2bcd(unsigned short val)
{
    return ((val / 10) << 4) + val % 10;
}

unsigned char checksum_xor(unsigned char* buf, unsigned char len)
{
    unsigned char csum  = 0;
    while (len--) csum ^= buf[len];	return csum;
}


int hscp_resend(struct hscp_priv* priv)
{
    if (priv->pending) {	--priv->pending;
	full_write(*(int*)priv->data, (void*)&priv->smsg,
		priv->smsg.len + 2 + 1);
    }	return  priv->pending;
}

void hscp_send(int fd, struct hscp_priv* priv, struct hscp_cmsg* smsg)
{
    smsg->stx = HSCP_CMSG_STX;
    smsg->buf[smsg->len - 2] = checksum_xor((void*)smsg, smsg->len + 1);
    smsg->buf[smsg->len - 1] = HSCP_CMSG_CEND;
    full_write(fd, (void*)smsg, smsg->len + 2 + 1);

    if (&priv->smsg != smsg) priv->smsg = *smsg;
    priv->pending = 2;		poll_time = 200;

//if (1 && smsg->cmd != SEND_ACK) dprintp((char*)smsg, smsg->len + 2 + 1);
return;
#ifdef	EV_H__
    ev_timer_set(priv->repeat, .2, .2);
    ev_timer_start(evlp, priv->repeat);
#endif
}

void hscp_selftest_status(int fd, struct hscp_priv* priv,
	struct hscp_cmsg* smsg)
{
    struct hscp_selftest* sst = (void*)smsg->buf;

    // FIXME: relocate status bytes to hscp_priv structure
    sst->gprs = '1';	sst->gsm  = '1';
    sst->gps  = '0';	sst->host = '0';

    smsg->cmd = SEND_SELFTEST;
    smsg->len = 4 + 2; //sizeof(*sst) + 2;

    hscp_send(fd, priv, smsg);
}

void hscp_update_status(int fd, struct hscp_priv* priv,
	nmeaINFO* info, struct hscp_cmsg* smsg)
{
    struct hscp_gps_status* gpss = (void*)smsg->buf;
    struct misc_priv* misc = &misc_priv;
    int val;

#if 0
    int i = 0;		smsg->cmd = 0xc0;
    // "hhmmssABBBBbbbbNLLLLLllllEsssddyymmddFFFFTCHAB"

    smsg->buf[i++] = '0' + (info->utc.hour / 10);
    smsg->buf[i++] = '0' + (info->utc.hour % 10);
    smsg->buf[i++] = '0' + (info->utc.min  / 10);
    smsg->buf[i++] = '0' + (info->utc.min  % 10);
    smsg->buf[i++] = '0' + (info->utc.sec  / 10);
    smsg->buf[i++] = '0' + (info->utc.sec  % 10);

    smsg->buf[i++] = 'A';	// XXX:

    val = info->lat * 1000;	val = abs(val);
    smsg->buf[i++] = '0';// +  (val / 10000000);
    smsg->buf[i++] = '0' + ((val / 1000000) % 10);
    smsg->buf[i++] = '0' + ((val / 100000)  % 10);
    smsg->buf[i++] = '0' + ((val / 10000)   % 10);
    smsg->buf[i++] = '0' + ((val / 1000)    % 10);
    smsg->buf[i++] = '0' + ((val / 100)	    % 10);
    smsg->buf[i++] = '0' + ((val / 10)	    % 10);
    smsg->buf[i++] = '0' +  (val 	    % 10);
    smsg->buf[i++] = info->lat < 0 ? 'S' : 'N';

    val = info->lon * 1000;	val = abs(val);
    smsg->buf[i++] = '0' +  (val / 10000000);
    smsg->buf[i++] = '0' + ((val / 1000000) % 10);
    smsg->buf[i++] = '0' + ((val / 100000)  % 10);
    smsg->buf[i++] = '0' + ((val / 10000)   % 10);
    smsg->buf[i++] = '0' + ((val / 1000)    % 10);
    smsg->buf[i++] = '0' + ((val / 100)	    % 10);
    smsg->buf[i++] = '0' + ((val / 10)	    % 10);
    smsg->buf[i++] = '0' +  (val	    % 10);
    smsg->buf[i++] = '0';
    smsg->buf[i++] = info->lon < 0 ? 'W' : 'E';

    val = info->speed;
    smsg->buf[i++] = '0' +  (val / 100);
    smsg->buf[i++] = '0' + ((val / 10)	% 10);
    smsg->buf[i++] = '0' +  (val	% 10);

    val = info->direction;
    smsg->buf[i++] = '0' + (val / 10);
    smsg->buf[i++] = '0' + (val % 10);

    val = (info->utc.year + 1900) % 100;
    smsg->buf[i++] = '0' + (val / 10);
    smsg->buf[i++] = '0' + (val % 10);
    val =  info->utc.mon + 1;
    smsg->buf[i++] = '0' + (val / 10);
    smsg->buf[i++] = '0' + (val % 10);
    smsg->buf[i++] = '0' + (info->utc.day / 10);
    smsg->buf[i++] = '0' + (info->utc.day % 10);

    smsg->buf[i++] = '0';	// XXX: car status
    smsg->buf[i++] = '0';
    smsg->buf[i++] = '0';
    smsg->buf[i++] = '0';

    smsg->buf[i++] = 0x11;
    smsg->buf[i++] = 24;	// XXX: CSQ
    smsg->buf[i++] = misc->guard;
    smsg->buf[i++] = misc->acc;
    smsg->buf[i++] = '0';	// call status

    smsg->len = i + 2;

#else// XXX:

    smsg->cmd = SEND_GPS_STATUS;
    smsg->len = //sizeof(*gpss) + 2;
	    (3 + 4 + 4 + 2 + 2 + 1 + 1 + 1 + 2 + 1 + 1 + 1 + 3) + 2;

    gpss->time.hour = bin2bcd((info->utc.hour + 8) % 24);
    gpss->time.min  = bin2bcd( info->utc.min);
    gpss->time.sec  = bin2bcd( info->utc.sec);

    val = info->lat * 1000;	// XXX:
    gpss->lat[0] = (val < 0 ? 0x80 : 0x00) | bin2bcd(abs(val) / 1000000);
    gpss->lat[1] = bin2bcd(((val = abs(val)) / 10000) % 100);
    gpss->lat[2] = bin2bcd( (val / 100) % 100);
    gpss->lat[3] = bin2bcd(  val % 100);

    val = info->lon * 1000;
    gpss->lon[0] = (val < 0 ? 0x80 : 0x00) | bin2bcd(abs(val) / 1000000);
    gpss->lon[1] = bin2bcd(((val = abs(val)) / 10000) % 100);
    gpss->lon[2] = bin2bcd( (val / 100) % 100);
    gpss->lon[3] = bin2bcd(  val % 100);

    val = info->speed;
    //gpss->speed = (unsigned short)info->speed;	// XXX:
    gpss->speed[0] = bin2bcd(val / 100);
    gpss->speed[1] = bin2bcd(val % 100);

    val = info->direction;
    gpss->dir[0]   = bin2bcd(val / 100);
    gpss->dir[1]   = bin2bcd(val % 100);

    gpss->gps  = '1';	// XXX:

    // FIXME: relocate these status bytes to hscp_priv structure
    gpss->gprs = 0x01;	gpss->type  = 0x00;	gpss->X = 0x00;
    gpss->net  = 0x01;	gpss->alert = 0x00;

    gpss->overspeed = misc->oa.os;	gpss->reserved = 0x00;
    gpss->dummy = 0x00;			gpss->csq = 24;	// FIXME:
    gpss->watch = misc->guard;		gpss->acc = misc->acc;

    gpss->date.year = bin2bcd((info->utc.year + 1900) % 100);
    gpss->date.mon  = bin2bcd( info->utc.mon  + 1);
    gpss->date.day  = bin2bcd( info->utc.day);
#endif

    hscp_send(fd, priv, smsg);	priv->pending = 0;	// XXX:
}

static void set_guard(struct misc_priv* priv)
{
    gettimeofday(&priv->gtime, NULL);
    if (0) fdprintf(STDOUT_FILENO, "%u: setting guard to %d\n",
	    (unsigned)priv->gtime.tv_sec, priv->guard);

    leds_path[20] = priv->guard ? '4' : '3';	priv->gcnt = !priv->guard;
    xopen_xwrite_close(leds_path, "1");		usleep(100 * 1000);
    xopen_xwrite_close(leds_path, "0");		// XXX:
}

void hscp_recv(int fd, struct hscp_priv* priv)
{
    int i = 0, len, k; // j
    int gsmfd = *(int*)gsmd_priv.data;
    struct gprs_priv* gprs = &gprs_priv;
    struct misc_priv* misc = &misc_priv;
    struct hscp_cmsg *cmsg = NULL, *smsg = &priv->smsg;
#define iobuf bb_common_bufsiz1

    if ((len = safe_read(fd, iobuf, sizeof(iobuf))) < 1) {
	signalled = SIGHUP;	return;
    }

ADJ:
    for (/*j = i*/; i < len; ++i) {	int l;
	if (iobuf[i] != HSCP_CMSG_STX) continue;
	else cmsg = (void*)&iobuf[i];

	if (len - i < offsetof(struct hscp_cmsg, buf)) cmsg->len = 0;
	while (len < (l = cmsg->len + offsetof(struct hscp_cmsg, buf) + i)) {
	    if (sizeof(iobuf) < l) ;	// FIXME:
	    if (0  < (l = safe_read(fd, &iobuf[len], l - len))) len += l;
	    else { signalled = SIGHUP;	return; }
	}   l = checksum_xor((void*)cmsg, cmsg->len + 1);

	if (cmsg->buf[cmsg->len - 1] == HSCP_CMSG_SEND &&
	    cmsg->buf[cmsg->len - 2] ==	l) break;	// XXX:
	else cmsg = NULL;
    }	if (!cmsg) return;

//dprintp((char*)cmsg, cmsg->len + 2 + 1);	// XXX: i - j + 1
    if (cmsg->cmd != SEND_ACK) {	smsg = (void*)&iobuf[len];
	smsg->cmd  = SEND_ACK;		smsg->len = 0 + 2;
	hscp_send(fd, priv, smsg);	smsg = &priv->smsg;
	priv->pending = 0;		usleep(10 * 1000);	// XXX:
    }

    switch (cmsg->cmd) {			// TODO:
    case SEND_SMS_MOBILE:
	//full_write(gsmfd, "AT+CMGF=0\r", 10);	// FIXME: wait for "OK"

	full_write(gsmfd, "AT+CMGS=", 8);
	full_write(gsmfd, cmsg->buf, 3);
	full_write(gsmfd, "\r", 1);		// FIXME: wait for ">"

	// convert to hexdecimal string for PDU
	for (k = 3; k + 2 < cmsg->len; ++k) {
	    unsigned char v = cmsg->buf[k], c;

	    c = (v >> 4);	c = c + (c < 0x0a ? '0' : 'A' - 0x0A);
	    full_write(gsmfd, &c, 1);

	    c = (v & 0x0f);	c = c + (c < 0x0a ? '0' : 'A' - 0x0A);
	    full_write(gsmfd, &c, 1);
	}   full_write(gsmfd, "", 1);

	// FIXME: wait for "OK"
	if (1) smsg->cmd = SEND_SMS_SUCCEED;	// XXX:
	else   smsg->cmd = SEND_SMS_FAILED;	smsg->len = 0 + 2;
	hscp_send(fd, priv, smsg);	break;

    case SEND_CMD_DIAL:
	full_write(gsmfd, "ATD", 3);
	full_write(gsmfd, cmsg->buf, cmsg->len - 2);
	full_write(gsmfd, ";\r", 2);

	// FIXME: wait for "OK" ???
	if (1) smsg->cmd = SEND_ACCEPTED;	// XXX:
	else   smsg->cmd = SEND_HANG;	smsg->len = 0 + 2;
	hscp_send(fd, priv, smsg);	break;

    case SEND_CMD_HANG:		full_write(gsmfd, "ATH\r", 4);

	// FIXME: wait for "OK"
	if (1) smsg->cmd = SEND_HANG;	smsg->len = 0 + 2;
	hscp_send(fd, priv, smsg);	break;

    case SEND_HANDSET_VER:		break;	// XXX:

    case SEND_CMD_SELFTEST:
	hscp_selftest_status(fd, priv, smsg);		break;

    case SEND_ACK:		priv->pending = 0;	break;

    //case REQUEST_HOST:	break;		// TODO:

    case SEND_CMD_ACCEPT:	full_write(gsmfd, "ATA\r", 4);

	if (0) break;	// FIXME: wait for "OK"
	smsg->cmd = SEND_ACCEPTED;	smsg->len = 0 + 2;
	hscp_send(fd, priv, smsg);	break;

    case HANDSET_FREE:
	full_write(gsmfd, "AT+CHF=0,1\r", 11);
	break;	// FIXME: wait for "OK"

    case SET_APN:
	if (!((k = cmsg->len - 1  - 1 - 1) < sizeof(gprs->apn)))
	    k = sizeof(gprs->apn) - 1;
	memcpy(gprs->apn, &cmsg->buf[1], k);	gprs->apn[k] = 0;
	gprs_conn(gsmfd, gprs);		break;	// XXX:

    case SET_SELF_PN:
	if (!((k = cmsg->len  - 1 - 1 - 4) < sizeof(gprs->spkt.tid)))
	    k = sizeof(gprs->spkt.tid) - 1;
	memcpy(gprs->spkt.tid, &cmsg->buf[4], k);   gprs->spkt.tid[k] = 0;
	gprs_conn(gsmfd, gprs);		break;	// XXX:

    case SET_USER_PN:	// XXX:
	if (!((k = cmsg->len  - 1 - 1 - 2) < sizeof(misc->mn)))
	    k = sizeof(misc->mn)  - 1;
	memcpy(misc->mn, &cmsg->buf[1], k);
	misc->mn[k] = 0;		break;

    case HSCP_SET_GUARD: 	// PWD: 888888
	misc->guard = 0x01;	set_guard(misc); 	break;
    case HSCP_RELEASE_GUARD:	// XXX:
	misc->guard = 0x00;	set_guard(misc); 	break;

    case SET_SERVER_IP: 	// PWD: 20050215
	// XXX: <1,"202.038.024.001",2020>

	if ((k = cmsg->len) < 5) break;
	for (k -= 2; --k && cmsg->buf[k] != ','; ) ;
	if (!k) break; else cmsg->buf[k - 1] = '\0';
	if (1) {	unsigned char c, n, f = 1, m = 0;
	    for (n = 4; ; ++n) {
		if (!(c = cmsg->buf[n])) break;
		if (f) {
		    if ('0' == c) continue; else f = 0;
		}   else if (c == '.') f = 1;
		gprs->sip[m++] = c;
	    }   gprs->sip[m] = '\0';
	} else	// XXX:
	strcpy(gprs->sip,   (char*)&cmsg->buf[4]);

	cmsg->buf[cmsg->len - 3]  = '\0';
	strcpy(gprs->sport, (char*)&cmsg->buf[++k]);

	smsg->cmd = SET_SUCCESS;	smsg->len = 1 + 2;
	smsg->buf[0] = 0x00;		hscp_send(fd, priv, smsg);

	gprs_conn(*(int*)gprs->data, gprs);	break;

    default: dprintp((unsigned char*)cmsg, len - i);
    }

    if ((i = (char*)&cmsg->buf[cmsg->len] - iobuf) < len) goto ADJ;
}

void gpsc_recv(int fd, struct gpsc_priv* priv)
{
    static unsigned stc = 0x3f;
    struct gprs_priv* gprs = &gprs_priv;
    struct misc_priv* misc = &misc_priv;
    nmeaINFO* info = &priv->info;
    nmeaTIME* res  = &info->utc;
    int len;

//#define iobuf bb_common_bufsiz1
    if ((len = safe_read(fd, iobuf, sizeof(iobuf))) < 1) {
	signalled = SIGHUP;	return;
    }	//full_write(STDOUT_FILENO, iobuf, len);

    info->smask = 0;	// XXX:
    if ((len = nmea_parse(&priv->parser, iobuf, len, info)) < 1 ||
	    !(info->smask & GPGGA)) return;	// XXX: 1 Hz tick

    if (misc->oa.os && misc->alarm != ALARM_OVERSPEED) {
	if (misc->oa.os < info->speed) {
	    if (misc->oa.dl < ++misc->oa.ct) {
		misc->alarm = ALARM_OVERSPEED;	misc->acnt = 1;
		gprs_send_alarm(*(int*)gprs->data, gprs, GPRS_OVERS_ALARM);
	    }
	} else  misc->oa.ct = 0;
    }

    if (misc->rg.tl.lon < 180.f * 100 && misc->alarm != ALARM_REGION) {
	if (misc->rg.br.lat < info->lat || info->lat < misc->rg.tl.lat ||
	    misc->rg.br.lon < info->lon || info->lon < misc->rg.tl.lon) {
	    misc->alarm = ALARM_REGION;		misc->acnt = 1;
	    gprs_send_alarm(*(int*)gprs->data, gprs, GPRS_REGION_ALARM);
	}
    }

    if (misc->gprs && misc->gpsr.inte
		   && misc->gpsr.inte < ++misc->gpsr.intv) {
	struct gprs_cpkt* spkt = (void*)&iobuf[len];

	memcpy(spkt, &gprs->spkt, offsetof(struct gprs_cpkt, cmd));
	spkt->cmd = GPRS_GET_GPS;	spkt->buf[0] = 0;
	gprs_set_gpsi(spkt->buf, info);
	gprs_send(*(int*)gprs->data, gprs, spkt);
	//len += 100;	// XXX:

	if (misc->gpsr.maxt && misc->gpsr.maxt < ++misc->gpsr.cnts)
	    misc->gpsr.inte = 0;	misc->gpsr.intv = 0;
    }

    if (misc->alarm != ALARM_NONE && misc->acnt &&
	    !(++misc->acnt & 0x3f)) {	unsigned alrm = 0;
	switch (misc->alarm) {
	case ALARM_GUARD:	alrm = GPRS_GUARD_ALARM;	break;
	case ALARM_OVERSPEED:	alrm = GPRS_OVERS_ALARM;	break;
	case ALARM_POWER:	alrm = GPRS_POWER_ALARM;	break;
	case ALARM_REGION:	alrm = GPRS_REGION_ALARM;	break;
	case ALARM_START:	alrm = GPRS_START_ALARM;	break;
	case ALARM_DOOR:	alrm = GPRS_DOOR_ALARM;		break;
	case ALARM_SHAKE:	alrm = GPRS_SHAKE_ALARM;	break;
	default: dprintn(misc->alarm);
	}

	gprs_send_alarm(*(int*)gprs->data, gprs, alrm);
    }

    if (misc->hint && ++misc->hcnt == misc->hint) {
	struct gprs_cpkt* spkt = (void*)&iobuf[len];

	memcpy(spkt, &gprs->spkt, offsetof(struct gprs_cpkt, cmd));
	spkt->cmd = GPRS_CPKT_ACK;
#if 0
    {	unsigned short v, n = 0;
	v = htons(misc->hint << 1);
	memcpy(&spkt->buf[n], &v, 2);	n += 2;

	memcpy(&spkt->buf[n], gprs->sip, 16);	n += 16;

	v = htons(atoi(gprs->sport));
	memcpy(&spkt->buf[n], &v, 2);	//n += 2;
    }
#endif
	gprs_send(*(int*)gprs->data, gprs, spkt);
    }

    if (0 && misc->gcnt && 25 < ++misc->gcnt) {	// XXX:
	misc->guard = !misc->acc;	set_guard(misc);
    }

    if (!(++stc & 0x3f)) {		struct tm ntm;	time_t tim;
	ntm.tm_year = res->year;	ntm.tm_mon  = res->mon;
	ntm.tm_mday = res->day;		ntm.tm_hour = res->hour;
	ntm.tm_min  = res->min;		ntm.tm_sec  = res->sec;
	ntm.tm_isdst = -1;	// XXX:
	tim = mktime(&ntm);		if (stime(&tim)) ;
    }

    hscp_update_status(*(int*)hscp_priv.data,	// XXX:
	    &hscp_priv, info, (void*)&iobuf[len]);
return;

  { extern double fabs(double x);
    double lon  = nmea_ndeg2degree(info->lon);
    double lat  = nmea_ndeg2degree(info->lat);

    fdprintf(STDOUT_FILENO,
	"[%d-%02d-%02d %02d:%02d:%02d.%03d] (%g%c %g%c %gA) <%gkm/h %gD>\n",
	     res->year + 1900, res->mon + 1, res->day,
	    (res->hour + 8) % 24, res->min,  res->sec, res->hsec,
	    fabs(lon), 0 < lon ? 'E' : 'W', fabs(lat), 0 < lat ? 'N' : 'S',
	    info->elv, info->speed, info->direction);
  }

    if (info->satinfo.inview)
	fdprintf(STDOUT_FILENO, "\tID\tElv.\tAzimuth\tSNR(dB)\t%d/%d\n",
		info->satinfo.inuse, info->satinfo.inview);
    for (len = 0; len < info->satinfo.inview; ++len) {
	nmeaSATELLITE* sat = &info->satinfo.sat[len];
	fdprintf(STDOUT_FILENO, "\t%02d\t%3d\t%5d\t%3d\t%c\n",
		sat->id, sat->elv, sat->azimuth, sat->sig,
		sat->in_use ? '*' : ' ');
    }
  {
static const char *opmd[] = { "Invaild", "Fix not avaible", "2D", "3D" },
		  *gpsq[] = { "Invalid", "Fix", "Differential", "Sensitive" };
    fdprintf(STDOUT_FILENO, "GPS quality: %s; Operating mode: %s\n",
	    gpsq[info->sig], opmd[info->fix]);
  }
}

void gprs_conn(int fd, struct gprs_priv* priv)
{
    leds_path[20] = '1';	xopen_xwrite_close(leds_path, "0");

    full_write(fd, "AT+CIPCLOSE\r", 13);	// FIXME: wait for "CLOSE OK"
    full_write(fd, "AT+CIPSHUT\r", 11);		// FIXME: wait for "SHUT OK"
    usleep(20 * 1000);	// XXX:

    full_write(fd, "AT+CGDCONT=1,\"IP\",\"", 19);
    full_write(fd, priv->apn, strlen(priv->apn));
    full_write(fd, "\"\r", 2);	// FIXME: wait for "OK"
    full_write(fd, "AT+CIPCSGP=1,\"", 14);
    full_write(fd, priv->apn, strlen(priv->apn));
    full_write(fd, "\"\r", 2);	// FIXME: wait for "OK"

    full_write(fd, "AT+CIPSTART=\"TCP\",\"", 19);
    full_write(fd, priv->sip,   strlen(priv->sip));
    full_write(fd, "\",\"", 3);
    full_write(fd, priv->sport, strlen(priv->sport));
    full_write(fd, "\"\r", 2);
    // FIXME: wait for "OK"
}

void gprs_send(int fd, struct gprs_priv* priv, struct gprs_cpkt* spkt)
{
    full_write(fd, "AT+CIPSEND=100\r", 15);	// XXX:
    // FIXME: wait for ">"
    if (0) {
	gprs_conn(fd, priv);
	if (spkt != &priv->spkt) priv->spkt = *spkt;
	priv->pending = 1;	return;
    }	priv->pending = 0;

    spkt->xor = checksum_xor((unsigned char*)spkt,
	    offsetof(struct gprs_cpkt, cmd));	// XXX:

    full_write(fd, (void*)spkt, sizeof(*spkt));	// XXX:
    //full_write(fd, "", 1);	// XXX:

return;
if (1 && spkt->cmd != GPRS_CPKT_ACK)
    dprintp((unsigned char*)spkt, sizeof(*spkt));
}

void gprs_set_gpsi(unsigned char* buf, nmeaINFO* info)
{
    int val, k = 1;	buf[k++] = 1;

    val = info->lon * 1000;
    buf[k++] =  val / 100000;
    buf[k++] = (val / 1000) % 100;
    buf[k++] =((val % 1000) * 60) / 1000;

    val = info->lat * 1000;
    buf[k++] =  val / 100000;
    buf[k++] = (val / 1000) % 100;
    buf[k++] =((val % 1000) * 60) / 1000;

    val = info->speed * 10;
    buf[k++] = '0' +  (val / 1000);
    buf[k++] = '0' + ((val / 100) % 10);
    buf[k++] = '0' + ((val / 10)  % 10);
    buf[k++] = '.';
    buf[k++] = '0' +  (val % 10);
    buf[k++] =  0;

    val = info->direction * 10;
    buf[k++] = '0' +  (val / 1000);
    buf[k++] = '0' + ((val / 100) % 10);
    buf[k++] = '0' + ((val / 10)  % 10);
    buf[k++] = '.';
    buf[k++] = '0' +  (val % 10);
    buf[k++] =  0;	// XXX:
}

void send_alarm_sms(int fd, char* mn, const char* ucs)
{
    unsigned short i, j;	// XXX: UCS-2BE

    // FIXME: implement variable length
    static char asms[] = "0011000D9168" "3112651932F6" "000801" "14"
	    "672c8f66966960c5ff1a" "975e6cd570b9706b" "ff01" "";

    usleep(10 * 1000);	// XXX:
    full_write(fd, "AT+CMGS=035\r", 13);	// FIXME: wait for ">"

    for (i = 0, j = 12; i < strlen(mn)/*11*/; i += 2, j += 2) {
	asms[j]     = mn[i + 1];
	asms[j + 1] = mn[i];
    }   asms[j - 2] = 'F';
    memcpy(&asms[j + (6 + 2 + 20)], ucs, 16);

    full_write(fd, asms, sizeof(asms) - 1/*73*/);

//dprintp(asms, sizeof(asms) - 1);
}

void gprs_send_alarm(int fd, struct gprs_priv* priv, unsigned cmd)
{
    struct misc_priv* misc = &misc_priv;
    struct gprs_cpkt* spkt = (void*)&iobuf[sizeof(iobuf) - 100]; // XXX:

    memcpy(spkt, &priv->spkt, offsetof(struct gprs_cpkt, cmd));
    spkt->cmd = cmd;	spkt->buf[0] = misc->alarm;

    gprs_set_gpsi(spkt->buf, &gpsc_priv.info);
    gprs_send(fd, priv, spkt);
    set_guard(misc);	// XXX:

    fdprintf(STDOUT_FILENO, "Alarming: %d\n", misc->alarm);
//dprintp((unsigned char*)spkt, sizeof(*spkt));
}

void gsmd_recv(int fd, struct gsmd_priv* priv)
{
    int i = 0, len, j, k;
    struct misc_priv* misc = &misc_priv;
    struct hscp_priv* hscp = &hscp_priv;
    struct hscp_cmsg* smsg = &hscp->smsg;
    int hfd = *(int*)hscp->data;

//#define iobuf bb_common_bufsiz1
    if ((len = safe_read(fd, iobuf, sizeof(iobuf))) < 1) {
	signalled = SIGHUP;	return;
    }	//full_write(STDOUT_FILENO, iobuf, len);

ADJ:
    for (j = i, k = 0; ; ++i) {	int l;
	if (i == len) {
	    if (sizeof(iobuf) < len) ;	// FIXME:
	    if (0 < (l = safe_read(fd, &iobuf[len],
		    sizeof(iobuf) - len))) len += l;
	    else { signalled = SIGHUP;	return; }
	}
	if (iobuf[i] == '\n') break; else
	if (iobuf[j] == GPRS_CPKT_STX && !k) {
	    struct gprs_cpkt* cpkt = (void*)&iobuf[j];	int cmd = -1;
	    while (len < (l = j + 40)) {
		if (sizeof(iobuf) < l) ;	// FIXME:
		if (0 < (l = safe_read(fd, &iobuf[len], l - len))) len += l;
		else { signalled = SIGHUP;	return; }
	    }

//dprintp((unsigned char*)cpkt, 40);
	    if (//strncmp(cpkt->tid, priv->spkt.tid, sizeof(cpkt->tid)) ||
		    checksum_xor((unsigned char*)cpkt,
			    offsetof(struct gprs_cpkt, cmd)) != cpkt->xor)
		; //{ k = 1; continue; }

	    if ((cmd = cpkt->cmd) == GPRS_SERVER_ACK) {
		if ((i = (j += 40)) < len) goto ADJ; else return;
	    }	cpkt->cmd = GPRS_CPKT_ACK;	// XXX:
	    gprs_send(fd, priv, cpkt);	usleep(10 * 1000);	// XXX:

	    switch ((cpkt->cmd = cmd)) {
	    case GPRS_GET_GPS:		cpkt->buf[0] = 0;
		gprs_set_gpsi(cpkt->buf, &gpsc_priv.info);
		gprs_send(fd, priv, cpkt);	break;

	    case GPRS_FREE_ALARM:	cpkt->buf[0] = 0;
		leds_path[20] = '1';	xopen_xwrite_close(leds_path, "0");
		misc->alarm = ALARM_NONE;	misc->acnt = 1;	// XXX:
		gprs_send(fd, priv, cpkt);	usleep(10 * 1000);

		cpkt->cmd = GPRS_GET_GPS;
		gprs_set_gpsi(cpkt->buf, &gpsc_priv.info);
		xopen_xwrite_close(leds_path, "1");	// XXX:
		gprs_send(fd, priv, cpkt);	break;
		// XXX: receive control from server

	    case GPRS_SHUT_OILC:
		if ((1 || cpkt->buf[0])/* && !misc->acc*/) {
		    misc->power = 0x00;	leds_path[20] = '2';
		    xopen_xwrite_close( leds_path, "0");
		}			cpkt->buf[0] = 0;
		gprs_send(fd, priv, cpkt);	break;
		// XXX:
	    case GPRS_OPEN_OILC:
		if (1 || cpkt->buf[0]) {
		    misc->power = 0x01;	leds_path[20] = '2';
		    xopen_xwrite_close( leds_path, "1");
		}			cpkt->buf[0] = 0;
		gprs_send(fd, priv, cpkt);	break;

	    case GPRS_SERVER_ACK:		break;

	    case GPRS_REPEAT_GPS:
		if (cpkt->buf[0] == 0xff) misc->gpsr.inte = 0; else
		if (cpkt->buf[0]) {	// FIXME:
		    memcpy(&misc->gpsr.inte, &cpkt->buf[1], 4);
		    misc->gpsr.inte = 1 + ntohl(misc->gpsr.inte) / 10;
			//* 36 / (int)gpsc.info->speed / 10;
		} else {		// XXX:
		    memcpy(&misc->gpsr.inte, &cpkt->buf[1], 4);
		    misc->gpsr.inte = ntohl(misc->gpsr.inte);
		}
		    memcpy(&misc->gpsr.maxt, &cpkt->buf[5], 4);
		    misc->gpsr.maxt = ntohl(misc->gpsr.maxt);
		    misc->gpsr.intv = misc->gpsr.cnts = 0;	break;

	    case GPRS_QUERY_STATUS: {
		unsigned short uphr, n = 0;
		struct sysinfo info;	sysinfo(&info);
		uphr = htons(info.uptime / 3600);

		cpkt->buf[n++] =  misc->alarm;
		cpkt->buf[n++] = !misc->power + (misc->guard << 2);
		memcpy(&cpkt->buf[n], &uphr, 2);	n += 2;
		cpkt->buf[n++] = 60;	// XXX:
	    }	gprs_send(fd, priv, cpkt);	break;

	    case GPRS_RESET:
		switch (cpkt->buf[0]) {
		case 0x02:	misc->guard = 0x00;
				misc->power = 0x01;	break;
		case 0x03:  //if (!misc->acc)
				misc->guard = 0x01;
				misc->power = 0x01;	break;
		case 0x04:	misc->guard = 0x00;
		/*if (!misc->acc)*/ misc->power = 0x00;	break;
		case 0x05:  //if (!misc->acc)  break;
				misc->guard = 0x01;
				misc->power = 0x00;	break;
		case 0x01:	// TODO:
		default: ;
		}	cpkt->buf[0] = 0;	set_guard(misc);

		leds_path[20] = '2';	xopen_xwrite_close(leds_path,
			misc->power ? "1" : "0");

		gprs_send(fd, priv, cpkt);	break;

	    case GPRS_SET_OVERS:
		if (cpkt->buf[0]) {
		    memcpy(&misc->oa.dl, &cpkt->buf[1], 2);
		    misc->oa.os = ntohs(misc->oa.dl);
		    memcpy(&misc->oa.dl, &cpkt->buf[3], 2);
		    misc->oa.dl = ntohs(misc->oa.dl) * 60;	// XXX:

		    cpkt->buf[0] = 0;	misc->oa.ct = 0;
		} else {	//misc->alarm = ALARM_NONE;
		    gprs_send(fd, priv, cpkt);	usleep(10 * 1000);

		    cpkt->cmd = GPRS_GET_GPS;	misc->oa.os = 0;
		    gprs_set_gpsi(cpkt->buf, &gpsc_priv.info);
		}   gprs_send(fd, priv, cpkt);	break;

	    case GPRS_SET_REGION:
		if (cpkt->buf[0]) {	unsigned short v, n = 1;
		    v = cpkt->buf[n++]; v *= 100;  v += cpkt->buf[n++];
		    misc->rg.tl.lon = cpkt->buf[n++] / 60.f + v;

		    v = cpkt->buf[n++]; v *= 100;  v += cpkt->buf[n++];
		    misc->rg.tl.lat = cpkt->buf[n++] / 60.f + v;

		    v = cpkt->buf[n++]; v *= 100;  v += cpkt->buf[n++];
		    misc->rg.br.lon = cpkt->buf[n++] / 60.f + v;

		    v = cpkt->buf[n++]; v *= 100;  v += cpkt->buf[n++];
		    misc->rg.br.lat = cpkt->buf[n++] / 60.f + v;

		    cpkt->buf[0] = 0;
		} else {
		    gprs_send(fd, priv, cpkt);	usleep(10 * 1000);

		    cpkt->cmd = GPRS_GET_GPS;
		    //misc->alarm = ALARM_NONE;
		    misc->rg.tl.lon = 180.f * 100;
		    gprs_set_gpsi(cpkt->buf, &gpsc_priv.info);
		}   gprs_send(fd, priv, cpkt);	break;

	    case GPRS_MONITORING:	++cpkt->cmd;
		memcpy(misc->mn, cpkt->buf, 16); cpkt->buf[0] = 0;

		full_write(fd, "ATD", 3);	// XXX:
		full_write(fd, misc->mn, strlen(misc->mn)/*11*/);
		full_write(fd, ";\r", 2);	// FIXME: wait for "OK" ???

		gprs_send(fd, priv, cpkt);	break;

	    case GPRS_QUERY_VERS: {
		unsigned short  v, n = 0;
		v = 1000;	v = htons(v);	// XXX: MS
		memcpy(&cpkt->buf[n], &v, 2);	n += 2;

		cpkt->buf[n++] = 1;	cpkt->buf[n++] = 1;
		cpkt->buf[n++] = 2;	// XXX: HV/SV/TPV

		v = 2;	v = htons(v);	// GSM/GPRS
		memcpy(&cpkt->buf[n], &v, 2);	n += 2;

		v = 3;	v = htons(v);	// GPS
		memcpy(&cpkt->buf[n], &v, 2);	n += 2;

		v = 1;	v = htons(v);	// P1
		memcpy(&cpkt->buf[n], &v, 2);	n += 2;

		v = 1;	v = htons(v);	// P2
		memcpy(&cpkt->buf[n], &v, 2);	n += 2;

		v = 1;	v = htons(v);	// P3
		memcpy(&cpkt->buf[n], &v, 2);	//n += 2;
	    }	gprs_send(fd, priv, cpkt);	break;

	    case GPRS_QUERY_SERVER: {
		unsigned short v, n = 0;
		v = htons(misc->hint << 1);
		memcpy(&cpkt->buf[n], &v, 2);	n += 2;

		memcpy(&cpkt->buf[n], priv->sip, 16);	n += 16;

		v = htons(atoi(priv->sport));
		memcpy(&cpkt->buf[n], &v, 2);	//n += 2;
	    }	gprs_send(fd, priv, cpkt);	break;

	    case GPRS_SET_SERVER:	// XXX:
		memcpy(&misc->hint, &cpkt->buf[0], 2);
		misc->hint = ntohs(misc->hint) >> 1;
		gprs_send(fd, priv, cpkt);	break;

	    case GPRS_QUERY_SMS:	case GPRS_SET_SMS:
		// TODO: "AT+CSCA?\r"	// +CSCA: "+8613800551500",145

	    case GPRS_SUSPEND:		case GPRS_RESUME:	// TODO:

	    default: dprintp((unsigned char*)cpkt, 40);
		cpkt->buf[0] = 0;	gprs_send(fd, priv, cpkt);
	    }

	    if ((i = (j += 40)) < len) goto ADJ;
	}
    }	while (!isprint(iobuf[j]) && j < i) ++j;
    if (i == j) { if (++i < len) goto ADJ; else return; }
//full_write(STDOUT_FILENO, &iobuf[j], i - j + 1);

    if (!memcmp(&iobuf[j], "Call Ready", 10)) {
	// XXX: init sequences
	//gprs_conn(fd, priv);
    }	else
    if (!memcmp(&iobuf[j], "CONNECT OK", 10)) {
	struct gprs_cpkt* spkt = (void*)&iobuf[len];

	usleep(20 * 1000);	// XXX:
	leds_path[20] = '1';	xopen_xwrite_close(leds_path, "1");
	memcpy(spkt, &priv->spkt, offsetof(struct gprs_cpkt, cmd));
	spkt->cmd = GPRS_REGISTER;	gprs_send(fd, priv, spkt);
	full_write(STDOUT_FILENO, "GPRS (re-)connected.\n", 21);

	if (priv->pending) {		usleep(10 * 1000);	// XXX:
	    gprs_send(fd, priv, &priv->spkt);
	}   misc->gprs = 0x01;	// XXX:
    }	else
    if (!memcmp(&iobuf[j], "CLOSED", 6) ||
	!memcmp(&iobuf[j], "TCP ERROR: ", 11) ||
	//!memcmp(&iobuf[j], "+PDP: DEACT", 11) ||	// XXX:
	!memcmp(&iobuf[j], "CONNECT FAIL", 12)) {
	misc->gprs = 0x00;	gprs_conn(fd, priv);
    }	else
    if (!memcmp(&iobuf[j], "STATE: ", 7) && (j += 7) &&
       (!memcmp(&iobuf[j], "IP INITIAL", 10) ||
	//!memcmp(&iobuf[j], "+FCERROR", 8) ||
	!memcmp(&iobuf[j], "PDP DEACT", 9) ||
	!memcmp(&iobuf[j], "IP CLOSE", 8))) {
	misc->gprs = 0x00;	gprs_conn(fd, priv);
    }	else
    if (!memcmp(&iobuf[j], "+CMTI: \"SM\",", 12)) {
	int c, l;	j += 12;
//	full_write(fd, "AT+CMGF=1\r", 10);	// FIXME: wait for "OK"
	full_write(fd, "AT+CMGR=", 8);
	full_write(fd, &iobuf[j], i - j);
//if (++i < len) goto ADJ;	// XXX: for SMS_TXT

	smsg->cmd = SEND_NEW_SMS_PDU;
	for (j = ++i, c = 0; ; ++i) {
	    if (i == len) {
		if (sizeof(iobuf) < len) ;	// FIXME:
		if (0 < (l = safe_read(fd, &iobuf[len],
			sizeof(iobuf) - len))) len += l;
		else { signalled = SIGHUP;	return; }
	    }
	    if (iobuf[i] == '\n') {
		if (c < 1) {
		    if (!memcmp(&iobuf[j], "+CMGR: 0,", 9)) ++c;
		    j = i + 1;
		} else break;
	    }
	}

	// FIXME: seperate function
	c = iobuf[j++]; c = c - ('9' < c ? 'A' - 10 : '0'); l  = c << 4;
	c = iobuf[j++]; c = c - ('9' < c ? 'A' - 10 : '0'); l += c;
	j += (l << 1) + 2;
	c = iobuf[j++]; c = c - ('9' < c ? 'A' - 10 : '0'); l  = c << 4;
	c = iobuf[j++]; c = c - ('9' < c ? 'A' - 10 : '0'); l += c;
	c = iobuf[j++]; c = c - ('9' < c ? 'A' - 10 : '0'); k  = c << 4;
	c = iobuf[j++]; c = c - ('9' < c ? 'A' - 10 : '0'); k += c;

	smsg->buf[0] = '"';
	if (k == 0x91 && iobuf[j] == '6' && iobuf[j + 1] == '8') {
	    smsg->buf[k = 1] = '+';	++k, l += 2;
	} else k = 1;

	for ( ; k < l; j += 2) {
	    smsg->buf[k++] = iobuf[j + 1];
	    smsg->buf[k++] = iobuf[j];
	}   smsg->buf[k - 1] = '"';	smsg->buf[k] = '"';	j += 2;

	c = iobuf[j++]; c = c - ('9' < c ? 'A' - 10 : '0'); l  = c << 4;
	c = iobuf[j++]; c = c - ('9' < c ? 'A' - 10 : '0'); l += c;
	smsg->buf[k++] = '"';
	smsg->buf[k++] = iobuf[j + 1];
	smsg->buf[k++] = iobuf[j];	j += 2;
	smsg->buf[k++] = '/';
	smsg->buf[k++] = iobuf[j + 1];
	smsg->buf[k++] = iobuf[j];	j += 2;
	smsg->buf[k++] = '/';
	smsg->buf[k++] = iobuf[j + 1];
	smsg->buf[k++] = iobuf[j];	j += 2;
	smsg->buf[k++] = ',';
	smsg->buf[k++] = iobuf[j + 1];
	smsg->buf[k++] = iobuf[j];	j += 2;
	smsg->buf[k++] = ':';
	smsg->buf[k++] = iobuf[j + 1];
	smsg->buf[k++] = iobuf[j];	j += 2;
	smsg->buf[k++] = ':';
	smsg->buf[k++] = iobuf[j + 1];
	smsg->buf[k++] = iobuf[j];	j += 2;
	smsg->buf[k++] = '+';
	smsg->buf[k++] = iobuf[j + 1];
	smsg->buf[k++] = iobuf[j];	j += 2;
	smsg->buf[k++] = '"';

	c = iobuf[j++]; c = c - ('9' < c ? 'A' - 10 : '0'); l  = c << 4;
	c = iobuf[j++]; c = c - ('9' < c ? 'A' - 10 : '0'); l += c;
	smsg->buf[k++] = '"';
	while (l--) {	int n;
	    c = iobuf[j++]; c = c - ('9' < c ? 'A' - 10 : '0'); n  = c << 4;
	    c = iobuf[j++]; c = c - ('9' < c ? 'A' - 10 : '0'); n += c;
	    smsg->buf[k++] = n;
	}   smsg->buf[k++] = '"';

	smsg->len = k + 2;
	hscp_send(hfd, hscp, smsg);		// FIXME: wait for "OK"

	full_write(fd, "AT+CMGD=1\r", 10);	// FIXME: delete this SMS
    }	else
    if (!memcmp(&iobuf[j], "+CMGR: \"REC UNREAD\",", 20)) {
	smsg->cmd = SEND_NEW_SMS_TXT;	k = 0;
	for (j += 20; ; ++j) {	unsigned c = iobuf[j];
	    if (c == ',')  break; else smsg->buf[k++] = c;
	}
	for (++j; iobuf[j] != ','; ++j) ;
	for (++j; ; ++j) {	unsigned c = iobuf[j];
	    if (c == '\r') break; else smsg->buf[k++] = c;
	}

	smsg->buf[k++] = '"';
	for (j = i; ; ++i) {
	    if (i == len) {	unsigned l;
		if (sizeof(iobuf) < len) ;	// FIXME:
		if (0 < (l = safe_read(fd, &iobuf[len],
			sizeof(iobuf) - len))) len += l;
		else { signalled = SIGHUP;	return; }
	    }
	    if (iobuf[i] == '\n') {
		while (!isprint(iobuf[j]) && j < i) ++j;
		if (!memcmp(&iobuf[j], "OK", 2)) break; else
		    while (!(i < j)) smsg->buf[k++] = iobuf[j++];
	    }
	}   smsg->buf[k++] = '"';

	smsg->len = k + 2;
	hscp_send(hfd, hscp, smsg);

	full_write(fd, "AT+CMGD=1\r", 10);	// FIXME: delete this SMS
    }	else
    if (!memcmp(&iobuf[j], "NO CARRIER", 10)) {
	//full_write(fd, "AT+CHF=0,0\r", 11);	// XXX:
	smsg->cmd = SEND_HANG;	smsg->len = 0 + 2;
	hscp_send(hfd, hscp, smsg);
    }	else
    if (!memcmp(&iobuf[j], "+CLIP: \"", 8)) {	// "RING"
	smsg->cmd = SEND_NEW_CALL;	 k = 0;
	for (j += 8; ; ++j) {	unsigned c = iobuf[j];
	    if (c == '"') break; else smsg->buf[k++] = c;
	}   smsg->len = k + 2;

	//k = strlen(misc->mn); //k = 11;
	if (misc->guard && !memcmp(misc->mn, smsg->buf, k))
	    full_write(fd, "ATA\r", 4); else	// XXX:
	hscp_send(hfd, hscp, smsg);
    //}	else if (!memcmp(&iobuf[j], "+CMS ERROR: ", 12)) {
    }	else if (!memcmp(&iobuf[j], "OK", 2) || iobuf[j] == '>') {
    } else full_write(STDOUT_FILENO, &iobuf[j], i - j + 1);

    if (++i < len) goto ADJ;
}

void btni_recv(int fd, struct btni_priv* priv)
{
//#define iobuf bb_common_bufsiz1
    struct input_event* ie = (void*)iobuf;
    struct misc_priv* misc = &misc_priv;
    struct gprs_priv* gprs = &gprs_priv;
    int gsmfd = *(int*)gprs->data;
    int len = sizeof(*ie);

    while (len) {	int n;
	if ((n = safe_read(fd, ie, len)) < 1) {
	    signalled = SIGHUP;	return;
	}   else len -= n;
    }	if (ie->type != EV_KEY) return;

    if (ie->code == BTN_1 && ie->value) {	// XXX:
	misc->alarm = ALARM_GUARD;	misc->acnt = 1;
	leds_path[20] = '1';	xopen_xwrite_close(leds_path, "0");
	gprs_send_alarm(*(int*)gprs->data, gprs, GPRS_GUARD_ALARM);

	usleep(20 * 1000);	// XXX:
	full_write(gsmfd, "ATD", 3);
	full_write(gsmfd, misc->mn, strlen(misc->mn)/*11*/);
	full_write(gsmfd, ";\r", 2);	// FIXME: wait for "OK" ???

	leds_path[20] = '1';	xopen_xwrite_close(leds_path, "1");
	full_write(STDOUT_FILENO, "Monitoring ...\n", 15);	return;
    }

    if (0) fdprintf(STDOUT_FILENO, "%u: T-%x, C-%x, V-%x\n",
	    (unsigned)ie->time.tv_sec, ie->type, ie->code, ie->value);
    if (ie->time.tv_sec < misc->gtime.tv_sec + 15) return;	// XXX:

    switch (ie->code) {
    case BTN_0: misc->acc   =  ie->value; break;
    case BTN_2: misc->door  =  ie->value; break;
    case BTN_3: misc->power = !ie->value; break;
    }	if (!(ie->value && misc->guard)) return;	// XXX:

    leds_path[20] = '1';	xopen_xwrite_close(leds_path, "0");
    switch (ie->code) {
    case BTN_0:	// 非法点火
	if (misc->alarm == ALARM_START) break; else
	    misc->alarm  = ALARM_START, misc->acnt = 1;
	gprs_send_alarm(*(int*)gprs->data, gprs, GPRS_START_ALARM);
	send_alarm_sms(gsmfd, misc->mn, "975e6cd570b9706b");	break;

    case BTN_2:	// 车门开启
	if (misc->alarm == ALARM_DOOR) break; else
	    misc->alarm  = ALARM_DOOR,  misc->acnt = 1;
	gprs_send_alarm(*(int*)gprs->data, gprs, GPRS_DOOR_ALARM);
	send_alarm_sms(gsmfd, misc->mn, "8f6695e85f00542f");	break;

    case BTN_3:	// 脚刹踩动 / 意外掉电 / （后备箱开启）
	if (misc->alarm == ALARM_POWER) break; else
	    misc->alarm  = ALARM_POWER,	misc->acnt = 1;
	gprs_send_alarm(*(int*)gprs->data, gprs, GPRS_POWER_ALARM);
	leds_path[20] = '2';	xopen_xwrite_close(leds_path, "0");
	//send_alarm_sms(gsmfd, misc->mn, "610f591663897535");	break;
	send_alarm_sms(gsmfd, misc->mn, "811a52398e2952a8");	break;

    case BTN_4:	// 车身震动
	if (misc->alarm == ALARM_SHAKE) break; else
	    misc->alarm  = ALARM_SHAKE, misc->acnt = 1;
	gprs_send_alarm(*(int*)gprs->data, gprs, GPRS_SHAKE_ALARM);
	send_alarm_sms(gsmfd, misc->mn, "8f668eab970752a8");	break;

    default: ;
    }	leds_path[20] = '1';	xopen_xwrite_close(leds_path, "1");
}

void stdi_recv(int fd, struct gsmd_priv* priv)
{
    int len;

//#define iobuf bb_common_bufsiz1
    if ((len = safe_read(fd, iobuf, sizeof(iobuf))) < 1) {
	signalled = SIGHUP;	return;
    }	//full_write(STDOUT_FILENO, iobuf, len);

    iobuf[len - 1] = 0x0d;	//iobuf[len++] = 0x0a;
    full_write(*(int*)priv->data, iobuf, len);	// XXX:
}


int raw_seriel_init(const char* path, speed_t speed, struct termios* tio)
{
    struct termios new_tio;	int fd;

    fd = open_or_warn(path, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) ; else if (fcntl(fd, F_SETFL, 0) < 0) ;

    if (!tio) tio = &new_tio;
    if (tcgetattr(fd, tio) < 0) ; else
    if ( tio == &new_tio) tio->c_lflag &= ~(ECHO | ECHONL); else {
	new_tio = *tio;		tio = &new_tio;		cfmakeraw(tio);
    }

if (0) {
    tio->c_lflag &= ~(ISIG | ICANON | ECHO | IEXTEN);
    tio->c_iflag &= ~(BRKINT | IXON | ICRNL);
    tio->c_oflag &= ~(ONLCR);
    tio->c_cc[VMIN]  = 1;
    tio->c_cc[VTIME] = 0;

    tio->c_lflag |= ICANON;	// XXX:
    tio->c_cc[VEOL] = HSCP_CMSG_SEND;
}

    cfsetspeed(tio, tty_value_to_baud(speed));
    if (tcsetattr(fd, TCSAFLUSH, tio) < 0) ;
    //if (tcflush(fd, TCIOFLUSH) < 0) ;

    return fd;
}


#ifdef	BB_VER
int mmstcar_main(int argc, char* argv[]) MAIN_EXTERNALLY_VISIBLE;
int mmstcar_main(int argc, char* argv[])
#else
int main(int argc, char* argv[])
#endif//BB_VER
{
    speed_t speed;
    unsigned opts;
    const char* path;
    struct pollfd pfd[6];

    struct hscp_priv* hscp = &hscp_priv;
    struct gpsc_priv* gpsc = &gpsc_priv;
    struct gsmd_priv* gsmd = &gsmd_priv;
    struct misc_priv* misc = &misc_priv;

#ifdef	EV_H__
    ev_prepare prepare;
    ev_fork fork_watcher;
    ev_io gpsc_watcher, gsmd_watcher, hscp_watcher;
    ev_io btni_watcher, avpc_watcher, stdi_watcher;
    ev_periodic periodic_watcher;
    ev_signal signal_watcher;
    ev_async async_watcher;
    ev_timer timer_watcher;
    ev_child child_watcher;
    ev_stat stat_watcher;
    ev_check check;
    ev_idle idle;

    evlp = ev_default_loop(EVFLAG_AUTO);
#endif//EV_H__

    //setlocale(LC_CTYPE, "");
    //if (!setenv("TZ", "cst-8", 0)) tzset();
    putenv((char*)"TZ=UTC0");  	// XXX:

    fdprintf(STDOUT_FILENO, PACKAGE_NAME " - %s"
	    "\n\n  written by  " AUTHOR_STRING "\n\n"
	    "Copyright (c) 2008.  QiXiang, Inc.  "
	    "All rights reserved.\n\n", VERSION_STRING);

    bb_signals(0 + (1 << SIGHUP) + (1 << SIGINT) + (1 << SIGTERM)
		 + (1 << SIGPIPE), signal_handler);

    // XXX: initialization
    leds_path[20] = '2';	xopen_xwrite_close(leds_path, "1");
    misc->guard = 0x01;		misc->power = 0x01;
    set_guard(misc);		misc->hint  = 120;
    misc->oa.os = 120.f;	misc->oa.dl = 31;
    misc->rg.tl.lon = 180.f * 100;	strcpy(misc->mn, "13959740094");

    gsmd->spkt.stx = GPRS_CPKT_STX;	gsmd->spkt.ver = GPRS_CPKT_VER;
    if (1) strcpy(gsmd->spkt.tid, "1010");	// XXX: "13866162440"
    else   strcpy(gsmd->spkt.tid, "1011");	// XXX: "15905515539"

    nmea_parser_init(&gpsc->parser);	gpsc->info.fix = NMEA_FIX_BAD;
    //nmea_zero_INFO(&gpsc->info);

    // TODO: parse other command line arguments
    //opt_complementary = "";
    opts = getopt32(argv, "i:"/*"g:s:p:"*/,
	    &path/*, &gprs_dev, &hscp_dev, gpsc_dev*/);
    //argc -= optind;	argv += optind;

    if (opts & 0x01) {
	strcpy(gsmd->sip,   strtok((char*)path, ":"));
	strcpy(gsmd->sport, strtok(NULL, ":"));
    } else {
	strcpy(gsmd->sip, "61.136.186.75");
	strcpy(gsmd->sport, "8000");
    }	strcpy(gsmd->apn, "CMNET");

    speed = 19200;
#ifdef	i386
    path = "/dev/ttyUSB0";
#else// XXX:
    path = "/dev/ttySAC2";
#endif
    pfd[0].fd = raw_seriel_init(path, speed, &hscp->tio);
    pfd[0].events = POLLIN;	hscp->data = &pfd[0].fd;	// XXX:

    speed = 4800;
#ifdef	i386
    path = "/dev/ttyUSB1";
#else// XXX:
    path = "/dev/ttySAC0";
#endif
    pfd[1].fd = raw_seriel_init(path, speed, &gpsc->tio);
    pfd[1].events = POLLIN;	//gpsc->data = &pfd[1].fd;

    speed = 115200;
#ifdef	i386
    path = "/dev/ttyS0";
#else// XXX:
    path = "/dev/ttySAC1";
#endif
    pfd[2].fd = raw_seriel_init(path, speed, &gsmd->tio);
    pfd[2].events = POLLIN;	gsmd->data = &pfd[2].fd;

    path = "/dev/input/event0";
    pfd[3].fd = open_or_warn(path, O_RDWR | O_NOCTTY | O_NONBLOCK);
    pfd[3].events = POLLIN;

    // TODO: pfd[4] for pipe of media player
    pfd[4].fd = STDIN_FILENO;	pfd[4].events = POLLIN;

    pfd[5].fd = STDIN_FILENO;	pfd[5].events = POLLIN;
    speed = sizeof(pfd) / sizeof(pfd[0]);
    if (0 && isatty(STDIN_FILENO)) { struct termios tio;
	if (tcgetattr(STDIN_FILENO, &tio) < 0) ; else cfmakeraw(&tio);
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &tio) < 0) ;
    }	if (!isatty(STDIN_FILENO)) --speed;
    // XXX: don't read STDIN for daemon usage

    if (0) {	// XXX:
//#define iobuf bb_common_bufsiz1
	nmeaINFO* info = &gpsc->info;

	info->lat = 3151.817; info->lon = 11716.892;	// XXX: HeFei
	info->speed = 60;     info->direction = 123;

	info->utc.year = 108; info->utc.mon = 7;  info->utc.day = 23;
	info->utc.hour = 14;  info->utc.min = 49; info->utc.sec = 35;

	hscp_update_status(*(int*)hscp->data, hscp, info, (void*)iobuf);
    }

    // XXX: delete all read, sent and un-send SMS
    full_write(*(int*)gsmd->data, "AT+CMGDA=1\r", 11);
    full_write(*(int*)gsmd->data, "AT+CMGDA=3\r", 11);
    full_write(*(int*)gsmd->data, "AT+CMGDA=4\r", 11);
    //full_write(*(int*)gsmd->data, "AT+CMGDA=6\r", 11);

    full_write(*(int*)gsmd->data, "AT+CIPSTATUS\r", 13);	// XXX:
    while (!signalled && -1 < (argc = safe_poll(pfd, speed, poll_time))) {
	if (!argc) { if (!hscp_resend(hscp)) poll_time = -1;	continue; }

	if (pfd[0].revents) hscp_recv(pfd[0].fd, hscp);
	if (pfd[1].revents) gpsc_recv(pfd[1].fd, gpsc);
	if (pfd[2].revents) gsmd_recv(pfd[2].fd, gsmd);
	if (pfd[3].revents) btni_recv(pfd[3].fd, NULL);
	// TODO: pfd[4] for pipe of media player
	if (pfd[5].revents) stdi_recv(pfd[5].fd, gsmd);		// XXX:
    }

dprintn(signalled); return 0;
#ifdef	EV_H__
    ev_set_io_collect_interval(evlp, 0.05);

    ev_io_init(&hscp_watcher, hscp_callback, pfd[0].fd, EV_READ);
		hscp_watcher.data = hscp;
    ev_io_start(evlp, &hscp_watcher);

    ev_io_init(&gpsc_watcher, gpsc_callback, pfd[1].fd, EV_READ);
		gpsc_watcher.data = gpsc;
    ev_io_start(evlp, &gpsc_watcher);

    ev_io_init(&gsmd_watcher, gsmd_callback, pfd[2].fd, EV_READ);
		gsmd_watcher.data = gsmd;
    ev_io_start(evlp, &gsmd_watcher);

    ev_io_init(&btni_watcher, btni_callback, pfd[3].fd, EV_READ);
		btni_watcher.data = NULL;
    ev_io_start(evlp, &btni_watcher);

    // TODO: avpc_watcher for pipe of media player

    ev_io_init(&stdi_watcher, stdi_callback, STDIN_FILENO, EV_READ);
		stdi_watcher.data = gsmd;	// XXX:
    ev_io_start(evlp, &stdi_watcher);

    ev_timer_init(&timer_watcher, timer_callback, .2, .2);
		hscp->repeat = &timer_watcher;
    //ev_timer_start(evlp, &timer_watcher);

    ev_loop(evlp, 0);
#endif//EV_H__

    nmea_parser_destroy(&gpsc->parser);
    //ev_default_destroy();

    return 0;
}

/*  Build instructions:
    gcc -DNDEBUG -O3 -Wall -pipe -o mmstcar mmstcar.c -lnmea \
	    -Wno-strict-aliasing -DCONFIG_PROPERTY \
	    #-D_GNU_SOURCE #-DDEBUG #-lev
 */
// vim:ts=8:sts=4:
