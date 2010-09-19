//#!/usr/bin/tcc -run
/****************************************************************
 * $ID: strapui.c      Îå, 17  9ÔÂ 2010 17:15:41 +0800  mhfan $ *
 *                                                              *
 * Description:                                                 *
 *                                                              *
 * Maintainer:  ·¶ÃÀ»Ô(MeiHui FAN)  <mhfan@ustc.edu>            *
 *                                                              *
 * CopyLeft (c)  2010  M.H.Fan                                  *
 *   All rights reserved.                                       *
 *                                                              *
 * This file is free software;                                  *
 *   you are free to modify and/or redistribute it   	        *
 *   under the terms of the GNU General Public Licence (GPL).   *
 ****************************************************************/

//kbuild:#lib-y :=
//kbuild:lib-$(CONFIG_STRAPUI) += strapui.o
//kbuild:EXTRA_CFLAGS += #-Wno-multichar #-Wno-unused -Wstrict-aliasing=2
//kbuild:EXTRA_CFLAGS += -I/usr/local/include
//kbuild:
//config:config STRAPUI
//config:       bool "strapui"
//config:       default n
//config:       help
//config:         Firmware utiltility for creating, appending, dumping
//config:         and flashing usage.
//config:
//applet:IF_STRAPUI(APPLET(strapui, _BB_DIR_USR_SBIN, _BB_SUID_DROP))
//applet:
//usage:#define strapui_trivial_usage
//usage:        "[OPTIONS]"
//usage:#define strapui_full_usage "\n\n"
//usage:        "Options:"
//usage-     "\n        TODO"
//usage:

#include "libbb.h"

#include "strapui.h"

#if 0

Design:

* BootMenu
    - Ubuntu
    - Android
    - External (SD, USB, Net...)

* UpgradeUI

* TSCalUI

#endif

int strapui_main(int argc, char* argv[]) MAIN_EXTERNALLY_VISIBLE;
int strapui_main(int argc, char* argv[])
{
    twin_format_t fmt = TWIN_ARGB32;	// XXX:

    strapui_priv *sui, strapui;
    twin_pixmap_t *pix;
    twin_screen_t *scr;
    twin_t* twin;
    int hx, hy;
    char* path;

    twin_feature_init();
    strapui_init(sui = &strapui);

    TWIN_FIXED_GOLDEN = //(sqrtf(5.f) - 1.f) / 2;
	    (twin_fixed_sqrt(twin_int_to_fixed(5)) - TWIN_FIXED_ONE) >> 1;

    (void)argc; (void)argv;

#if 0	// FIXME:
    opt_complementary =
	    "p::d--nias:n--diac:i--ndas:a--ndis:n?vp:d?p:i:n:d:a:h-";
    opts = getopt32(argv, "d:i:n:a:p:v:s:ch", &fwh->path,
	    &fwh->path, &fwh->path, &fwh->path, &compath, &vers, &sign);
    //argc -= optind;	argv += optind;
#endif

#if USE_X11
    hx = STRAPUI_SCREEN_WIDTH, hy = STRAPUI_SCREEN_HEIGHT;

    if (!(twin = twin_x11_create(XOpenDisplay(0), hx, hy)))
	bb_error_msg_and_die("Failed to create X11 screen!");

    scr = twin->screen;
#else
    if (!(twin = twin_fbdev_create(-1, SIGUSR1)))
	bb_error_msg_and_die("Failed to create FBdev screen!");

    scr = twin->screen;

    //twin_linux_evdev_create(NULL, scr);	// FIXME:
    twin_linux_mouse_create(NULL, scr);
    //twin_linux_js_create(scr);

    path = strapui_spath(sui, STRAPUI_CURSOR);
    if (!(pix = twin_load_X_cursor(path, 0, &hx, &hy))) {
	bb_error_msg("Failed to load: %s\n"
		"Using default cursor!", path);
	  pix = twin_get_defult_cursor(&hx, &hy);
    }	strapui_spath_clean(sui, path);

    if (pix) twin_screen_set_cursor(scr, pix, hx, hy);
#endif

    if (1) bb_info_msg("Twin screen info.: %ux%u@32",
	    scr->width, scr->height);

    path = strapui_spath(sui, STRAPUI_BACKGROUND);
    pix  = strapui_pixmap(path, fmt, scr->width, scr->height);
    strapui_spath_clean(sui, path);

    twin_screen_set_background(scr, pix);

    if ((path = strapui_spath(sui, STRAPUI_VENDOR_LOGO))) {
	twin_operand_t sop, mop;

	pix = strapui_pixmap(path, fmt, 0, 0);

	pix->x = center_golden(scr->width,  -pix->width);
	pix->y = center_golden(scr->height, -pix->height);

	mop.source_kind = TWIN_SOLID;
	mop.u.argb = 0x8f000000;

	sop.u.pixmap = pix;
	sop.source_kind = TWIN_PIXMAP;

	if (1) bb_info_msg("Rendering vendor-logo: %u, %u", pix->x, pix->y);

      if (0) twin_pixmap_show(pix, scr, scr->top); else if (1) { // XXX:
	twin_window_t* win;

	win = twin_window_create(scr, fmt, TwinWindowPlain,
		pix->x, pix->y, pix->width, pix->height);

	twin_composite(win->pixmap, 0, 0, &sop, 0, 0,
		&mop, 0, 0, TWIN_SOURCE, pix->width, pix->height);
	twin_pixmap_destroy(pix);

	twin_window_show(win);
      } else {
	twin_composite(scr->background, pix->x, pix->y, &sop, 0, 0,
		&mop, 0, 0, TWIN_OVER,  pix->width, pix->height);
	twin_screen_damage(scr, hx, hy, pix->width, pix->height);

	twin_pixmap_destroy(pix);
      }
    }	strapui_spath_clean(sui, path);

    sui->twin = twin;

    // TODO:

#if USE_X11
#else
    twin_fbdev_activate(twin);
#endif

    twin_dispatch();

    strapui_exit(sui);

#if USE_X11
    twin_x11_destroy(twin);
#else
    twin_fbdev_destroy(twin);
#endif

    return 0;
}

