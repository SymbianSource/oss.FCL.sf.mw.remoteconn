// Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//

/**
 @file
 @internalComponent
*/

#ifndef MTPPLAYBACKCONTROLDPCONST_H
#define MTPPLAYBACKCONTROLDPCONST_H

#include <mtp/mtpprotocolconstants.h>

/**
define all the operations that are supported by the device data provider
*/
static const TUint16 KMTPPlaybackControlDpSupportedOperations[] = 
	{
	EMTPOpCodeGetDevicePropDesc,
	EMTPOpCodeGetDevicePropValue,
	EMTPOpCodeSetDevicePropValue,
	EMTPOpCodeResetDevicePropValue,
	EMTPOpCodeSkip
	};

/**
define all the device properties supported by the device data provider
*/
static const TUint16 KMTPPlaybackControlDpSupportedProperties[] = 
	{
	EMTPDevicePropCodeVolume,
	EMTPDevicePropCodePlaybackRate,
	EMTPDevicePropCodePlaybackObject,
	EMTPDevicePropCodePlaybackContainerIndex,
	EMTPDevicePropCodePlaybackPosition	
	};

static const TUint16 KMTPPlaybackControlDpSupportedEvents[] = 
	{
	EMTPEventCodeDevicePropChanged
	};

#endif // MTPPLAYBACKCONTROLDPCONST_H

