/*****************************************************************************
 * Copyright (C) 2001-2007  PEAK System-Technik GmbH
 *
 * linux@peak-system.com
 * www.peak-system.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Maintainer(s): Fabrice Vergnaud (f.vergnaud@peak-system.com)
 *
 *****************************************************************************/

/**
 * @file pcbtrace.c
 * @brief Tools to trace CAN message within PCANBasic API.
 *
 * $Id: pcbtrace.c 27 2016-01-12 14:41:33Z Fabrice $
 *
 */

#include "pcbtrace.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "pcaninfo.h"


#define PCBTRACE_MAX_MSG	500

static char * pcbtrace_hw_to_string(enum pcaninfo_hw hw);
static int pcbtrace_write_header(struct pcbtrace_ctx *ctx);

/* PRIVATE FUNCTIONS */
char * pcbtrace_hw_to_string(enum pcaninfo_hw hw) {
	char * res;

	res = "";
	switch (hw) {
	case PCANINFO_HW_DNG:
		res = "PCAN_DNGBUS";
		break;
	case PCANINFO_HW_ISA:
		res = "PCAN_ISABUS";
		break;
	case PCANINFO_HW_LAN:
		res = "PCAN_LANBUS";
		break;
	case PCANINFO_HW_PCC:
		res = "PCAN_PCCBUS";
		break;
	case PCANINFO_HW_PCI:
		res = "PCAN_PCIBUS";
		break;
	case PCANINFO_HW_PEAKCAN:
		res = "PCAN_CANBUS";
		break;
	case PCANINFO_HW_USB:
		res = "PCAN_USBBUS";
		break;
	case PCANINFO_HW_VIRTUAL:
		res = "PCAN_VIRTUALBUS";
		break;
	case PCANINFO_HW_NONE:
		res = "PCAN_NONEBUS";
		break;
	default:
		res = "UNKNOWN";
		break;
	}
	return res;
}

