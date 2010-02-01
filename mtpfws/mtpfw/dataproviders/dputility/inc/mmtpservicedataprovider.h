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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/inc/mmtpservicedataprovider.h

/**
 @file
 @internalTechnology
*/

#ifndef __MMTPSERVICEDATAPROVIDER_H__
#define __MMTPSERVICEDATAPROVIDER_H__

#include <mtp/mtpprotocolconstants.h>

class MMTPSvcObjectHandler;
class MMTPServiceHandler;
class CMTPTypeStorageInfo;

class MMTPServiceDataProvider
{
public:
	/**
	Get object handler pointer for request processor.
	@param aFormatCode Format code which owned by the returned object handler.
	@return MMTPSvcObjectHandler* object handler's pointer. If the pointer is 
	NULL, then means the input format code is not supported by the data provider.
	*/
	virtual MMTPSvcObjectHandler* ObjectHandler(TUint16 aFormatCode) const = 0;
	/**
	Get service handler pointer for request processor.
	@return MMTPServiceHandler* service handler's pointer. If the pointer is 
	NULL, then means data provider doesn't support service extension.
	*/
	IMPORT_C virtual MMTPServiceHandler* ServiceHandler() const;
	/**
	Check if a service property code is valid.
	@param aPropCode A service property code need to be checked.
	@return TBool ETrue is valid.
	*/
	IMPORT_C virtual TBool IsValidServicePropCodeL(TUint16 aPropCode) const;
	/**
	Check if a format code is valid.
	@param aFormatCode A format code need to be checked data provider.
	@return TBool ETrue is valid.
	*/
	virtual TBool IsValidFormatCodeL(TUint16 aFormatCode) const = 0;
	/**
	Check if a object property code is valid.
	@param aFormatCode A format code which the property code belong to.
	@param aPropCode A object property code need to be checked.
	@return TBool ETrue is valid.
	*/
	virtual TBool IsValidObjectPropCodeL(TUint16 aFormatCode, TUint16 aPropCode) const = 0;
	/**
	Get service id owned by a data provider.
	@return TUint32 Service id.
	*/
	IMPORT_C virtual TUint32 ServiceID() const;
	/**
	Get storage id owned by a data provider.
	@return TUint32 Storage id.
	*/
	virtual TUint32 StorageId() const = 0;
	/**
	Get the AO's granularity for delete all objects and get object proper list. 
	@return TUint32 Operation granularity, normally the value is 50.
	*/
	IMPORT_C virtual TUint32 OperationGranularity() const;
	/**
	Dispatch GetStorageInfo to data provider
	@param aStorageInfo StorageInfo dataset.
	@return See MTP response code definition
	@leave One of the system wide error codes, if a general processing error
	*/
	virtual TMTPResponseCode GetStorageInfoL(CMTPTypeStorageInfo& aStorageInfo) const = 0;
};

#endif // __MMTPSERVICEDATAPROVIDER_H__
