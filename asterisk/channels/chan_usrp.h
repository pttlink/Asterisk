/* GNU Radio Interface Channel Driver for app_rpt/Asterisk
 *
 * chan_usrp.h - Version 200511
 *
 * Copuright (C) 2010 Max Parke, KA1RBI
 *
 * All Rights Reserved
 * Licensed under the GNU GPL v2 (see below)
 * 
 * Refer to AUTHORS file for listing of authors/contributors to app_rpt.c and other related AllStar programs
 * as well as individual copyrights by authors/contributors.  Unless specified or otherwise assigned, all authors and
 * contributors retain their individual copyrights and license them freely for use under the GNU GPL v2.
 *
 * Notice:  Unless specifically stated in the header of this file, all changes
 *          are licensed under the GNU GPL v2 and cannot be relicensed. 
 *
 * The AllStar software is the creation of Jim Dixon, WB6NIL with serious contributions by Steve RoDgers, WA6ZFT
 * 
 * This software is based upon and dependent upon the Asterisk - An open source telephone toolkit
 * Copyright (C) 1999 - 2006, Digium, Inc.
 *
 * See http://www.asterisk.org for more information about the Asterisk project. Please do not directly contact
 * any of the maintainers of this project for assistance; the project provides a web site, mailing lists and IRC
 * channels for your use.
 *
 * License:
 * --------
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * ------------------------------------------------------------------------
 * This program is free software, distributed under the terms of the GNU General Public License Version 2. See the LICENSE file
 * at the top of the source tree for more information.
 *
 */

#define USRP_VOICE_FRAME_SIZE (160*sizeof(short))  // 0.02 * 8k

enum { USRP_TYPE_VOICE=0, USRP_TYPE_DTMF, USRP_TYPE_TEXT };

// udp data header 
struct _chan_usrp_bufhdr {
	char eye[4];		// verification string
	uint32_t seq;		// sequence counter
	uint32_t memory;	// memory ID or zero (default)
	uint32_t keyup;		// tracks PTT state
	uint32_t talkgroup;	// trunk TG id
	uint32_t type;		// see above enum
	uint32_t mpxid;		// for future use
	uint32_t reserved;	// for future use
};
