// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef CMTPDATAPROVIDER_H
#define CMTPDATAPROVIDER_H

/**
@file
@internalComponent
*/

#include "rmtpframework.h"

#include <e32base.h>

#include <mtp/mmtpdataproviderframework.h>
#include <mtp/mtpdataproviderapitypes.h>
#include <mtp/tmtptypeevent.h>
#include <mtp/tmtptyperequest.h>
#include <mtp/tmtptyperesponse.h>

#include "mtpdebug.h"

class CMTPConnection;
class CMTPDataProviderPlugin;
class CMTPDataProviderConfig;
class MMTPDataProvider;
class MMTPDataProviderConfig;
class MTPFrameworkConfig;
class MMTPObjectMgr;
class MMTPTransactionProxy;
class MMTPDataCodeGenerator;

/**
Implements the API portion of the API/SPI interface pair by which MTP framework 
and data provider plug-ins interact. A unique CMTPDataProvider instance is 
created and bound to each CMTPDataProviderPlugin instance that is loaded, and 
functions on its behalf to:
    a)	Process MTP operational and media object data to and from the MTP 
        framework, and;
    b)	Manage MTP transaction protocol state transitions to ensure that the 
        MTP transaction state remains valid under both normal and error 
        recovery processing conditions.
@internalTechnology
 
*/
class CMTPDataProvider : 
    public CActive, 
    public MMTPDataProviderFramework
    {
public:

    /**
    The enumeration state bit flags.
    */
    enum TEnumerationStates
        {
        EUnenumerated               = 0x00000000,
        EObjectsEnumerationState    = 0x0000000F,
        EObjectsEnumerating         = 0x00000003,
        EObjectsEnumerated          = 0x00000001,
        EStoragesEnumerationState   = 0x000000F0,
        EStoragesEnumerating        = 0x00000030,
        EStoragesEnumerated         = 0x00000010,
        EEnumerated                 = (EStoragesEnumerated | EObjectsEnumerated)
        };
        
public:

	static CMTPDataProvider* NewL(TUint aId, TUid aImplementationUid, CMTPDataProviderConfig* aConfig);
	static CMTPDataProvider* NewLC(TUint aId, TUid aImplementationUid, CMTPDataProviderConfig* aConfig);
	~CMTPDataProvider();
 
	void ExecuteEventL(const TMTPTypeEvent& aEvent, MMTPConnection& aConnection);
	void ExecuteRequestL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection);
	IMPORT_C void ExecuteProxyRequestL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection, MMTPTransactionProxy& aProxy);
    
    void EnumerateObjectsL(TUint32 aStorageId);
    void EnumerateStoragesL();
    IMPORT_C TUint EnumerationState() const;
    IMPORT_C TUid ImplementationUid() const;
	IMPORT_C CMTPDataProviderPlugin& Plugin() const;
    IMPORT_C TBool Supported(TMTPSupportCategory aCategory, TUint aCode) const;
    IMPORT_C const RArray<TUint>& SupportedCodes(TMTPSupportCategory aCategory) const;
    
    void SetDataProviderId(TUint aId);
    
    static TInt LinearOrderUid(const TUid* aUid, const CMTPDataProvider& aObject);
    static TInt LinearOrderUid(const CMTPDataProvider& aL, const CMTPDataProvider& aR);
	static TInt LinearOrderDPId(const TUint* aDPId, const CMTPDataProvider& aObject);
    static TInt LinearOrderDPId(const CMTPDataProvider& aL, const CMTPDataProvider& aR);
    static TInt LinearOrderEnumerationPhase(const CMTPDataProvider& aL, const CMTPDataProvider& aR);
    
