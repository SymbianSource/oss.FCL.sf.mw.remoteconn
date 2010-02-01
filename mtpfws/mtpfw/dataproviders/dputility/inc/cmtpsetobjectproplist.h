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

#ifndef CMTPSETOBJECTPROPLIST_H
#define CMTPSETOBJECTPROPLIST_H

#include "cmtprequestprocessor.h"
#include "rmtpframework.h"
#include "rmtpdpsingletons.h"

class CMTPTypeObjectPropList;

/** 
Implements the file data provider SetObjectPropsList request processor.
@internalComponent
*/
class CMTPSetObjectPropList : public CMTPRequestProcessor
    {
public:

    IMPORT_C static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);    
    IMPORT_C ~CMTPSetObjectPropList();    
    
private:

    CMTPSetObjectPropList(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);
    void ConstructL();

private: // From CMTPRequestProcessor

    void ServiceL();
    TBool DoHandleResponsePhaseL();
    TBool HasDataphase() const;
    
private:

    TMTPResponseCode CheckPropCode(TUint16 aPropertyCode, TUint16 aDataType) const;
    TMTPResponseCode CheckObjectHandleL(TUint32 aHandle) const;

private:
    RMTPDpSingletons		iDpSingletons;
    CMTPTypeObjectPropList* iPropertyList;    
    RMTPFramework			iSingleton;	
	CMTPObjectMetaData*		iObjMeta;
    };
    
#endif // CMTPSETOBJECTPROPLIST_H

