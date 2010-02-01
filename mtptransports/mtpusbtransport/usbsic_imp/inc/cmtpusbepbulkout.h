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

#ifndef CMTPUSBEPBULKOUT_H
#define CMTPUSBEPBULKOUT_H

#include "cmtpusbepbase.h"

class CMTPUsbConnection;
class MMTPType;

/**
Implements the USB MTP device class bulk-out endpoint data transfer controller.
@internalComponent
 
*/
class CMTPUsbEpBulkOut: public CMTPUsbEpBase
    {
public:

    static CMTPUsbEpBulkOut* NewL(TUint aId, CMTPUsbConnection& aConnection);
    ~CMTPUsbEpBulkOut();
    
    void ReceiveBulkDataL(MMTPType& aData);

private: // From CMTPUsbEpBase

    void ReceiveDataCompleteL(TInt aError, MMTPType& aSink);
    
private:

    CMTPUsbEpBulkOut(TUint aId, CMTPUsbConnection& aConnection);
    };
    
#endif // CMTPUSBEPBULKOUT_H