int pcbtrace_write_header(struct pcbtrace_ctx *ctx) {
	char buf[PCBTRACE_MAX_MSG];
	size_t n;
	time_t traw;
	struct tm *t;

	if (ctx == NULL || !ctx->status || !ctx->pfile)
		return EINVAL;

	snprintf(buf, PCBTRACE_MAX_MSG, ";$FILEVERSION=1.1\n;\n");
	n = fwrite(buf, strlen(buf), sizeof(char), ctx->pfile);
	if (n <= 0)
		return errno;


	time(&traw);
	t = localtime(&traw);
	gettimeofday(&ctx->time_start, NULL);
	snprintf(buf, PCBTRACE_MAX_MSG, ";$STARTTIME=%lu.%lu\n;\n", ctx->time_start.tv_sec, ctx->time_start.tv_usec);
	n = fwrite(buf, strlen(buf), sizeof(char), ctx->pfile);
	if (n <= 0)
		return errno;

	snprintf(buf, PCBTRACE_MAX_MSG, ";   %s\n;\n", ctx->directory);
	n = fwrite(buf, strlen(buf), sizeof(char), ctx->pfile);
	if (n <= 0)
		return errno;

	snprintf(buf, PCBTRACE_MAX_MSG, ";   Start time: %.2d/%.2d/%.4d %.2d:%.2d:%.2d.000.0\n",
		t->tm_mday, t->tm_mon, t->tm_year,t->tm_hour,t->tm_min,t->tm_sec);
	n = fwrite(buf, strlen(buf), sizeof(char), ctx->pfile);
	if (n <= 0)
		return errno;

	snprintf(buf, PCBTRACE_MAX_MSG, ";   PCAN-Channel: %s\n", ctx->chname);
	n = fwrite(buf, strlen(buf), sizeof(char), ctx->pfile);
	if (n <= 0)
		return errno;
	snprintf(buf, PCBTRACE_MAX_MSG, ";   Generated by PCAN-Basic API\n;\n");
	n = fwrite(buf, strlen(buf), sizeof(char), ctx->pfile);
	if (n <= 0)
		return errno;
	snprintf(buf, PCBTRACE_MAX_MSG, ";   Message Number\n");
	n = fwrite(buf, strlen(buf), sizeof(char), ctx->pfile);
	if (n <= 0)
		return errno;
	snprintf(buf, PCBTRACE_MAX_MSG, ";   |         Time Offset (ms)\n");
	n = fwrite(buf, strlen(buf), sizeof(char), ctx->pfile);
	if (n <= 0)
		return errno;
	snprintf(buf, PCBTRACE_MAX_MSG, ";   |         |        Type\n");
	n = fwrite(buf, strlen(buf), sizeof(char), ctx->pfile);
	if (n <= 0)
		return errno;
	snprintf(buf, PCBTRACE_MAX_MSG, ";   |         |        |        ID (hex)\n");
	n = fwrite(buf, strlen(buf), sizeof(char), ctx->pfile);
	if (n <= 0)
		return errno;
	snprintf(buf, PCBTRACE_MAX_MSG, ";   |         |        |        |     Data Length Code\n");
	n = fwrite(buf, strlen(buf), sizeof(char), ctx->pfile);
	if (n <= 0)
		return errno;
	snprintf(buf, PCBTRACE_MAX_MSG, ";   |         |        |        |     |   Data Bytes (hex) ...\n");
	n = fwrite(buf, strlen(buf), sizeof(char), ctx->pfile);
	if (n <= 0)
		return errno;
	snprintf(buf, PCBTRACE_MAX_MSG, ";   |         |        |        |     |   |\n");
	n = fwrite(buf, strlen(buf), sizeof(char), ctx->pfile);
	if (n <= 0)
		return errno;
	snprintf(buf, PCBTRACE_MAX_MSG, ";---+--   ----+----  --+--  ----+---  +  -+ -- -- -- -- -- -- --\n");
	n = fwrite(buf, strlen(buf), sizeof(char), ctx->pfile);
	if (n <= 0)
		return errno;

	return 0;
}

/* PUBLIC FUNCTIONS */
void pcbtrace_set_defaults(struct pcbtrace_ctx *ctx) {
	if (ctx == NULL)
		return;
	sprintf(ctx->directory, ".");
	ctx->flags = 0;
	ctx->maxsize = 10;
	ctx->status = PCAN_PARAMETER_OFF;
}

int pcbtrace_open(struct pcbtrace_ctx *ctx, enum pcaninfo_hw hw, uint ch_idx) {
	char filename[PCBTRACE_MAX_CHAR_SIZE];
	char *str;
	size_t len;
	time_t traw;
	struct tm *t;

	if (ctx == NULL)
		return EINVAL;
	/* check trace is already closed */
	pcbtrace_close(ctx);
	/* build filename based on time and channel */
	time(&traw);
	t = localtime(&traw);
	filename[0] = '\0';
	str = "";
	if (ctx->flags & TRACE_FILE_DATE) {
		snprintf(filename, PCBTRACE_MAX_CHAR_SIZE, "%04d%02d%02d", t->tm_year, t->tm_mon, t->tm_mday);
		str = "_";
	}
	if (ctx->flags & TRACE_FILE_TIME) {
		snprintf(filename, PCBTRACE_MAX_CHAR_SIZE, "%s%04d%02d%02d", filename, t->tm_hour, t->tm_min, t->tm_sec);
		str = "_";
	}
	snprintf(ctx->chname, PCBTRACE_MAX_CHAR_SIZE, "%s%d", pcbtrace_hw_to_string(hw), ch_idx);
	snprintf(filename, PCBTRACE_MAX_CHAR_SIZE, "%s%s%s", filename, str, ctx->chname);
	ctx->idx = 1;
	ctx->msg_cnt = 0;
	/* remove trailing '/' in location */
	len = strnlen(ctx->directory, PCBTRACE_MAX_CHAR_SIZE);
	if (len > 2 && ctx->directory[len - 1] == 0 &&
			ctx->directory[len - 2] == '/')
		ctx->directory[len - 2] = 0;
	snprintf(ctx->filename, PCBTRACE_MAX_CHAR_SIZE, "%s/%s_%d.trc", ctx->directory, filename, ctx->idx);
	/* open file and update context */
	ctx->pfile = fopen(ctx->filename, "w");
	if (ctx->pfile)
		return pcbtrace_write_header(ctx);
	return errno;
}

