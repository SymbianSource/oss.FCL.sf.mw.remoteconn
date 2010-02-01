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

#ifndef CMTPIMAGEDPGETOBJECTPROPLIST_H
#define CMTPIMAGEDPGETOBJECTPROPLIST_H

#include "cmtprequestprocessor.h"

class CMTPImageDpObjectPropertyMgr;
class CMTPTypeArray;
class CMTPTypeObjectPropList;
class CMTPImageDataProvider;

/** 
Implements the file data provider GetObjectPropList request processor.
@internalComponent
*/
class CMTPImageDpGetObjectPropList : public CMTPRequestProcessor
    {
public:
    
    static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider);    
    ~CMTPImageDpGetObjectPropList(); 

private: // From CMTPRequestProcessor

    void ServiceL();
    TMTPResponseCode CheckRequestL();
    
private:

    CMTPImageDpGetObjectPropList(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider);
    void ConstructL();

    TMTPResponseCode CheckPropCode() const;
    TMTPResponseCode CheckDepth() const;
    
    void GetObjectHandlesL();
    void GetObjectHandlesL(TUint32 aStorageId, TUint32 aFormatCode, TUint32 aParentHandle);
    void GetRootObjectHandlesL(TUint32 aFormatCode, TUint32 aDepth);
    
    void ServicePropertiesL( TUint32 aHandle );
    void ServiceAllPropertiesL(TUint32 aHandle);
    void ServiceGroupPropertiesL(TUint32 aHandle,TUint16 aGroupCode);
    void ServiceOneObjectPropertyL(TUint32 aHandle, TUint32 aPropCode);
    
private:
	
    /**
    * Cancel the enumeration process
    */
    void DoCancel();

    /**
    * Handle completed request
    */
    void RunL();

    /**
    * Ignore the error, continue with the next one
    */
    TInt RunError( TInt aError );

    /**
    * Control the getobjectproplist
    */
    void StartL();
	
private: // Owned

    CMTPImageDpObjectPropertyMgr&  iPropertyMgr;
    RArray<TUint>                  iHandles;
    CMTPTypeObjectPropList*        iPropertyList;  
    CMTPObjectMetaData*            iObjectMeta;
    
    /**
     FLOGGER debug trace member variable.
     */
     __FLOG_DECLARATION_MEMBER_MUTABLE;
    };
    
#endif // CMTPIMAGEDPGETOBJECTPROPLIST_H

