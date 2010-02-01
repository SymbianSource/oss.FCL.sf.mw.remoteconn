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
*/


#include "cmtpextndevdpplugin.h"

static const TInt KDefaultValue = 55;

CDevDpReferncePlugin* CDevDpReferncePlugin::NewL()
    {
    CDevDpReferncePlugin* self = new(ELeave) CDevDpReferncePlugin;
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

	void  CDevDpReferncePlugin::ConstructL()
	{
	iData = new(ELeave) TMTPTypeUint32();
	}


void CDevDpReferncePlugin::Supported(TMTPSupportCategory /*aCategory*/ ,RArray <TUint>& aArray, TMTPOperationalMode aMode)
    {
    if(KMTPModeMTP == aMode)
        {
        aArray.Append(EMTPDevicePropCodePlaybackObject );  // 0xD411,
        }
    }


CDevDpReferncePlugin::CDevDpReferncePlugin()
    {
    }

CDevDpReferncePlugin::~CDevDpReferncePlugin()
     {
     delete iData;
     delete iPropDesc;
     }

/**
 returns the default value
*/
TInt  CDevDpReferncePlugin::GetDevPropertyL (const TMTPDevicePropertyCode aPropCode, MMTPType** aMMTPType)
    {
    TInt errorCode = KErrNone;
    switch(aPropCode)
        {
        case EMTPDevicePropCodePlaybackObject:
			iData->Set(KDefaultValue);
			*aMMTPType = iData;
        break;
        default:
            errorCode = KErrNotFound;
        break;
        }
    return errorCode;
    }

TInt  CDevDpReferncePlugin::GetDevPropertyDescL (const TMTPDevicePropertyCode aPropCode, MMTPType**  aMMTPType)
    {
    TInt errorCode = KErrNone;

    switch (aPropCode)
        {
        case EMTPDevicePropCodePlaybackObject:
        	delete iPropDesc;
			iPropDesc = NULL;
			iPropDesc = CMTPTypeDevicePropDesc::NewL(EMTPDevicePropCodePlaybackObject, 1, 0, NULL);
			iPropDesc->SetUint32L(CMTPTypeDevicePropDesc::EFactoryDefaultValue, 0);
	    	iPropDesc->SetUint32L(CMTPTypeDevicePropDesc::ECurrentValue, 1);
			*aMMTPType = iPropDesc;
        break;
        default:
            errorCode = KErrNotFound;
        break;
        }
    return errorCode;
    }
/**
called from ServiceL allocates the container for receiving data.
*/
TInt  CDevDpReferncePlugin::GetDevicePropertyContainerL(TMTPDevicePropertyCode aPropCode, MMTPType** aMmtpType)
    {
    TInt errorCode = KErrNone;
    iPropCode = aPropCode;
    switch (aPropCode)
        {
        case EMTPDevicePropCodePlaybackObject:
            {
            *aMmtpType = iData;
            }
        break;

        default:
            errorCode = KErrNotFound;
        break;
        }
    return errorCode;
    }


TMTPResponseCode CDevDpReferncePlugin::SetDevicePropertyL()
    {
    TInt errorCode = KErrNone;
    switch (iPropCode)
        {
        case EMTPDevicePropCodePlaybackObject:
            {
             //if  extension plugin supports more than one property. then add member a variable to 
   			 //store the incoming value in it.
             TUint int1 = (iData->Value());
            }
        break;

        default:
        errorCode = KErrNotFound;
        break;
        }

	  if(errorCode == KErrNone)
	 	 {
	     return EMTPRespCodeOK;
	  	}
	  else
	 	{
	    return EMTPRespCodeOperationNotSupported;
	  	}
    }


TInt CDevDpReferncePlugin::ResetDevPropertyL(TMTPDevicePropertyCode aPropCode)
    {
    TInt errorCode = KErrNone;
    switch (aPropCode)
        {
        case EMTPDevicePropCodePlaybackObject:
            {
            	iData->Set(KDefaultValue);// setting to default value
            }
            break;
           default:
            errorCode = KErrNotFound;
            break;
        }
    return errorCode;
    }

