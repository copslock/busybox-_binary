/****************************************************************
 * $ID: strapui.h      »’, 19  9‘¬ 2010 17:12:57 +0800  mhfan $ *
 *                                                              *
 * Description:                                                 *
 *                                                              *
 * Maintainer:  ∑∂√¿ª‘(MeiHui FAN)  <mhfan@ustc.edu>            *
 *                                                              *
 * CopyLeft (c)  2010  M.H.Fan                                  *
 *   All rights reserved.                                       *
 *                                                              *
 * This file is free software;                                  *
 *   you are free to modify and/or redistribute it   	        *
 *   under the terms of the GNU General Public Licence (GPL).   *
 ****************************************************************/
#ifndef STRAPUI_H
#define STRAPUI_H

#include <libtwin/twin.h>
#include <libtwin/twin_linux_mouse.h>
#include <libtwin/twin_linux_js.h>
#include <libtwin/twin_jpeg.h>
#include <libtwin/twin_png.h>

#if 1//ARCH_X86	// XXX:
#define USE_X11 1
#endif

#if USE_X11
#include <libtwin/twin_x11.h>
#define twin_t twin_x11_t
#else
#include <libtwin/twin_fbdev.h>
#define twin_t twin_fbdev_t
#endif

enum {
    STRAPUI_SCREEN_HEIGHT = 600,
    STRAPUI_SCREEN_WIDTH  = 800,

#define STRAPUI_PREFIX		"/usr/local"
#define STRAPUI_DEFAULT_SKIN	"default"

#define STRAPUI_VENDOR_LOGO	"logo.png"  // "vendor.png"
#define STRAPUI_BACKGROUND	"background.jpg"
#define STRAPUI_CURSOR		"cursor.gz"

#define STRAPUI_DATA_PATH	STRAPUI_PREFIX "/share/strapui"

#define strapui_spath(x, a) \
	(char*)STRAPUI_DATA_PATH "/skins" "/" STRAPUI_DEFAULT_SKIN "/" a
#define strapui_spath_clean(x, a)
};

#define GOLDEN_PROPORTION	(0.618)
extern twin_fixed_t TWIN_FIXED_GOLDEN;

typedef struct strapui_priv {
    twin_t *twin;

    char dpfx[PATH_MAX];
    uint16_t dend, send;
    //char* skin;   // XXX: in &dpfx[dlen + 1]
}   strapui_priv;

int strapui_init(strapui_priv* sui);
int strapui_exit(strapui_priv* sui);

#ifndef strapui_spath
void strapui_spath_clean(strapui_priv* sui, char* path);
char* strapui_spath(strapui_priv* sui, const char* name);
#endif

twin_pixmap_t* strapui_pixmap(const char* path,
	twin_format_t fmt, int dstw, int dsth);

int center_golden(int range, int length);

#endif//STRAPUI_H
// vim:sts=4:ts=8:
