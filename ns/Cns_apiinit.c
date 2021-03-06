/*
 * Copyright (C) 1999-2000 by CERN/IT/PDP/DM
 * All rights reserved
 */
 
#ifndef lint
static char sccsid[] = "@(#)Cns_apiinit.c,v 1.3 2000/05/29 11:38:41 CERN IT-PDP/DM Jean-Philippe Baud";
#endif /* not lint */

/*	Cns_apiinit - allocate thread specific or global structures */

#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include "Cglobals.h"
#include "Cns_api.h"
#include "serrno.h"
#include<sys/stat.h>
static int Cns_api_key = -1;

int DLL_DECL
Cns_apiinit(struct Cns_api_thread_info **thip)
{
	Cglobals_get (&Cns_api_key,
	    (void **) thip, sizeof(struct Cns_api_thread_info));
	if (*thip == NULL) {
		serrno = ENOMEM;
		return (-1);
	}
	if(! (*thip)->initialized) {
		umask ((*thip)->mask = umask (0));
		(*thip)->initialized = 1;
	}
	return (0);
}

int DLL_DECL *
C__Cns_errno()
{
struct Cns_api_thread_info *thip;
	Cglobals_get (&Cns_api_key,
	    (void **) &thip, sizeof(struct Cns_api_thread_info));
	return (&thip->ns_errno);
}
