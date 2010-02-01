// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @publishedPartner
 @released
*/
#ifndef MMTPDEVDPEXTN_H_
#define MMTPDEVDPEXTN_H_

#include <e32base.h>
#include <e32cons.h>
#include <e32cmn.h>
#include <ecom/ecom.h>
#include <mtp/mtpdataproviderapitypes.h>
#include <mtp/tmtptyperequest.h>
#include <mtp/mtpprotocolconstants.h>


class MExtnDevicePropDp
{
public:
/**
This interface method is designed to implement GetDevicePropValue (Operation Code EMTPOpCodeGetDevicePropValue).
This operation returns the current value of the particular device property.
@param aPropCode : device property code eg: PlaybackObject(D411)
@param aMMTPtype : out param memory need to be allocated by the implementation.
				   Owner ship of this param should be with allocator.
*/
virtual TInt GetDevPropertyL(const TMTPDevicePropertyCode aPropCode, MMTPType** aMMTPtype) = 0;


/**
This interface method is designed to implement GetDevicePropDesc (Operation Code EMTPOpCodeGetDevicePropDesc).
This operation returns the current property desc of the particular device property.
@param aPropCode : device property code eg: PlaybackObject(D411)
@param aMMTPtype : out param memory need to be allocated by the implementation.
				   Owner ship of this param should be with allocator.
*/
virtual TInt GetDevPropertyDescL(const TMTPDevicePropertyCode aPropCode, MMTPType** aMMTPtype) = 0;

/**
@param aMMTPtype : out param memory need to be allocated by the implementation.
				   Owner ship of this param should be with allocator.
SetDevicePropertyL function uses the allocated memory to set the device property when 
a EMTPOpCodeSetDevicePropValue operation code comes.				   

*/
virtual TInt GetDevicePropertyContainerL(TMTPDevicePropertyCode aPropCode, MMTPType** aMMTPtype) = 0;

/**
This method sets the device property implemented by extension plugin.
*/
virtual TMTPResponseCode SetDevicePropertyL() = 0;

/**
This method returns the supported properties by extension plugin.
*/
virtual void Supported (TMTPSupportCategory aCategory, RArray <TUint>& aArray, TMTPOperationalMode aMode) = 0;

/**
This interface method is designed to implement RessetDevicePropValue. 
@param aPropCode : device property code .
*/
virtual TInt ResetDevPropertyL(const TMTPDevicePropertyCode aPropCode) = 0;

};


#endif /*MMTPDEVDPEXTN_H_*/
