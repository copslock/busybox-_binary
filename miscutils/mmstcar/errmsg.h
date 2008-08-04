/****************************************************************
 * $ID: errmsg.h          Thu Oct 30 11:07:30 2003 CST  mhfan $ *
 *                                                              *
 * Description:                                                 *
 *                                                              *
 * Maintainer:  ∑∂√¿ª‘(MeiHui FAN)  <mhfan@ustc.edu>            *
 *                                                              *
 * Maintainer:  M.H.Fan  <mhfan@ustc.edu>			*
 *              Laboratory of Structural Biology                *
 *              School of Life Science                          *
 *              Univ. of Sci.& Tech. of China (USTC)	        *
 *              People's Republic of China (PRC)                *
 *                                                              *
 * CopyRight (c)  2005~2009  M.H.Fan				*
 *   All rights reserved.                                       *
 ****************************************************************/
#ifndef	ERRMSG_H
#define ERRMSG_H

#ifdef	__linux__
#include <unistd.h>   // __unix__
#endif//__linux__

#ifdef	__STDC__
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <limits.h>
#else//__cplusplus
#include <ctype>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <cassert>
#include <climits>

using std::strerror;
using std::fprintf;
using std::stderr;
using std::errno;

//using std::__FILE__;
//using std::__LINE__;
//using std::__TIME__;
//using std::__DATE__;
#endif//__STDC__

/*
 * NOTE: the "do {" ... "} while (0);" bracketing around the macros
 * allows the err_abort and errno_abort macros to be used as if they
 * were function calls, even in contexts where a trailing ";" would
 * generate a null statement. For example,
 *
 *      if (status != 0) err_abort (status, "message");
 *      else return status;
 *
 * will not compile if err_abort is a macro ending with "}", because
 * C does not expect a ";" to follow the "}". Because C does expect
 * a ";" following the ")" in the do...while construct, err_abort and
 * errno_abort can be used as if they were function calls.
 */

#ifdef	DEBUG
/*
 * Define a macro that can be used for diagnostic output from
 * examples. When compiled -DDEBUG, it results in calling printf
 * with the specified argument list. When DEBUG is not defined, it
 * expands to nothing.
 */

// for tracing & debugging

#undef	dtrace
#undef	dprintn
#undef	dprints
#undef	dprintp
#undef	fprintf

#if	defined(__KERNEL__)
#define errno 0 // XXX:
#define fprintf(_, ...) printk(__VA_ARGS__)
#elif	defined(LIBBB_H)
#define fprintf(_, ...) fdprintf(STDERR_FILENO, __VA_ARGS__)
#elif	defined(AVUTIL_LOG_H)
#define fprintf(_, ...) av_log(NULL, AV_LOG_INFO, __VA_ARGS__)
#elif	defined(MPLAYER_MP_MSG_H)
#define fprintf(_, ...) mp_msg(MSGT_GLOBAL, MSGL_INFO, __VA_ARGS__)
#endif// XXX:

#define dtrace	do { fprintf(stderr, \
	    "\033[36mTRACE\033[1;34m==>\033[33m%16s" \
	    "\033[36m: \033[32m%4d\033[36m: \033[35m%-24s \033[34m" \
	    "[\033[0;37m%s\033[1;34m, \033[0;36m%s\033[1;34m]\033[0m\n", \
	    __FILE__, __LINE__, __func__, __TIME__, __DATE__); \
	    if (errno < 0) fprintf(stderr, "Errmsg: %s (%d)\n", \
		    strerror(errno), errno); \
	} while (0)

#define dprintp(a, n) do { unsigned short i_, m_ = sizeof((a)[0]); \
	fprintf(stderr, "\033[33m" #a ": \033[36m" \
		"%p\033[0m ==> %x\n", a, (n)); \
	m_ = (m_ < 2 ? 24 : (m_ < 4 ? 16 : 8)); \
	for (i_ = 0; i_ < (n); ) { \
	    unsigned short j_ = ((n) < i_ + m_ ? (n) - i_ : m_); \
	    for ( ; j_--; ++i_) \
		if (16 < m_) fprintf(stderr, "%02x ", (a)[i_]); else \
		if ( 8 < m_) fprintf(stderr, "%04x ", (a)[i_]); else \
			     fprintf(stderr, "%08x ", (a)[i_]); \
	    fprintf(stderr, "\n"); } \
	} while (0)

