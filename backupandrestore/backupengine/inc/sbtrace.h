/**
* Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:
* Main entry point in the process.
* 
*
*/



/**
 @file
*/

#ifndef __SBTRACE_H__
#define __SBTRACE_H__

#define LEAVEIFERROR(err, trace) \
{ \
TInt munged_err=(err); \
if (munged_err < 0) \
{ \
trace; \
User::Leave(munged_err);\
}\
}
	
#endif //__SBTRACE_H__
