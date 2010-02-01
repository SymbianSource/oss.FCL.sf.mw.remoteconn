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

#ifndef MTPOBJECTMGRQUERYTYPES_H
#define MTPOBJECTMGRQUERYTYPES_H

#include <e32def.h>
#include <mtp/mtpprotocolconstants.h> 

class RDbView;

/**
Defines the MTP data provider framework object store enumeration manager object
query parameters.
*/
class TMTPObjectMgrQueryParams
    {
public:

    IMPORT_C TMTPObjectMgrQueryParams(TUint32 aStorageId, TUint32 aFormatCode, TUint32 aParentHandle, TUint32 aDpId = KMTPDataProviderAll);

public:

    /**
    The identifier of the storage on which the objects being queried 
    reside. A value of @see KMTPStorageAll requests indicates that an 
    aggregated set of objects residing on all storages is required.
    */
    TUint32 iStorageId;
    
    /**
    The MTP object format code of the objects being queried. A value of 
    @see KMTPFormatsAll indicates that an aggregated set of all object 
    formats is required.
    */
    TUint32 iFormatCode;
    
    /**
    The MTP object handle of the parent of the objects being queried. A 
    value of @see KMTPHandleNoParent indicates that the set of objects 
    residing in the root folder of their storage is required. 
    */
    TUint32 iParentHandle;
	
    /**
    The data provider which owns the MTP objects @see KMTPDataProviderAll 
    indicates objects owned by any data providres
    */
	TUint32  iDpId;
    };
  
/**
Defines the MTP data provider framework object store enumeration manager 
object query context data. This data indicates if there is additional 
object data to be retrieved which matches the query.
*/
class RMTPObjectMgrQueryContext
    {
public:

    IMPORT_C RMTPObjectMgrQueryContext();
    
    IMPORT_C void Close();
    
    IMPORT_C TBool QueryComplete() const;
    
private:

    friend class CMTPObjectStore;
    IMPORT_C RDbView& DbView();
    IMPORT_C void OpenL();
    IMPORT_C TBool IsOpen() const;
    
private:

    TAny* iContext;
    };
    
#endif // MTPOBJECTMGRQUERYTYPES_H
