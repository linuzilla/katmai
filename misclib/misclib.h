/*
 *	misclib.h
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#ifndef __MISC_LIBRARIES_H_
#define __MISC_LIBRARIES_H_


struct in_addr;

int	chomp (char *buffer);
int	rtrim (char *buffer);
int	ltrim (char *buffer);
int	strip_dos_eof (char *buffer);
int	fork_and_exec (void (*func)(void));
int	local_addr (const int sock, const char *intf, struct in_addr *myaddr);
void	timer_clear (void);
int	timer_started (void);
double	timer_ended (void);
double	timer_snap (const int n); 

int	lprintf (const int level, const char *format, ...);
FILE *  lprintf_setting (const int level, char *fp);
int 	lprintf_mask (const int mask);

#endif
