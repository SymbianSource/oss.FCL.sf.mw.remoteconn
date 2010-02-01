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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/inc/mmtpservicehandler.h

/**
 @file
 @internalTechnology
*/

#ifndef __MMTPSERVICEHANDLER_H__
#define __MMTPSERVICEHANDLER_H__

#include <mtp/cmtptypeserviceproplist.h>
#include <mtp/cmtptypeobjectpropdesc.h>
#include <mtp/tmtptypeguid.h>
#include "mtpsvcdpconst.h"

class CMTPTypeServiceInfo;
class CMTPTypeServiceCapabilityList;
class CMTPTypeDeleteServicePropList;
class CMTPTypeServicePropDescList;
class CMTPTypeObjectPropDesc;
class CRepository;

class MMTPServiceHandler
	{
public:
	virtual TMTPResponseCode GetServiceCapabilityL(TUint16 aServiceFormatCode, CMTPTypeServiceCapabilityList& aServiceCapabilityList) const = 0;
	virtual TMTPResponseCode GetServiceInfoL(CMTPTypeServiceInfo& aServiceInfo) const = 0;
	virtual TMTPResponseCode GetServicePropDescL(TUint16 aServicePropertyCode, CMTPTypeServicePropDescList& aPropDescList) const = 0;
	virtual TMTPResponseCode GetServicePropertyL(TUint16 aPropertyCode, CMTPTypeServicePropList& aPropList) const = 0;
	virtual TMTPResponseCode SetServicePropertyL(const CMTPTypeServicePropListElement& aPropElement) = 0;
	virtual TMTPResponseCode DeleteServicePropertyL(TUint16 aPropertyCode) = 0;
	/**
	Get all service property codes into an array.
	@param aArray Returned service property codes array.
	*/
	virtual void GetAllSevicePropCodesL(RArray<TUint32>& aPropCodes) const = 0;
	virtual void GetAllServiceFormatCodeL(RArray<TUint32>& aFormatCodes) const = 0;

public:
	IMPORT_C static CMTPTypeObjectPropDesc* GenerateSvcObjPropertyL(
		TUint aDataType, const TMTPTypeGuid& aPKNamespace,
		TUint aPKID, const TDesC& aName, TUint8 aFormFlag,
		const MMTPType* aForm, CMTPTypeObjectPropDesc::TMTPObjectPropDescGetSet aObjGetSet,
		TUint16 aObjPropCode, TUint32 aGroupCode = 0);
	IMPORT_C static CMTPTypeObjectPropDesc* GenerateSvcObjPropertyLC(
		TUint aDataType, const TMTPTypeGuid& aPKNamespace,
		TUint aPKID, const TDesC& aName, TUint8 aFormFlag,
		const MMTPType* aForm, CMTPTypeObjectPropDesc::TMTPObjectPropDescGetSet aObjGetSet,
		TUint16 aObjPropCode, TUint32 aGroupCode = 0);

	IMPORT_C static CMTPTypeObjectPropDesc* GenerateGenericObjectPropDescLC(TUint16 aObjPropCode);

	IMPORT_C static TMTPResponseCode SaveServicePropValue(
		CRepository& aRepository, TUint aColumnNum, TInt aNewData);
	IMPORT_C static TMTPResponseCode SaveServicePropValue(
		CRepository& aRepository, TUint aColumnNum, TMTPTypeGuid& aNewData);
	};

#endif // __MMTPSERVICEHANDLER_H__
