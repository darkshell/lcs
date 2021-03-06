/*
 * Cupv_util.h,v 1.1 2002/05/28 09:47:41 bcouturi Exp
 */

/*
 * Copyright (C) 1999-2002 by CERN/IT/DS/HSM
 * All rights reserved
 */
 
/*
 * @(#)Cupv_util.h,v 1.1 2002/05/28 09:47:41 CERN IT-DS/HSM Ben Couturier
 */

#ifndef _CUPV_UTIL_H
#define _CUPV_UTIL_H
#include "osdep.h"
#include "Cupv_constants.h" 
#include "Cupv_struct.h"

#define INC_PTR(ptr,n)		(ptr) = (char*)(ptr) + (n)

#define  WRITE_STR(ptr,str, first)     { if (!first) { \
                                     (void) strcpy((char*)(ptr),(char*)(STR_SEP)); INC_PTR(ptr,strlen(STR_SEP)); } \
                                        (void) strcpy((char*)(ptr),(char*)(str)); \
					  INC_PTR(ptr,strlen(str)); \
                                          first = 0; }
					

/* Funtions fron Cupv_util.c */
EXTERN_C int  DLL_DECL Cupv_strtoi _PROTO((int *,char *,char **, int));
EXTERN_C void DLL_DECL Cupv_util_time _PROTO((time_t, char *));
EXTERN_C int DLL_DECL Cupv_parse_privstring _PROTO((char *));
EXTERN_C void DLL_DECL Cupv_build_privstring _PROTO((int , char *));
EXTERN_C int DLL_DECL Cupv_getuid _PROTO((const char *name));
EXTERN_C int DLL_DECL Cupv_getgid _PROTO((const char *name));
EXTERN_C char DLL_DECL *Cupv_getuname _PROTO((uid_t));
EXTERN_C char DLL_DECL *Cupv_getgname _PROTO((gid_t));

#endif






