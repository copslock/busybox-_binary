/* vi: set sw=8 ts=8: */
/*
 * This file suffers from chronically incorrect tabification
 * of messages. Before editing this file:
 * 1. Switch you editor to 8-space tab mode.
 * 2. Do not use \t in messages, use real tab character.
 * 3. Start each source line with message as follows:
 *    |<7 spaces>"text with tabs"....
 * or
 *    |<5 spaces>"\ntext with tabs"....
 */
#ifndef BB_USAGE_H
#define BB_USAGE_H 1

#define NOUSAGE_STR "\b"

INSERT

#ifndef mke2fs_full_usage
#define mke2fs_full_usage	mkfs_ext2_full_usage
#define mke2fs_trivial_usage	mkfs_ext2_trivial_usage
#endif// XXX:

#define busybox_notes_usage \
       "Hello world!\n"

#endif
