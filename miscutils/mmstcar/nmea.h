/****************************************************************
 * $ID: nmea.h         Sat, 26 Jul 2008 08:41:02 +0800  mhfan $ *
 *                                                              *
 * Description:                                                 *
 *                                                              *
 * Maintainer:  ∑∂√¿ª‘(Meihui Fan)  <mhfan@ustc.edu>            *
 *                                                              *
 * CopyRight (C)  2008~2009  M.H.Fan                            *
 *   All rights reserved.                                       *
 ****************************************************************/
#ifndef NMEA_H
#define NMEA_H	"National Marine Electronics Association"

/* References:
 * GPS software and interface descriptions:
    http://www.topology.org/soft/gps.html

 * ESR's Guide to Hacking With GPS:
    http://gpsd.berlios.de/gps-hacking.html
    http://gpsd.berlios.de/references.html
    http://gpsd.berlios.de/hacking.html

 * GPS NMEA data format description:
    http://www.gpsinformation.org/dale/nmea.htm
    http://hi.baidu.com/bg4uvr/blog/item/a829f0815517f9dcbc3e1edd.html
 */

#define NMEA_BAUD_RATE	4800	// 8n1
#define NMEA_SENT_MAXL	82
#define NMEA_SENT_STX	"$"
#define NMEA_SENT_STP	"*"
#define NMEA_TALKER_ID	"GP"

struct {
    char  msid[4];
    unsigned char numf;
    int (*func)(char*, void*);
    char* desc;
}   nmea_sent_desc[] = {
#define _(s)	s
    [NMEA_RECORD_GGA] = { "GGA", 14, NULL,
	_("GPS fixed information") },
    [NMEA_RECORD_GLL] = { "GLL",  7, NULL,
	_("Geographic position - latitude/longitude") },
    [NMEA_RECORD_GSA] = { "GSA", 17, NULL,
	_("GNSS DOP and active satellites") },
    [NMEA_RECORD_GSV] = { "GSV", 19, NULL,
	_("GNSS satellites in view") },
    [NMEA_RECORD_RMC] = { "RMC", 12, NULL,
	_("Recommended minimum specific GNSS data") },
    [NMEA_RECORD_VTG] = { "VTG", 12, NULL,
	_("Course over ground and ground speed") }
};

char* position_fix_indicators[] = {
    _("Fix not available or invalid"),
    _("GPS SPS Mode, fix valid"),
    _("Differential GPS, SPS Mode, fix valid"),
    _("GPS PPS Mode, fix valid"),
    _("Real Time Kinematic"),
    _("Float RTK"),
    _("Dead Reckoning Mode, fix valid"),
    _("Manual input mode"),
    _("Simulation mode"),
};

struct satellite_info {
    unsigned id:9, elevation:7, zaimuth:9, snr:7;
};

struct nmea_info {
    unsigned long long utc_time;
    float latitude, longitude;
    float altitude, geoid_wgs84;
    float pdop, hdop, vdop;
    float speed, course;
    float magnetic_variation;
    float age_of_diff_corr;
    unsigned char sv_inview:4, sv_inuse:4;
    unsigned char status:1, pos_fix_ind:4, mode1:1, mode2:2;
    unsigned short reserved;
    char aunit, gunit, mode;
    // Autonomous, DGPS, Estimated-DR, Not-valid, Simulator
    unsigned char diff_ref_station[5];
    struct satellite_info sv_info[12];
};

#endif//NMEA_H
// vim:sts=4:ts=8:
