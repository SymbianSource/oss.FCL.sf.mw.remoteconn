// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef CMTPIMAGEDPSETOBJECTPROPLIST_H
#define CMTPIMAGEDPSETOBJECTPROPLIST_H

#include "cmtprequestprocessor.h"

class CMTPImageDpObjectPropertyMgr;
class CMTPTypeObjectPropList;
class CMTPImageDataProvider;

/** 
Implements the file data provider SetObjectPropsList request processor.
*/
class CMTPImageDpSetObjectPropList : public CMTPRequestProcessor
    {
public:

    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider);    
    ~CMTPImageDpSetObjectPropList();    
    
private:

    CMTPImageDpSetObjectPropList(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider);
    void ConstructL();

private: // From CMTPRequestProcessor

    void ServiceL();
    TBool DoHandleResponsePhaseL();
    TBool HasDataphase() const;
    
private:

    TMTPResponseCode CheckPropCode(TUint16 aPropertyCode, TUint16 aDataType) const;

private:

    /**
    FLOGGER debug trace member variable.
    **/
    __FLOG_DECLARATION_MEMBER_MUTABLE;
    
    CMTPImageDataProvider&         iDataProvider;
    CMTPImageDpObjectPropertyMgr&  iPropertyMgr;
    CMTPTypeObjectPropList* iPropertyList;
    CMTPObjectMetaData*		iObjectMeta;
    };
    
#endif // CMTPIMAGEDPSETOBJECTPROPLIST_H

