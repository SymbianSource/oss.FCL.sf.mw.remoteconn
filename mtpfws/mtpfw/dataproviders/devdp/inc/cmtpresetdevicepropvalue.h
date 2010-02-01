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

#ifndef CMTPRESETDEVICEPROPVALUE_H
#define CMTPRESETDEVICEPROPVALUE_H

#include "cmtpgetdevicepropdesc.h"
#include "mtpdebug.h"

class CMTPTypeString;
/** 
Implements the device data provider SetDevicePropValue request processor.
@internalComponent
*/
class CMTPResetDevicePropValue : public CMTPRequestProcessor
    {
    
public:

    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);    
    ~CMTPResetDevicePropValue();    

     
private:    

    CMTPResetDevicePropValue(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);
    void ConstructL();
  


private: // From CMTPRequestProcessor
    
    TMTPResponseCode CheckRequestL();

    void ServiceL();
    void HandleExtnServiceL(TInt aPropCode, MExtnDevicePropDp* aExtnDevplugin);
//	void DoCancel();
//	void RunL();    
	void ServiceSessionInitiatorVersionInfoL();
	void ServicePerceivedDeviceTypeL();
	void ServiceDateTimeL();
	void ServiceDeviceIconL();
	void ServiceSupportedFormatsOrderedL();
	void ServiceFunctionalIDL();
	void ServiceModelIDL();
	void ServiceUseDeviceStageL();
	
private:
    TMTPTypeGuid* GetGUIDL( const TUint aKey);
    void SaveGUID( const TUint aKey, TMTPTypeGuid& aValue );

private: // Owned

    /**
    FLOGGER debug trace member variable.
    */
    __FLOG_DECLARATION_MEMBER_MUTABLE;

//    CMTPTypeString* iString;
    TUint32                 iPropCode;
    TMTPTypeGuid*           iData;
    RMTPDeviceDpSingletons  iDpSingletons;  
    CRepository*            iRepository;
    };
    
#endif // CMTPResetDevicePropValue_H