#ifndef strapui_spath
void strapui_spath_clean(strapui_priv* sui, char* path)
{
    sui->dpfx[sui->dend] = '\0';
    sui->dpfx[sui->send] = '\0';
return;
    free(path);
}

char* strapui_spath(strapui_priv* sui, const char* name)
{
    char* path;

#if 0
    uint16_t l, t, m, n;
    l = strlen(sui->dpfx);
    m = strlen(sui->skin);
    t = l + 1 + m + 1 + strlen(name) + 1;
#else// XXX:
    uint16_t l, t;

    t = sui->send + 1 + strlen(name) + 1;
#endif

    if (PATH_MAX - 1 < t) {
	bb_error_msg("Path length exceeds %u: %s",
		PATH_MAX, name);	return NULL;
    }

    if ((path = sui->dpfx)) || (path = malloc_or_warn(t)) {
	if (1) l = sui->dend; else
	;//strcpy(&path[0], sui->dpfx);
	strcpy(&path[l++], "/");
	if (1) l = sui->send; else
	;//strcpy(&path[l], sui->skin), l += m;
	strcpy(&path[l++], "/");
	strcpy(&path[l], name);

	if (access(path, R_OK)) {
	    bb_simple_perror_msg(path);	return NULL;
	}
    }

    return path;
}
#endif

twin_pixmap_t* strapui_pixmap(const char* path,
	twin_format_t fmt, int dstw, int dsth)
{
    twin_pixmap_t* pix;

    if (1) bb_info_msg("Loading pixmap: %s", path);

    if (1 && strncmp(strrchr(path, '.') + 1, "png", 3))
	pix = twin_jpeg_to_pixmap(path, fmt); else
	pix = twin_png_to_pixmap (path, fmt);

    if (!pix) {
	bb_error_msg("Failed to load: %s\n"
		"Using default pixmap pattern!", path);
	pix = twin_make_pattern();	// XXX:
    } else if (!dstw || !dsth) {
	if (1) bb_info_msg("Original pixmap size: %ux%u",
		pix->width, pix->height);
    } else if (1)

    if (dstw != pix->width || dsth != pix->height) {
	twin_operand_t sop;
	twin_fixed_t sx, sy;

	sx = twin_fixed_div(twin_int_to_fixed(pix->width),
			    twin_int_to_fixed(dstw));
	sy = twin_fixed_div(twin_int_to_fixed(pix->height),
			    twin_int_to_fixed(dsth));
	twin_matrix_scale(&pix->transform, sx, sy);

	sop.source_kind = TWIN_PIXMAP;
	sop.u.pixmap = pix;

	if (1) bb_info_msg("Scaling pixmap: %ux%u -> %ux%u",
		pix->width, pix->height, dstw, dsth);

	if (!(pix = twin_pixmap_create(fmt, dstw, dsth)))
	    bb_error_msg_and_die("Fail to create new pixmap!");

	twin_composite(pix, 0, 0, &sop, 0, 0, NULL, 0, 0,
		TWIN_SOURCE, dstw, dsth);

	twin_pixmap_destroy(sop.u.pixmap);
    }

    return pix;
}

twin_fixed_t TWIN_FIXED_GOLDEN = twin_double_to_fixed(0.618);
int center_golden(int range, int length)
{
    twin_fixed_t prop = TWIN_FIXED_GOLDEN;

    if (range < 0) {
	range = -range;
	return (range - length) >> 1;
	prop = TWIN_FIXED_HALF;
    }

    if (length < 0) {
	length = -length;
	prop = TWIN_FIXED_ONE - prop;
    }

    return  twin_fixed_to_int(
	    twin_fixed_mul(twin_int_to_fixed(range), prop) -
	    twin_fixed_mul(twin_int_to_fixed(length), TWIN_FIXED_ONE - prop));
}

int strapui_init(strapui_priv* sui)
{   // TODO:
    (void)sui;
    return 0;
}

int strapui_exit(strapui_priv* sui)
{   // TODO:
    (void)sui;
    return 0;
}

// vim:sts=4:ts=8:
