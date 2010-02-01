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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/inc/mmtpsvcobjecthandler.h

/**
 @file
 @internalTechnology
*/

#ifndef __MMTPSVCOBJHANDLER_H__
#define __MMTPSVCOBJHANDLER_H__

#include <mtp/mtpprotocolconstants.h>

class CMTPObjectMetaData;
class CMTPTypeArray;
class CMTPTypeObjectInfo;
class CMTPTypeObjectPropList;
class CMTPTypeObjectPropListElement;

class MMTPSvcObjectHandler
	{
public:
	/**
	Dispatch SendObjectInfoL to object handler
	@param aObjectInfo The ObjectInfo dataset specifed in the MTP request
	@param aParentHandle Specify the new object's parent handle, dp can change the handler if necessary.
	@param aSuid Concrete object handler need set the parameter with new created object suid
	@return See MTP response code definition
	@leave One of the system wide error codes, if a general processing error
	*/
	IMPORT_C virtual TMTPResponseCode SendObjectInfoL(const CMTPTypeObjectInfo& aObjectInfo, TUint32& aParentHandle, TDes& aSuid);
	/**
	Dispatch GetObjectInfoL to object handler
	@param aObjectMetaData Specify the getting object's metadata, including suid, objecthandle etc.
	@param aObjectInfo The returned ObjectInfo dataset.
	@return See MTP response code definition
	@leave One of the system wide error codes, if a general processing error
	*/
	IMPORT_C virtual TMTPResponseCode GetObjectInfoL(const CMTPObjectMetaData& aObjectMetaData, CMTPTypeObjectInfo& aObjectInfo);
	/**
	Dispatch SendObjectPropList to object handler
	@param aObjectSize The new object size specified in the MTP request
	@param aObjectPropList Provide info for dp to create object
	@param aParentHandle Specify the new object's parent handle, dp can change the handler if necessary.
	@param aSuid Concrete object handler need set the parameter with new created object suid
	@return See MTP response code definition
	@leave One of the system wide error codes, if a general processing error
	*/
	virtual TMTPResponseCode SendObjectPropListL(TUint64 aObjectSize, const CMTPTypeObjectPropList& aObjectPropList, 
												TUint32& aParentHandle, TDes& aSuid) = 0;
	/**
	Dispatch GetObjectPropListL to object handler, handler only need append the specified object prop code's info.
	@param aObjectMetaData Specify the getting object's metadata, including suid, objecthandle etc.
	@param aPropList The returned ObjectPropList dataset.
	@param aPropertyCode Specify which object property need to be gotten
	@return See MTP response code definition
	@leave One of the system wide error codes, if a general processing error
	*/
	virtual TMTPResponseCode GetObjectPropertyL(const CMTPObjectMetaData& aObjectMetaData, TUint16 aPropertyCode, CMTPTypeObjectPropList& aPropList) = 0;
	/**
	Set a property value of object with concrete object handler
	@param aSuid Specify the object's suid which will be used by object handle to find the concrete object
	@param element include the object's property information, including ObjectHandle, PropertyCode, Datatype and Value
	@param aOperationCode Used to distinguish if the property can be changed by specified operation or not.
	@return See MTP response code definition
	@leave One of the system wide error codes, if a general processing error
	*/
	virtual TMTPResponseCode SetObjectPropertyL(const TDesC& aSuid, const CMTPTypeObjectPropListElement& aElement, TMTPOperationCode aOperationCode) = 0;
	/**
	Dispatch DeleteObjectPropertyL to object handler
	@param aObjectMetaData Specify the deleting object's metadata, including suid, objecthandle etc.
	@param aPropertyCode The property code which need to be deleted.
	@return See MTP response code definition
	@leave One of the system wide error codes, if a general processing error
	*/
	virtual TMTPResponseCode DeleteObjectPropertyL(const CMTPObjectMetaData& aObjectMetaData, TUint16 aPropertyCode) = 0;
	/**
	Dispatch GetBufferForSendObjectL to object handler, the function used to get a buffer which 
	allocated by concrete object handler for the setting object
	@param aObjectMetaData the new object's metadata, including suid, objecthandle etc.
	@param aBuffer The new buffer allocated by object handler
	@return See MTP response code definition
	@leave One of the system wide error codes, if a general processing error
	*/
	virtual TMTPResponseCode GetBufferForSendObjectL(const CMTPObjectMetaData& aObjectMetaData, MMTPType** aBuffer) = 0;
	/**
	Dispatch GetObjectL to object handler. the object handler will allocate a new buffer for the object,
	it will be released by ReleaseObjectBuffer when the request processing is end.
	@param aObjectMetaData Specify the getting object's metadata, including suid, objecthandle etc.
	@param aBuffer The object's content.
	@return See MTP response code definition
	@leave One of the system wide error codes, if a general processing error
	*/
	virtual TMTPResponseCode GetObjectL(const CMTPObjectMetaData& aObjectMetaData, MMTPType** aBuffer) = 0;
	/**
	Delete an object with specified parameters
	@param aObjectMetaData Specify the deleting object's metadata, including suid, objecthandle etc.
	@return See MTP response code definition
	@leave One of the system wide error codes, if a general processing error
	*/
	virtual TMTPResponseCode DeleteObjectL(const CMTPObjectMetaData& aObjectMetaData) = 0;
	/**
	Dispatch SetObjectReferenceL to object handler. the object handler will set reference for specified handle,
	it will be released by ReleaseObjectBuffer when the request processing is end.
	@param aObjectMetaData object need to be set refence.
	@param aReferences reference array.
	@return See MTP response code definition
	@leave One of the system wide error codes, if a general processing error
	*/
	IMPORT_C virtual TMTPResponseCode SetObjectReferenceL(const CMTPObjectMetaData& aObjectMetaData, const CMTPTypeArray& aReferences);
	/**
	Dispatch GetObjectReferenceL to object handler. the object handler will fetch the specified object's reference array,
	it will be released by ReleaseObjectBuffer when the request processing is end.
	@param aObjectMetaData object need to get refence.
	@param aReferences reference array.
	@return See MTP response code definition
	@leave One of the system wide error codes, if a general processing error
	*/
	IMPORT_C virtual TMTPResponseCode GetObjectReferenceL(const CMTPObjectMetaData& aObjectMetaData, CMTPTypeArray& aReferences);
	/**
	Dispatch CopyObject to object handler. This operation causes the device to create a copy of the target object and place 
	that copy in a location that is indicated by the parameters of this operation.
	@param aObjectMetaData Specify the getting object's metadata, including suid, objecthandle etc.
	@param aDestStorageId StorageID that the newly copied object should be placed into.
	@param aNewParentHandle ObjectHandle of newly copied object’s parent.
	@return See MTP response code definition
	@leave One of the system wide error codes, if a general processing error
	*/
	IMPORT_C virtual TMTPResponseCode CopyObject(const CMTPObjectMetaData& aObjectMetaData, TUint32 aDestStorageId, TUint32 aNewParentHandle);
	/**
	Dispatch MoveObject to object handler. This operation changes the location of an object on the device, by changing the storage on 
	which it is stored, changing the location in which it is located, or both.
	@param aObjectMetaData Specify the getting object's metadata, including suid, objecthandle etc.
	@param aDestStorageId StorageID of store to move object to.
	@param aNewParentHandle ObjectHandle of the new ParentObject.
	@return See MTP response code definition
	@leave One of the system wide error codes, if a general processing error
	*/
	IMPORT_C virtual TMTPResponseCode MoveObject(const CMTPObjectMetaData& aObjectMetaData, TUint32 aDestStorageId, TUint32 aNewParentHandle);
	/**
	When dp received UpdateObjectPropList, if the dataset don't include object size value, request 
	processor can use this interface to get object size for the following SendObject.
	@param aSuid Concrete object handler need set the parameter with new created object suid
	@param aObjectSize The updating object's size.
	@return See MTP response code definition
	*/
	virtual TMTPResponseCode GetObjectSizeL(const TDesC& aSuid, TUint64& aObjectSize) = 0;
	/**
	Commit prevous setted property value
	@leave One of the system wide error codes, if a general processing error
	*/
	IMPORT_C virtual void CommitL();
	/**
	Create a new object and commit all cached property value
	@param aSuid Concrete object handler need set the parameter with new created object suid
	@leave One of the system wide error codes, if a general processing error
	*/
	IMPORT_C virtual void CommitForNewObjectL(TDes& aSuid);
	/**
	Rollback setted value for a object, called when SendObject fail or object handler commit fail
	*/
	IMPORT_C virtual void RollBack();
	/**
	After GetObject called, object handler will allocate a buffer and send to initiator. request processor
	need call this interface to release the existing buffer.
	*/
	IMPORT_C virtual void ReleaseObjectBuffer();
	/**
	Get the handler's all supported object prop code with specified group code, if group code is 0, then return
	all prop codes.
	@param aGroupId Group id
	@param aPropCodes Returned object prop code array.
	@return See MTP response code definition
	@leave One of the system wide error codes, if a general processing error
	*/
	virtual TMTPResponseCode GetAllObjectPropCodeByGroupL(TUint32 aGroupId, RArray<TUint32>& aPropCodes) = 0;

protected:
	IMPORT_C TMTPResponseCode CheckGenObjectPropertyL(const CMTPTypeObjectPropListElement& element, TMTPOperationCode aOperationCode) const;
	};

#endif // __MMTPSVCOBJHANDLER_H__