int pcbtrace_close(struct pcbtrace_ctx *ctx) {
	if (ctx == NULL)
		return EINVAL;
	if (ctx->pfile != NULL) {
		fclose(ctx->pfile);
		ctx->pfile = NULL;
	}
	return 0;
}

int pcbtrace_write_msg(struct pcbtrace_ctx *ctx, TPCANMsgFD *msg, int data_len, struct timeval *tv, int rx) {
	char buf[PCBTRACE_MAX_MSG];
	char * str;
	int i, n;

	if (ctx == NULL)
		return EINVAL;
	if (!ctx->status)
		return 0;
	if (ctx->pfile == NULL)
		return EBADF;

	ushort status;

	status = (msg->MSGTYPE & MSGTYPE_STATUS) > 0;
	ctx->msg_cnt++;

	/* write: "Frame_Nb)   Time_sec.micros   Direction   " */
	snprintf(buf, PCBTRACE_MAX_MSG, "%6lu)   %lu.%lu  %-5s  ",
			ctx->msg_cnt, tv->tv_sec, tv->tv_usec,
			status ? "Warng" : rx ? "Rx" : "Tx");
	n = fwrite(buf, strlen(buf), sizeof(char), ctx->pfile);
	if (n <= 0)
		return errno;
	/* write: "ID   LEN   " */
	if (msg->MSGTYPE & MSGTYPE_EXTENDED)
		snprintf(buf, PCBTRACE_MAX_MSG, "%08x  %d(%02x)  ", status ? 0xFFFFFFFF : msg->ID, data_len, msg->DLC);
	else
		snprintf(buf, PCBTRACE_MAX_MSG, "%04x  %d(%02x)  ", status ? 0xFFFFFFFF : msg->ID, data_len, msg->DLC);
	n = fwrite(buf, strlen(buf), sizeof(char), ctx->pfile);
	if (n <= 0)
		return errno;
	/* check on type content */
	if (status) {
		/* write: bus state */
		str = "";
		if(msg->DATA[3] & PCAN_ERROR_BUSLIGHT)
			str = " BUSLIGHT";
		if(msg->DATA[3] & PCAN_ERROR_BUSHEAVY)
			str = " BUSHEAVY";
		if(msg->DATA[3] & PCAN_ERROR_BUSOFF)
			str = " BUSOFF";
		snprintf(buf, PCBTRACE_MAX_MSG, "%02x %02x %02x %02x %s",
					msg->DATA[0], msg->DATA[1], msg->DATA[2],
					msg->DATA[3], str);
	}
	else {
		/* write: DATA */
		if(msg->MSGTYPE & PCAN_MESSAGE_RTR)
			snprintf(buf, PCBTRACE_MAX_MSG, "RTR");
		else {
			buf[0] = 0;
			for(i = 0; i < data_len; i++)
			{
				snprintf(buf, PCBTRACE_MAX_MSG, "%s%.2X ", buf, msg->DATA[i]);
			}
		}
	}
	n = fwrite(buf, strlen(buf), sizeof(char), ctx->pfile);
	if (n <= 0)
		return errno;

	return n;
}

int pcbtrace_write(struct pcbtrace_ctx *ctx, const char * buffer, uint size) {
	int n;

	n = fwrite(buffer, size, 1, ctx->pfile);
	if (n <= 0)
		return errno;

	n = -1;

	return n;
}

