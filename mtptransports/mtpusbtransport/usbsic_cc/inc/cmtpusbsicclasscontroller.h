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
#ifndef CMTPUSBSICCLASSCONTROLLER_H
#define CMTPUSBSICCLASSCONTROLLER_H

#include <e32std.h>
#include <cusbclasscontrollerplugin.h>
#include <mtp/rmtpclient.h>

class MUsbClassControllerNotify;
class TUsbDescriptor;

/**
Implements the USB MTP Class Controller API 
@internalComponent
*/
class CMTPUsbSicClassController : public CUsbClassControllerPlugIn
	{

public: 

	static CMTPUsbSicClassController* NewL(MUsbClassControllerNotify& aOwner);
	~CMTPUsbSicClassController();

private: // From CActive.

	void RunL();
	void DoCancel();
	TInt RunError(TInt aError);

private: // From CUsbClassControllerBase

	void Start(TRequestStatus& aStatus);
	void Stop(TRequestStatus& aStatus);

	void GetDescriptorInfo(TUsbDescriptor& aDescriptorInfo) const;

private:

	CMTPUsbSicClassController(MUsbClassControllerNotify& aOwner);
	void ConstructL();

private:

	RMTPClient iMTPSession;
	};

#endif // CMTPUSBSICCLASSCONTROLLER_H