#define dprintn(a) do { fprintf(stderr, "\033[33m" #a \
		": \033[36m%#x, %d, %g\033[0m\n", a, a, (double)a); \
	} while (0)	// XXX:

#define dprints(a) do { fprintf(stderr, "\033[33m" #a \
		": \033[36m%s\033[0m\n", a); } while (0)


#ifdef	SANITY_WRAP
#define	sanity_fswrap(func, ...) ( { ssize_t  t_; \
	while ((t_ = func(__VA_ARGS__)) < 0) { \
	    fprintf(stderr, "Fail to `" #func "' on %s/%d: %s (%d).\n", \
		    __FILE__, __LINE__, strerror(errno), errno); \
	    if (errno != EAGAIN && errno != EINTR) break; } \
	t_; } )

#define	sanity_iowrap(func, fd, buf, len) ( { \
	ssize_t  n_, m_ = 0;/* assert(-1 < fd && buf && 0 < len);*/ \
	do  while ((n_ = func(fd, \
		(unsigned char*)buf + m_, len - m_)) < 0) { \
	    fprintf(stderr, "Fail to " #func \
		    ": %s/%d: %s (%d); (%x/%x)\n", __FILE__, __LINE__, \
		    strerror(errno), errno, (unsigned)m_, (unsigned)len); \
	    if (errno != EAGAIN && errno != EINTR) break; } \
	while (0 < n_ && (m_ += n_) < len); m_; } )

#define	sanity_opwrap(func, fd, ...) ( { \
	if ((fd = func(__VA_ARGS__)) < 0) \
	    fprintf(stderr, "Fail to `" #func "' on %s/%d: %s (%d).\n", \
		    __FILE__, __LINE__, strerror(errno), errno); fd; } )

#define	sanity_mkdir(p_, m_) ( { int r_; \
	if (mkdir(p_, m_) && errno != EEXIST) \
	    fprintf(stderr, "Fail to mkdir `%s': %s (%d).\n", \
		    p_, strerror(errno), errno); r_; } )

#define	sanity_malloc(s_, p_) ( { if (!(p_ = malloc(s_))) \
	    fprintf(stderr, "Fail to allocate `%d' bytes memory: %s(%d)\n", \
		   s_, strerror(errno), errno); p_; } )
#else
#define	sanity_opwrap(func, fd, ...)	(fd = func(__VA_ARGS__))
#define	sanity_fswrap(func, ...)	func(__VA_ARGS__)
#define	sanity_iowrap(func, ...)	func(__VA_ARGS__)
#define	sanity_malloc(s, p)		(p = malloc(s))
#define	sanity_mkdir			mkdir
#endif//SANITY_WRAP

#define	sanity_close(fd) if (-1 < fd) { \
	sanity_fswrap(posix_fadvise, fd, 0, 0, \
		POSIX_FADV_DONTNEED); close(fd);/* fd = -1;*/ \
	}

#else//!DEBUG
#undef	dtrace
#undef	dprintn
#undef	dprints
#undef	dprintp
#endif//DEBUG

// for the standard Pthread errors.
#define err_abort(code,text) do { fprintf(stderr, \
		"\033[0;1;33m%s\033[34m at \"\033[4;37m%s\033[0;1;34m\":" \
		"\033[36m%d\033[34m: \033[31m%s\033[0;37m " \
		"(\033[1;35m%s\033[0;37m)\n", text, __FILE__, __LINE__, \
	    strerror(code), code); abort(); \
	} while (0)

// for the standard classic errors.
#define errno_abort(text) err_abort(errno,text)

#ifdef	__cplusplus
#include <new>

#if 0
static void err_new_handl()
{
    fprintf(stderr, "\033[0;1;35mError\033[34m at \"\033[4;37m%s\033[0;1;34m"
	    "\":\033[36m%d\033[34m: \033[31mNo enough memory!\033[0;37m\n",
	__FILE__, __LINE__);	abort();
}
#endif

//std::set_new_handler(err_new_handl);


// TODO: static assert (loki)

#endif//__cplusplus

#endif//ERRMSG_H
// vim:sts=4:ts=8:
