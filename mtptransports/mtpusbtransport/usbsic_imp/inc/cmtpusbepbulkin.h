// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef CMTPUSBEPBULKIN_H
#define CMTPUSBEPBULKIN_H

#include "cmtpusbepbase.h"

class CMTPUsbConnection;
class MMTPType;

/**
Implements the USB MTP device class bulk-in endpoint data transfer controller.
@internalComponent
 
*/
class CMTPUsbEpBulkIn: public CMTPUsbEpBase
    {
public:

    static CMTPUsbEpBulkIn* NewL(TUint aId, CMTPUsbConnection& aConnection);
    ~CMTPUsbEpBulkIn();

    void SendBulkDataL(const MMTPType& aData);

private: // From CMTPUsbEpBase

    void SendDataCompleteL(TInt aError, const MMTPType& aSource);
    
private:

    CMTPUsbEpBulkIn(TUint aId, CMTPUsbConnection& aConnection);
    };
    
#endif // CMTPUSBEPBULKIN_H
