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

#ifndef CMTPGETDEVICEPROPVALUE_H
#define CMTPGETDEVICEPROPVALUE_H

#include "cmtpgetdevicepropdesc.h"

class CMTPTypeString;
class TMTPTypeUint8;

/** 
Implements the device data provider GetDevicePropValue request processor.
@internalComponent
*/
class CMTPGetDevicePropValue : public CMTPGetDevicePropDesc
    {
    
public:

    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);    
    ~CMTPGetDevicePropValue();    
    
private:    

    CMTPGetDevicePropValue(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);
    void ConstructL();
    
private: // From CMTPGetDevicePropDesc

    void ServiceBatteryLevelL();
    void ServiceDeviceFriendlyNameL();
    void ServiceSynchronisationPartnerL();   
    void ServiceSessionInitiatorVersionInfoL();
    void ServicePerceivedDeviceTypeL();	
    void ServiceDateTimeL();
    void ServiceDeviceIconL();
    void ServiceSupportedFormatsOrderedL();
    void ServiceFunctionalIDL();
    void ServiceModelIDL();
    void ServiceUseDeviceStageL();
   void HandleExtnServiceL(TInt aPropCode, MExtnDevicePropDp* aExtnDevplugin);  //2113

private: // Owned

    TMTPTypeUint8   iBatteryLevel;
    TMTPTypeUint32 iUint32;	
    CMTPTypeString* iString;
    TMTPTypeUint8   iUint8;	
    TMTPTypeGuid*   iData;
    /*
    ownership of this pointer is belongs to devicedatastore so should not 
    delete this pointer.
    */
    const CMTPTypeArray* iMtpArray; 	
    };
    
#endif // CMTPGETDEVICEPROPVALUE_H

