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
 @internalComponent
 */


#ifndef CMTPPICTBRIDGEDPGETOBJECTPROPLIST_H_
#define CMTPPICTBRIDGEDPGETOBJECTPROPLIST_H_

#include "cmtprequestprocessor.h"
#include "cmtppictbridgedp.h"
#include "rmtpdpsingletons.h"

class CMTPTypeArray;
class CMTPTypeObjectPropList;

/** 
Implements the picturebridge data provider GetObjectPropList request processor.
*/
class CMTPPictBridgeDpGetObjectPropList : public CMTPRequestProcessor
    {
public:
    static MMTPRequestProcessor* NewL( MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, CMTPPictBridgeDataProvider& aDataProvider);
    ~CMTPPictBridgeDpGetObjectPropList();
    
private:
    CMTPPictBridgeDpGetObjectPropList( MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection, CMTPPictBridgeDataProvider& aDataProvider);
    void ConstructL();
    
private://from CMTPRequestProcessor
    void ServiceL();
    TMTPResponseCode CheckRequestL();
    
private:

    TMTPResponseCode CheckPropCode() const;
    TMTPResponseCode CheckDepth() const;
    
    void GetObjectHandlesL();
    void GetObjectHandlesL( TUint32 aStorageId, TUint32 aFormatCode, TUint32 aParentHandle );
    void GetRootObjectHandlesL( TUint32 aFormatCode, TUint32 aDepth );
    
    void ServiceAllPropertiesL(TUint32 aHandle, TBool aDiscoveryFile );
    void ServiceGroupPropertiesL(TUint32 aHandle,TUint16 aGroupCode, TBool aDiscoveryFile );
    void ServiceOneObjectPropertyL(TUint32 aHandle, TUint32 aPropCode, TBool aDiscoveryFile );
    
private: // Owned
    CMTPPictBridgeDataProvider&   iPictBridgeDP;    
    CMTPTypeArray*          iHandles;
    CMTPTypeObjectPropList* iPropertyList;
    RMTPDpSingletons		iDpSingletons; 
	CMTPObjectMetaData*		iObjMeta;
	TEntry 					iFileEntry;

    };

#endif /* CMTPPICTBRIDGEDPGETOBJECTPROPLIST_H_ */