public: // From MMTPDataProviderFramework

    TUint DataProviderId() const;
    TMTPOperationalMode Mode() const;
    void ReceiveDataL(MMTPType& aData, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection);
    void SendDataL(const MMTPType& aData, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection);
    void SendEventL(const TMTPTypeEvent& aEvent, MMTPConnection& aConnection);
    void SendEventL(const TMTPTypeEvent& aEvent);
    void SendResponseL(const TMTPTypeResponse& aResponse, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection);
    void TransactionCompleteL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection);
    void RouteRequestRegisterL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection);
    void RouteRequestUnregisterL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection);
    void ObjectEnumerationCompleteL(TUint32 aStorageId);
    void StorageEnumerationCompleteL();
    const MMTPDataProviderConfig& DataProviderConfig() const;
    const MMTPFrameworkConfig& FrameworkConfig() const;
	MMTPObjectMgr& ObjectMgr() const;
    MMTPReferenceMgr& ReferenceMgr() const;
    MMTPStorageMgr& StorageMgr() const;
    RFs& Fs() const;
    MMTPDataCodeGenerator& DataCodeGenerator() const;
    void NotifyFrameworkL( TMTPNotificationToFramework aNotification, const TAny* aParams );
	
private: // From CActive

	void DoCancel();
	void RunL();
	TInt RunError(TInt aError);

private:

   	CMTPDataProvider(TUint aId, TUid aImplementationUid, CMTPDataProviderConfig* aConfig);
    void ConstructL();
    
	void Schedule();
  	void SendErrorResponseL(TInt aError);
  	
private:

    /**
    */
    class CSupportedCodes : public CBase
        {
    public:
    
        static CSupportedCodes* NewLC(TMTPSupportCategory aCategory, MMTPDataProvider& aDp);
        virtual ~CSupportedCodes();
    
        const RArray<TUint>& Codes() const;
        TBool Supported(TUint aCode) const;
        
    private:
    
        CSupportedCodes();
        void ConstructL(TMTPSupportCategory aCategory, MMTPDataProvider& aDp);
        
    private:
    
        RArray<TUint> iCodes;
        };
        
private: // Owned

    /**
    FLOGGER debug trace member variable.
    */
    __FLOG_DECLARATION_MEMBER_MUTABLE;
    
    /**
    The data provider configurability parameter data.
    */
	CMTPDataProviderConfig*         iConfig;
    
    /**
    The construction completed flag. This is used to indicate that ownership 
    of passed objects has been assumed.
    */
    TBool                           iConstructed;
	
	/**
	The transaction phase of the currently active operation.
	*/
    TMTPTransactionPhase            iCurrentTransactionPhase;
    
    /**
    The enumeration state.
    */
	TUint                           iEnumerationState;
	
	/**
	The error recovery flag, indicating that an error occurred while processing
	a request. 
	*/
	TInt                            iErrorRecovery;
    
    /** 
    The data provider identifier.
    */
	TInt                            iId;
	
	/**
	The data provider implementation plug-in.
	*/
	CMTPDataProviderPlugin*         iImplementation;
	
	/**
	The data provider implementation UID.
	*/
	TUid                            iImplementationUid;
	
	/**
	The transaction phase of the currently active transaction proxy.
	*/
    TMTPTransactionPhase            iProxyTransactionPhase;
    
    /**
    The MTP response dataset buffer.
    */
	TMTPTypeResponse		        iResponse;
    
    /**
    The framework singletons.
    */
	RMTPFramework                   iSingletons;
    
    /** 
    The data provider's support categories table.
    */
    RPointerArray<CSupportedCodes>  iSupported;

	
    /** Indicates whether iTimer is active. */
    TBool							iTimerActive;

    /*****
	 * Added timer to delay request handling until storage/object enumeration has finished
	 */ 	
	RTimer iTimer;
        
private: // Not owned
	
	/**
	The MTP connection on which the currently active operation is being 
	processed.
	*/
	CMTPConnection*                 iCurrentConnection;
    
    /**
    The MTP request dataset buffer.
    */
	const TMTPTypeRequest*          iCurrentRequest;
	
	/**
	The currently active transaction proxy.
	*/
	MMTPTransactionProxy*           iProxy;
    };
    
#endif // CMTPDATAPROVIDER_H
