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
 @internalTechnology
*/

#ifndef CDUMMYDP_H
#define CDUMMYDP_H

#include <mtp/cmtpdataproviderplugin.h>

/** 
Defines the MTP data provider component test plug-in.

@internalTechnology
*/

const TUid KFrameworkUid = {0x05E00F14};

class CDummyDp : public CMTPDataProviderPlugin
    {
public:    
	static TAny* NewL(TAny* aParams);
    ~CDummyDp();  
    
private: // From CMTPDataProviderPlugin

    void Cancel();
    void ProcessEventL(const TMTPTypeEvent& aEvent, MMTPConnection& aConnection);
    void ProcessNotificationL(TMTPNotification aNotification, const TAny* aParams);
    void ProcessRequestPhaseL(TMTPTransactionPhase aPhase, const TMTPTypeRequest& aRequest, MMTPConnection& aConnection);
    void StartObjectEnumerationL(TUint32 aStorageId, TBool aPersistentFullEnumeration);
    void StartStorageEnumerationL();
    void Supported(TMTPSupportCategory aCategory, RArray<TUint>& aArray) const;

private:
    CDummyDp(TAny* aParams);
    void ConstructL();
    
    };
    
#endif // CDUMMYDP_H
