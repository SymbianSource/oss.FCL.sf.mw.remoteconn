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

#ifndef CMTPFILEDP_H
#define CMTPFILEDP_H

#include <mtp/cmtpdataproviderplugin.h>

#include "mmtpenumerationcallback.h"
#include "mtpdebug.h"
#include "rmtpfiledpsingletons.h"
#include "rmtpdpsingletons.h"

class CMTPFileDpExclusionMgr;
class CMTPFSEnumerator;
class MMTPRequestProcessor;

/** 
Implements the file data provider plugin.
@internalComponent

*/
class CMTPFileDataProvider : 
    public CMTPDataProviderPlugin, 
    public MMTPEnumerationCallback
    {
public:

    static TAny* NewL(TAny* aParams);
    ~CMTPFileDataProvider();
    
private: // From CMTPDataProviderPlugin

    void Cancel();
    void ProcessEventL(const TMTPTypeEvent& aEvent, MMTPConnection& aConnection);
    void ProcessNotificationL(TMTPNotification aNotification, const TAny* aParams);
    void ProcessRequestPhaseL(TMTPTransactionPhase aPhase, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection);
    void StartObjectEnumerationL(TUint32 aStorageId, TBool aPersistentFullEnumeration);
    void StartStorageEnumerationL();
    void Supported(TMTPSupportCategory aCategory, RArray<TUint>& aArray) const;
         
private: // From MMTPEnumerationCallback

    void NotifyEnumerationCompleteL(TUint32 aStorageId, TInt aError);

private:
     
    CMTPFileDataProvider(TAny* aParams);
    void ConstructL();

    TInt LocateRequestProcessorL(const TMTPTypeRequest& aRequest, MMTPConnection& aConnection);
    TInt LocateRequestProcessorL(const TMTPTypeEvent& aEvent, MMTPConnection& aConnection);    
    void SessionClosedL(const TMTPNotificationParamsSessionChange& aSession);
    void SessionOpenedL(const TMTPNotificationParamsSessionChange& aSession);
    
    /**
     * [SP-Format-0x3002]Special processing for PictBridge DP which own 6 dps file with format 0x3002, 
     * but it does not really own the format 0x3002.
     * 
     * Check whether the Pictbridgedp exists or not.
     */
    TBool PictbridgeDpExistL() const;
    static void ImplementationsCleanup(TAny* aData);
    
private:
	/**
    FLOGGER debug trace member variable.
    */
    __FLOG_DECLARATION_MEMBER_MUTABLE;
    
    /**
    The active request processors table.
    */ 
    RPointerArray<MMTPRequestProcessor> iActiveProcessors;
    
    /**
    The storage enumerator.
    */
    CMTPFSEnumerator*              		iFileEnumerator;
    
    /**
    The file dp exclusion manager.
    */
    CMTPFileDpExclusionMgr*				iExclusionMgr;
    
    /**
    The list of storages to be enumerated.
    */
    RArray<TUint32>                     iPendingEnumerations;
    
    /**
    File data provider singletons
    */
    RMTPFileDpSingletons 				iFileDPSingletons;
    
    /**
    DP utility singletons
    */
    RMTPDpSingletons					iDpSingletons;
    
    TInt								iActiveProcessor;
    TBool								iActiveProcessorRemoved;

    };
    
#endif // CMTPFILEDP_H
