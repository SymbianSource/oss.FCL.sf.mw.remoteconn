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

#ifndef CMTPDEVICEDATASTORE_H
#define CMTPDEVICEDATASTORE_H

#include <e32base.h>
#include <etel3rdparty.h>
#include <mtp/tmtptypeuint32.h>
#include <mtp/cmtptypearray.h>

#include "mtpdebug.h"
#include "rmtpframework.h"
#include "mmtpdevdpextn.h"
#include"cmtpextndevdp.h"

class CMTPTypeString;
class MMTPEnumerationCallback;
class RFs;
class RReadStream;
class RWriteStream;

/** 
Implements the MTP device data provider's device information data store.
@internalComponent
*/
class CMTPDeviceDataStore : public CActive
   {    

public:

    static CMTPDeviceDataStore* NewL();
    ~CMTPDeviceDataStore();
    
    TBool RequestPending() const; 
	void BatteryLevelL(TRequestStatus& aStatus, TUint& aBatteryLevel);
    const TDesC& DeviceFriendlyName() const;
    const TDesC& DeviceFriendlyNameDefault() const;
    const TDesC& DeviceVersion() const;
    const TDesC& Manufacturer() const;
    const TDesC& Model() const;
    const TDesC& MTPExtensions() const;
    const TDesC& SerialNumber() const;
    const TDesC& SynchronisationPartner() const;
    const TDesC& SynchronisationPartnerDefault() const;
    
    void SetDeviceFriendlyNameL(const TDesC& aName);
    void SetSynchronisationPartnerL(const TDesC& aName);
    
    void StartEnumerationL(TUint32 aStorageId, MMTPEnumerationCallback& aCallback);
    const TDesC& SessionInitiatorVersionInfo() const;
    const TDesC& SessionInitiatorVersionInfoDefault() const;
    void SetSessionInitiatorVersionInfoL(const TDesC& aName);
    TUint32 PerceivedDeviceTypeDefault() const;
    TUint32 PerceivedDeviceType() const;
    const TDesC& DateTimeL();
    TInt SetDateTimeL(const TDesC& aDateTime );    
    void DateTimeToStringL(TDes& aDateTime);
    void StringToDateTime(TDes& aDateTime );
    void LoadDeviceIconL();
    const CMTPTypeArray& DeviceIcon();
    TInt ValidateString(const TDesC& aDateTimeStr, TDes& aDateTime, TInt &aOffsetVal);

	

	/*utility methods */
	void SetSupportedDevicePropertiesL(RArray<TUint> &aSupportedDevProps);
	const CMTPTypeArray & GetSupportedDeviceProperties();	
	void SetExtnDevicePropDp(MExtnDevicePropDp* aExtnDevicePropDp);
	MExtnDevicePropDp* ExtnDevicePropDp();

private: // Form CActive

    void DoCancel();
    TInt RunError(TInt aError);
    void RunL();
    void CheckDeviceIconProperties( RArray<TUint> &aSupportedDeviceProperties);
    
private:

    CMTPDeviceDataStore();
    void ConstructL();

    TBool Enumerated() const;
    void ExternalizeL(RWriteStream& aWriteStream) const;
    void InternalizeL(RReadStream& aReadStream);
    const TDesC& PropertyStoreName();
    void AppendMTPExtensionSetsL(TBool& aCompleted);
    void RestoreL();
    void Schedule(TInt32 aState);
    void SetRequestComplete(TRequestStatus& aRequest, TUint aErr);
    void SetRequestPending(TRequestStatus& aRequest);
    void SetState(TInt32 aState);
    TInt32 State() const;
    void StoreL();
    void StoreFormattedSerialNumber(const TDesC& aSerialNo);
    
private: // Owned

    enum TState    
        {
        EUndefined                      = 0x00000000,
        EEnumeratingDevicePropertyStore = 0x00000001,
        EEnumeratingDeviceVersion       = 0x00000002,
        EEnumeratingPhoneId             = 0x00000003,
        EEnumeratingBatteryLevel        = 0x00000004,
        EEnumeratingVendorExtensions	= 0x00000005,
        EEnumerated                     = 0x80000000,
        EEnumeratedBatteryLevel         = 0x80000004,
        };
    
private: // Owned

    /**
    FLOGGER debug trace member variable.
    */
    __FLOG_DECLARATION_MEMBER_MUTABLE;

    CTelephony::TBatteryInfoV1 		iBatteryInfoV1;
    CTelephony::TBatteryInfoV1Pckg  iBatteryInfoV1Pckg;  
    CMTPTypeString*                 iDeviceFriendlyName;
    RBuf                            iDeviceVersion;
    RMTPFramework                   iSingletons;
    RBuf                			iMTPExtensions; 
    CTelephony::TPhoneIdV1          iPhoneIdV1;
    CTelephony::TPhoneIdV1Pckg      iPhoneIdV1Pckg; 
    TFileName                       iPropertyStoreName;  
    RBuf                            iSerialNumber;
    TInt32                          iState;
    TUint32                         iStorageId;
    CMTPTypeString*                 iSynchronisationPartner; 
    CTelephony*                     iTelephony;
    HBufC*							iDeviceFriendlyNameDefault;
    HBufC*							iSyncPartnerNameDefault;
    TInt							iCurrentDpIndex;
    //HBufC*						iSessionInitiatorVersionInfoDefault;
    CMTPTypeString*  				iSessionInitiatorVersionInfo;
    TMTPTypeUint32  				iPerceivedDeviceType;
    CMTPTypeString*  				iDateTime;
    CMTPTypeArray*  				iDeviceIcon; 

    // this array is to store all supported device properties.
    CMTPTypeArray*                                   iSupportedDevProArray;
    TUint8			iFormatOreder; 
    
private: // Not owned.

    MMTPEnumerationCallback* 	    iCallback;
    TRequestStatus*                 iPendingStatus;
    TUint*                          iPendingBatteryLevel;
    
    MExtnDevicePropDp*              iExtnDevicePropDp;
    };
   
#endif // CMTPDEVICEDATASTORE_H
