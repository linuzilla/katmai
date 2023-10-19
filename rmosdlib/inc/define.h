/*****************************************************************************/
/*
 *      define.h -- REALmagic simple windowing application in GTK
 *
 *      Copyright (C) 2000-2002 Sigma Designs
 *                    written by Nicolas Vignal <nicolas_vignal@sdesigns.com>
 *                    modified by Emmanuel Michon <emmanuel_michon@sdesigns.com>
 *                                Pascal Cannenterre <pascal_cannenterre@sdesigns.com>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*****************************************************************************/

#ifndef __DEFINE_H__
#define __DEFINE_H__

#include "osthread.h"

#include <signal.h>

/*
#include <X11/Xlib.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <glib.h>
*/

#ifdef _VOD_
#include "pch_externalapi.h"
#include "streaming.h"
#endif

/* Analog overlay settings library. */
#include "settingsheader.h"
#include "handlemedia.h"
#include "osdcontrol.h"
// #include "handledisc.h"
#include "fmpovlctrl.h"
#include "fmpcontrol.h"

#ifdef DEBUG
#define G_MESSAGE(args...) g_message(args)
#define G_WARNING(args...) g_warning(args)
#define G_PRINT(args...)   g_print(args)
#else
#define G_MESSAGE(args...)
#define G_WARNING(args...)
#define G_PRINT(args...)
#endif

#ifndef UNUSED_ARG
#define UNUSED_ARG 0xdeadbeef
#endif

#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif

typedef struct _GuiElement_
{
  GtkWidget *window;
  GtkWidget *statusbar;
  GtkWidget *timer;
  GtkWidget *repeat_statusbar;
  GtkWidget *keypad_statusbar;
  GtkWidget *filew;
  GtkWidget *stream_window;
  GtkWidget *options;
  GtkWidget *dvd_options;
  GtkWidget *playlistwin;
  GtkWidget *msg_window;
  GtkItemFactory *item_factory;
  guint context_id;
  guint repeat_context_id;
  guint timer_id;
  guint keypad_context_id;
  gint timer_tag;
  gint playlist_runner_tag;
  gint osd_delay_tag;
  guint refresh_interval;
  gint playlist_selected_row;
  guint playlist_length;
  gint window_width;
  gint window_height;
}GuiElement;

extern INFO Info;
extern FMPOVERLAYVDO ovl;
extern GuiElement GUI;

/* specific defines for the GUI */
#ifdef _VOD_
#define GTKPLAYERNAME "REALmagic GTK+ Media Player"
#else
#define GTKPLAYERNAME "REALmagic GTK+ DVD Player"
#endif

#define WAIT_FOR_FMPCLOSE 300000 //microseconds
#define TIMER_REFRESHING_DELAY 1000 //milliseconds
#define PLAYLIST_NEXT_FILE_DELAY 500 //milliseconds 
#define USER_SEEK_TIME 0xfeed
#define SEEK_TIME_ACCURACY 5 // seconds
#define SEEK_BAR_DEFAULT_LENGTH 3600 // seconds
#define SEEK_BAR_REFRESHING_INTERVAL 3 // seconds
#define KEYPAD_NUMBER_OF_DIGITS 2
#define DVD_PGLEVEL_PASSWD_MAX_LENGTH 8

#endif /* DEFINE   */
