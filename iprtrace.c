/******************************************************************/
/* Linux IBM IPR trace dump utility                               */
/* Description: IBM Storage IOA Interface Specification (IPR)     */
/*              Linux trace dump utility                          */
/*              Used to dump the internal trace of an IOA         */
/* Usage: iprtrace /dev/ipr0                                   */
/*        iprtrace /dev/scsi/host0/controller                     */
/*                                                                */
/* (C) Copyright 2000, 2001                                       */
/* International Business Machines Corporation and others.        */
/* All Rights Reserved.                                           */
/******************************************************************/

/*
 * $Header: /cvsroot/iprdd/iprutils/Attic/iprtrace.c,v 1.2 2003/10/23 01:50:54 bjking1 Exp $
 */

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mount.h>
#include <linux/types.h>
#include <scsi/scsi.h>
#include <scsi/scsi_ioctl.h>
#include <scsi/sg.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/file.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <syslog.h>
#include <term.h>

#ifndef iprlib_h
#include "iprlib.h"
#endif

struct ipr_trace_entry
{
	u32 element[4];
};

int main (int argc, char *argv[])
{
    struct ipr_ioctl_cmd_internal ioa_cmd;
    struct ipr_trace_entry *p_trace_entry;
    int num_trace_entries;
    int fd, rc, i;

    if (argc != 2)
    {
        printf("Usage: iprtrace [controller]"IPR_EOL);
        printf("  Example: iprtrace /dev/ipr0"IPR_EOL);
        return -1;
    }

    openlog("iprtrace",
            LOG_PERROR | /* Print error to stderr as well */
            LOG_PID |    /* Include the PID with each error */
            LOG_CONS,    /* Write to system console if there is an error
                          sending to system logger */
            LOG_USER);

    fd = open(argv[1], O_RDWR);

    if (fd <= 1)
    {
        syslog(LOG_ERR, "Cannot open %s. %m"IPR_EOL, argv[1]);
        return -1;
    }

    num_trace_entries = IPR_DUMP_TRACE_ENTRY_SIZE/sizeof(struct ipr_trace_entry);

    p_trace_entry = malloc(IPR_DUMP_TRACE_ENTRY_SIZE);

    memset(ioa_cmd.cdb, 0, 16);

    ioa_cmd.buffer = p_trace_entry;
    ioa_cmd.buffer_len = IPR_DUMP_TRACE_ENTRY_SIZE;
    ioa_cmd.read_not_write = 1;
    ioa_cmd.driver_cmd = 1;

    rc = ipr_ioctl(fd, IPR_GET_TRACE, &ioa_cmd);

    close(fd);

    if (rc != 0)
    {
        syslog(LOG_ERR, "Get trace failed. %m"IPR_EOL);
        return -1;
    }

    for (i = 0; i < num_trace_entries; i++)
    {
        printf("0x%08x  0x%08x  0x%08x  0x%08x\n",
               p_trace_entry[i].element[0], p_trace_entry[i].element[1],
               p_trace_entry[i].element[2], p_trace_entry[i].element[3]);
    }

    return 0;
}

