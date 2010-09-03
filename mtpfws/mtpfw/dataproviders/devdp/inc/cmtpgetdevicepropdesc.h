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

#ifndef CMTPGETDEVICEPROPDESC_H
#define CMTPGETDEVICEPROPDESC_H

#include "cmtprequestprocessor.h"
#include "rmtpdevicedpsingletons.h"

#include "mmtpdevdpextn.h"

//#define  value for Unordered format 0 for unordered and 1 for ordered
#define FORMAT_UNORDERED 0x00
#define FORMAT_ORDERED 0x01

//define here for supporting metadata service, not support now
const TUint64 KUseDeviceStageH = 0;
const TUint64 KUseDeviceStageL = 0;

class CMTPTypeDevicePropDesc;
class CRepository;
/** 
Implements the device data provider GetDevicePropDesc request processor.
@internalComponent
*/
class CMTPGetDevicePropDesc : public CMTPRequestProcessor
    {

public:

    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);    
    ~CMTPGetDevicePropDesc();    
  //  void SetExtnDevicePropDp(MExtnDevicePropDp* aExtnDevicePropDp);
   // MExtnDevicePropDp* ExtnDevicePropDp();

    
protected:    

    CMTPGetDevicePropDesc(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);
    void ConstructL();
    
    // From CMTPRequestProcessor
    virtual TMTPResponseCode CheckRequestL();
public:
    TMTPTypeGuid* GetGUIDL( const TUint aKey);
    void SaveGUID( const TUint aKey, TMTPTypeGuid& aValue );
private: // From CMTPRequestProcessor

    void ServiceL();
    
	void DoCancel();
	void RunL();

private:

    virtual void ServiceBatteryLevelL();
    virtual void ServiceDeviceFriendlyNameL();
    virtual void ServiceSynchronisationPartnerL();
    virtual void ServiceSessionInitiatorVersionInfoL();
    virtual void ServicePerceivedDeviceTypeL();
    virtual void ServiceDateTimeL();
    virtual void ServiceDeviceIconL();
    virtual void ServiceSupportedFormatsOrderedL();
    virtual void ServiceFunctionalIDL();
    virtual void ServiceModelIDL();
    virtual void ServiceUseDeviceStageL();
    virtual void HandleExtnServiceL(TInt aPropCode, MExtnDevicePropDp* aExtnDevplugin );

protected:	
	virtual TUint8 GetFormatOrdered();
protected: // Owned

 //	 MExtnDevicePropDp* iExtnDevicePropDp;
  
    /**
    The current battery level.
    */
    TUint                   iBatteryLevelValue;
    
    /**
    The device DP singletons handle.
    */
    RMTPDeviceDpSingletons  iDpSingletons;   
    
  
    
private: // Owned.
    
    CMTPTypeDevicePropDesc* iPropDesc;
    TMTPTypeGuid*           iData;
    TUint32                 iPropCode;
    CRepository*            iRepository;
   //	MMTPType* mtptype;
     };
    
#endif

