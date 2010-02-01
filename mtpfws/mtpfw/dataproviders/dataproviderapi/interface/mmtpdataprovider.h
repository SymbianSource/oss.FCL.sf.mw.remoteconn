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
 @publishedPartner
 @released
*/

#ifndef MMTPDATAPROVIDER_H
#define MMTPDATAPROVIDER_H

#include <badesca.h>
#include <e32base.h>
#include <mtp/mtpdataproviderapitypes.h>

class MMTPConnection;
class MMTPDataProviderFramework;
class TMTPTypeEvent;
class TMTPTypeRequest;

/** 
Defines the MTP data provider Service Provider Interface (SPI).
@publishedPartner
@released
*/
class MMTPDataProvider
    {
public:

    /**
    Cancels the most recent outstanding request made to the MTP data provider 
    service provider interface. If there is no such request outstanding, then 
    no action is taken.
    */
    virtual void Cancel() = 0;
    
    /**
    Notifies the data provider that an asynchronous event dataset has been 
    received on the specified MTP connection.
    @param aEvent The MTP event dataset.
    @param aConnection The MTP connection on which the event was received.
    @leave One of the system wide error code, if a processing failure occurs
    in the data provider.
    */
    virtual void ProcessEventL(const TMTPTypeEvent& aEvent, MMTPConnection& aConnection) = 0;

    /**
    Notifies the data provider of a significant event which has been detected. 
    Additional details regarding the event may optionally be provided in the 
    accompanying notification parameter block.
    @param aNotification The notification type identifier.
    @param aParams The notification type specific parameter block
    @leave One of the system wide error code if a processing failure occurs
    in the data provider.
    */
    virtual void ProcessNotificationL(TMTPNotification aNotification, const TAny* aParams) = 0;
    
    /**
    Initiates an MTP transaction phase processing sequence in the data 
    provider. The data provider must respond by issuing a signal to the MTP 
    data provider framework layer interface that is appropriate for the request
    type and current phase (as specified by aPhase) of the MTP transaction 
    being processed, as follows:
    
    MTP Transaction Phase	Valid MMTPDataProviderFramework Signal
    ---------------------   --------------------------------------
	ERequestPhase			ReceiveDataL, SendDataL, SendResponseL
	EResponsePhase			SendResponseL
	ECompletingPhase		TransactionCompleteL
     
    @param aPhase The MTP transaction phase to be processed.
    @param aRequest The MTP request dataset of the active MTP transaction.
    @param aConnection The MTP connection on which the transaction is being 
    processed.
    @see MMTPDataProviderFramework::ReceiveDataL
    @see MMTPDataProviderFramework::SendDataL
    @see MMTPDataProviderFramework::SendResponseL
    @see MMTPDataProviderFramework::TransactionCompleteL
    @leave One of the system wide error code if a processing failure occurs
    in the data provider.
    */
    virtual void ProcessRequestPhaseL(TMTPTransactionPhase aPhase, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection) = 0;
    
    /**
    @Deprecated.
    Initiates an object store enumeration sequence to enumerate all MTP data 
    objects which reside on the specified storage and for which the data 
    provider is responsible. The data provider is expected to invoke the 
    appropriate @see MMTPObjectMgr methods to enumerate all MTP data objects 
    for which it is responsible and which it wishes to have presented to a 
    connected MTP initiator. The data provider must complete the sequence by 
    signalling @see ObjectEnumerationCompleteL to the data provider framework 
    layer interface.
    @param aStorageId The MTP StorageID of the storage to be enumerated. A 
    value of KMTPStorageAll indicates that all storages should be enumerated.
    @leave One of the system wide error code if a processing failure occurs
    in the data provider.
    @see MMTPDataProviderFramework::ObjectEnumerationCompleteL
    */	
    IMPORT_C virtual void StartObjectEnumerationL(TUint32 aStorageId);
    
    /**
    Initiates a storage enumeration sequence. The data provider is expected to 
    invoke the appropriate @see MMTPStorageMgr methods to enumerate all MTP 
    storages for which it is responsible and which it wishes to have presented 
    to a connected MTP initiator. The data provider must complete the sequence 
    by signalling @see StorageEnumerationCompleteL to the data provider 
    framework layer interface.
    @leave One of the system wide error code if a processing failure occurs
    in the data provider.
    @see MMTPDataProviderFramework::StorageEnumerationCompleteL
    */	
    virtual void StartStorageEnumerationL() = 0;
    
    /**
    Provides the set of MTP datacodes of the specified category that are 
    supported by the data provider.
    @param aCategory The requested MTP datacode category.
    @param aArray On successful completion, the set of MTP datacodes that are 
    supported by the data provider.
    */  
    virtual void Supported(TMTPSupportCategory aCategory, RArray<TUint>& aCodes) const = 0;
    
    /**
    Provides the MTP string values of the specified category that is supported 
    by the data provider. 
    @param aCategory The requested MTP datacode category.
    @param aStrings On successful completion, the set of string data that are 
    supported by the data provider.
    @leave One of the system wide error code if a processing failure occurs
    in the data provider.
    */  
    IMPORT_C virtual void SupportedL(TMTPSupportCategory aCategory, CDesCArray& aStrings) const;
    
    /**
    Provides an MTP data provider extension interface implementation 
    for the specified interface Uid. 
    @param aInterfaceUid The unique identifier for the extension interface being 
    requested.
    @return Pointer to an interface instance or 0 if the interface is not 
    supported. Ownership is NOT transfered.
    */
    IMPORT_C virtual TAny* GetExtendedInterface(TUid aInterfaceUid);
    
protected:

    /**
    Provides a handle to the MTP data provider framework layer interface.
    @return Handle of the MTP data provider framework layer interface. 
    Ownership is NOT transfered.
    */
    virtual MMTPDataProviderFramework& Framework() const = 0;
    
public:
    /**
    Initiates an object store enumeration sequence to enumerate all MTP data 
    objects which reside on the specified storage and for which the data 
    provider is responsible. The data provider is expected to invoke the 
    appropriate @see MMTPObjectMgr methods to enumerate all MTP data objects 
    for which it is responsible and which it wishes to have presented to a 
    connected MTP initiator. The data provider must complete the sequence by 
    signalling @see ObjectEnumerationCompleteL to the data provider framework 
    layer interface.
    @param aStorageId The MTP StorageID of the storage to be enumerated. A 
    value of KMTPStorageAll indicates that all storages should be enumerated.
    @param aPersistentFullEnumeration It is used by only persistent data providers,
    it indicates whether the persistent data providers should do the object full-enumreation or not. 
    @leave One of the system wide error code if a processing failure occurs
    in the data provider.
    @see MMTPDataProviderFramework::ObjectEnumerationCompleteL
    */	
	IMPORT_C virtual void StartObjectEnumerationL(TUint32 aStorageId, TBool aPersistentFullEnumeration);
    
    };

#endif // MMTPDATAPROVIDER_H
