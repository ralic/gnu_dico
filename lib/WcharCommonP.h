/* $Id: WcharCommonP.h,v 1.1 2001/11/03 22:34:29 gray Exp $ */
/*  Wchar Widgets
 *  Copyright (C) 1998 Gray
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _WcharCommonP_h_
#define _WcharCommonP_h_

#ifndef __PROTO
#if NeedFunctionPrototypes		     
#define __PROTO(c) c
#else
#define __PROTO(c) ()
#endif
#endif

#define DEBUG

#define FormatNarrow 0
#define FormatWide 1
#define MaxFormat 2

#include <assert.h>
extern void _wchar_internal_error __PROTO((char*, int, char*));
#define WcharInternalError(text) \
 _wchar_internal_error(__FILE__, __LINE__, text);

#endif
