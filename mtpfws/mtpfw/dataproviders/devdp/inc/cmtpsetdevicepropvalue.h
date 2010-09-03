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

#ifndef CMTPSETDEVICEPROPVALUE_H
#define CMTPSETDEVICEPROPVALUE_H

#include "cmtpgetdevicepropdesc.h"

class CMTPTypeString;

/** 
Implements the device data provider SetDevicePropValue request processor.
@internalComponent
*/
class CMTPSetDevicePropValue : public CMTPGetDevicePropDesc
    {
    
public:

    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);    
    ~CMTPSetDevicePropValue();    
    
private:    

    CMTPSetDevicePropValue(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);
    void ConstructL();
  
    void CompleteServiceDeviceFriendlyNameL();   
    void CompleteServiceSynchronisationPartnerL(); 

private: // From CMTPRequestProcessor
    
    TBool DoHandleResponsePhaseL();
    TBool HasDataphase() const;
    TMTPResponseCode CheckRequestL();
    
private: // From CMTPGetDevicePropDesc

    void ServiceBatteryLevelL();
    void ServiceDeviceFriendlyNameL();
    void ServiceSynchronisationPartnerL();
    void CompleteServiceSessionInitiatorVersionInfoL();
    void ServiceSessionInitiatorVersionInfoL();
    void CompletePerceivedDeviceTypeL();
    void ServicePerceivedDeviceTypeL();
    void CompleteServiceDateTimeL();
    void ServiceDateTimeL();
    void CompleteDeviceIconL();	
    void ServiceDeviceIconL();	
    void CompleteServiceSupportedFormatsOrderedL();
    void ServiceSupportedFormatsOrderedL();
    void ServiceUseDeviceStageL();
    void CompleteServiceUseDeviceStageL();
    void ServiceModelIDL();
    void CompleteServiceModelIDL();
    void ServiceFunctionalIDL();
    void CompleteServiceFunctionalIDL();
    void HandleExtnServiceL(TInt aPropCode, MExtnDevicePropDp* aExtnDevplugin);

private: // Owned

    CMTPTypeString* iString;
    TMTPTypeUint32 iUint32;	
    TMTPTypeUint8 iUint8;	
    CMTPTypeArray* iMtparray; 
    TMTPTypeGuid* iData;

    //
	
    };
    
#endif // CMTPSETDEVICEPROPVALUE_H

