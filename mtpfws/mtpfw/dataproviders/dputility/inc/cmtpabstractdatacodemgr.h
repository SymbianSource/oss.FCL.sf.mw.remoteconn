// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/inc/cmtpabstractdatacodemgr.h
// Abstract service provider's data code manager

/**
 @file
 @internalComponent
 */

#ifndef __CMTPABSTRACTDATACODEMGR_H__
#define __CMTPABSTRACTDATACODEMGR_H__

#include <mtp/mtpdataproviderapitypes.h>
#include <mtp/tmtptypeguid.h>

#include "mtpdebug.h"
#include "mtpsvcdpconst.h"

class MMTPDataProviderFramework;

/**
Keep service prop info, used by service related operation
@internalComponent
*/
class TMTPServicePropertyInfo
	{
public:
	TUint16 iIndex;             // The enum index value of Service Property definition
	TUint16 iServicePropCode;   // The Property Code, allocated by framework
	TMTPTypeGuid iServicePropPKeyNamespace;  // The GUID of the NameSpace this property belongs to. Specified by Spec.
	TUint32 iServicePropPKeyID; // The PKeyID, the index in the NameSpace. Specified by Spec.
	TPtrC iServicePropertyName; // The Property Name

public:
	static IMPORT_C TInt LinearOrderServicePropOrder(const TMTPServicePropertyInfo& aLhs, const TMTPServicePropertyInfo& aRhs);
	static IMPORT_C TInt LinearOrderServicePropOrder(const TUint16* aServicePropCode, const TMTPServicePropertyInfo& aObject);
	};

/**
Keep format related info and all object properties belong to the format
@internalComponent
*/
class RMTPServiceFormat
	{
public:
	IMPORT_C ~RMTPServiceFormat();

	TUint16 iIndex;                 // The enum index value of Service Format definition
	TUint16 iFormatCode;            // The Format Code, allocated by framework
	TMTPTypeGuid iFormatGUID;    // The Format GUID, specified by Spec.
	TUint16 iBaseFormatCode;        // The Base Format Code, reserved for further usage according to Spec.
	TPtrC iFormatName;              // The Format Name
	TPtrC iMIMEType;                // The Format MIME Type

	// Property Code is TUint16 actually, but RArray<T> need 4 byte alignment.
	// So use RArray<TUint32> to store property code
	RArray<TUint32> iProps;         // Store all object properties code to specified format
public:
	//static auxiliary function
	static IMPORT_C TBool FormatRelation(const TUint16* aFormatCode, const RMTPServiceFormat& aObject);
	};

/**
Implements the service related data code(format, service prop and object prop) management.
@internalComponent
*/
class CMTPFullEnumDataCodeMgr: public CBase
	{
public:
	IMPORT_C static CMTPFullEnumDataCodeMgr* NewL(MMTPDataProviderFramework& aFramework);
	IMPORT_C virtual ~CMTPFullEnumDataCodeMgr();

	// Common Service DataCodeMgr APIs
	IMPORT_C TUint ServiceID() const;
	IMPORT_C const TMTPTypeGuid& ServiceGUID() const;

	IMPORT_C void GetSevicePropCodesL(RArray<TUint32>& aArray) const;
	IMPORT_C const TMTPServicePropertyInfo* ServicePropertyInfo(TUint16 aServicePropCode) const;

	// FullEnum specific APIs
	IMPORT_C const RMTPServiceFormat& KnowledgeFormat() const;

private:
	CMTPFullEnumDataCodeMgr(MMTPDataProviderFramework& aFramework);
	void ConstructL();

	void BuildServiceIDL();
	void BuildFormatL();
	void BuildServicePropertyL();

private:
	MMTPDataProviderFramework& iFramework;
	RArray<TMTPServicePropertyInfo> iServiceProperties;

	// only Knowledge Format supported.
	RMTPServiceFormat* iKnowledgeFormat;

	TUint iServiceID;
	TMTPTypeGuid iServiceGUID;
	TMTPTypeGuid iPersistentServiceGUID;

	/**
	FLOGGER debug trace member variable.
	*/
	__FLOG_DECLARATION_MEMBER_MUTABLE;
	};
#endif // __CMTPABSTRACTDATACODEMGR_H__
