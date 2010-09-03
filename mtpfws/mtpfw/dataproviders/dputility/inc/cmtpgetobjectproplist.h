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

#ifndef CMTPGETOBJECTPROPLIST_H
#define CMTPGETOBJECTPROPLIST_H

#include "cmtprequestprocessor.h"
#include "rmtpdpsingletons.h"

class CMTPTypeArray;
class CMTPTypeObjectPropList;

/** 
Implements the file data provider GetObjectPropList request processor.
@internalComponent
*/
class CMTPGetObjectPropList : public CMTPRequestProcessor
    {
public:
    
    IMPORT_C static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);    
    IMPORT_C ~CMTPGetObjectPropList();    
    
private:    

private: // From CMTPRequestProcessor

    void ServiceL();
    TMTPResponseCode CheckRequestL();
    
private:

    CMTPGetObjectPropList(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);
    void ConstructL();

    TMTPResponseCode CheckFormatL() const;
    TMTPResponseCode CheckPropCode() const;
    TMTPResponseCode CheckDepth() const;
    
    void GetObjectHandlesL();
    void GetObjectHandlesL(TUint32 aStorageId, TUint32 aFormatCode, TUint32 aParentHandle);
    void GetFolderObjectHandlesL(TUint32 aFormatCode, TUint32 aDepth,TUint32 aParentHandle);
    void GetRootObjectHandlesL(TUint32 aFormatCode, TUint32 aDepth);
    void GetSpecificObjectHandlesL(TUint32 aHandle, TUint32 aFormatCode, TUint32 aDepth);
    
    void ServiceAllPropertiesL(TUint32 aHandle);
	void ServiceGroupPropertiesL(TUint32 aHandle,TUint16 aGroupCode);
    void ServiceOneObjectPropertyL(TUint32 aHandle, TUint32 aPropCode);
    
private: // Owned

    CMTPTypeArray*          iHandles;
    CMTPTypeObjectPropList* iPropertyList;   
    RMTPDpSingletons		iDpSingletons; 
    CMTPObjectMetaData*		iObjMeta;
    TEntry 					iFileEntry; 
    };
    
#endif // CMTPGETOBJECTPROPLIST_H

