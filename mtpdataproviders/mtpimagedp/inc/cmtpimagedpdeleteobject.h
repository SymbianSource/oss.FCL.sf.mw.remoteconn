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

#ifndef CMTPIMAGEDPDELETEOBJECT_H
#define CMTPIMAGEDPDELETEOBJECT_H

#include <badesca.h>
#include <f32file.h>

#include "cmtprequestprocessor.h"

class MMTPObjectMgr;
class CMTPImageDataProvider;
class CMTPImageDpObjectPropertyMgr;
/** 
Defines data provider GetObject request processor.
@internalComponent
*/
class CMTPImageDpDeleteObject : public CMTPRequestProcessor
	{
public:

	static MMTPRequestProcessor* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider);
	~CMTPImageDpDeleteObject();    

private: // From CMTPRequestProcessor

	TMTPResponseCode CheckRequestL();
	void ServiceL();
	
    /**
    * AO Run method, deletes a selection of files on the system
    */
	void RunL();
	
    /**
    * Used to cancel the deletion of the files
    */
	void DoCancel();

	TInt RunError( TInt aErr );

private:    

	CMTPImageDpDeleteObject(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection,CMTPImageDataProvider& aDataProvider);
	void ConstructL();
	TMTPResponseCode CheckStorageL(TUint32 aObjectHandle);
	
    /**
    * Gets all object handles (for GetObjectHandlesL)
    */
	void GetObjectHandlesL( TUint32 aStorageId, TUint32 aFormatCode, TUint32 aParentHandle );
	
	/**
	* Deletes an object from the file system and the object store
	*/
	void DeleteObjectL( TUint32 aHandle );
	
	/**
	AO start
	 */
	void Start();
	
	/**
	Send response to PC
	 */
	void SendResponseL();
	
private:

	CMTPImageDataProvider& iDataProvider;
	CMTPObjectMetaData*		iObjectMeta;
	
	/**
	Array of all the objects to delete
	 */
	RArray<TUint> iObjectsToDelete;
	
    /**
    The response code
     */
	TMTPResponseCode iResponseCode;
	
    /**
    The number of Objects to delete
     */
    TInt iObjectsNotDelete;
	};
	
#endif //CMTPIMAGEDPDELETEOBJECT_H

