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
// mw/remoteconn/mtpfws/mtpfw/dataproviders/dputility/inc/mtpsvcdpconst.h

/**
 @file
 @internalComponent
 */

#ifndef __MTPSVCDPCONST_H__
#define __MTPSVCDPCONST_H__

#include <mtp/mtpprotocolconstants.h>


/**
service types defined in MTP
 */
typedef enum
	{
	EMTPServiceTypeNormal = 0,
	EMTPServiceTypeAbstract = 1
	} TMTPServiceType;

/**
Abstract service types defined in MTP
 */
typedef enum
	{
	EMTPFullEnumSyncService = 0,
	EMTPAnchorSyncService
	} TMTPAbstrctServiceType;

// Service Name definition
_LIT(KFullEnumServiceName,  "FullEnumSync");

// FullEnum Sync Service GUID, specified by Spec.
const TUint32 KMTPFullEnumServiceGUID[] = {0x28d3aac9, 0xc07544be, 0x888165f3, 0x8d305909};

// FullEnumm Sync Service Persistent GUID, specified by Symbian/Nokia Implementation.
const TUint32 KMTPFullEnumServicePSGUID[] = {0x102827A1, 0x00000001, 0x00000000, 0x00000000};

/*
 Service property definition
 */

//FullEnum Sync Service Namespace for Service Property, specified by Spec.
const TUint32 KMTPFullEnumSyncServiceNSGUID[] = {0x63b10e6c, 0x4f3a456d, 0x95cb9894, 0xedec9fa5};
//SyncSvc Service Namespace for Service Property, specified by Spec.
const TUint32 KMTPSyncSvcServiceNSGUID[] = {0x703d392c, 0x532c4607, 0x91589cea, 0x742f3a16};

// FullEnum/Anchor Sync service property value definition
typedef enum
	{
	EMTPServicePropertyVersionProps = 0,
	EMTPServicePropertyReplicaID,
	EMTPServicePropertyKnowledgeObjectID,
	EMTPServicePropertyLastSyncProxyID,
	EMTPServicePropertyProviderVersion,         // In Simulator, it's not supported.
	EMTPServicePropertySyncFormat,              // In Simulator, it's moved to normal service property
	EMTPServicePropertyLocalOnlyDelete,
	EMTPServicePropertyFilterType,
	EMTPServicePropertySyncObjectReferences,    // In Simulator, it's not supported.
	EMTPAbstractServicePropertyEnd
	} TMTPAbstractServiceProperty;

// Define Full Enumeration Sync service property name
_LIT(KNameFullEnumVersionProps,         "FullEnumVersionProps");
_LIT(KNameFullEnumReplicaID,            "FullEnumReplicaID");
_LIT(KNameFullEnumKnowledgeObjectID,    "FullEnumKnowledgeObjectID");
_LIT(KNameFullEnumLastSyncProxyID,      "FullEnumLastSyncProxyID");
_LIT(KNameFullEnumProviderVersion,      "FullEnumProviderVersion");
_LIT(KNameSyncSvcLocalOnlyDelete,       "LocalOnlyDelete");
_LIT(KNameSyncSvcSyncFormat,            "SyncFormat");
_LIT(KNameSyncSvcFilterType,            "FilterType");
_LIT(KNameSyncSvcSyncObjectReferences,  "SyncObjectReferences");

/**
Abstract service format definition
 */
// Define all the abstract service format
typedef enum
	{
	EMTPFormatTypeFullEnumSyncKnowledge,
	EMTPAbstractFormatTypeEnd
	} TMTPAbstractServiceFormatType;

_LIT(KNameFullEnumSyncKnowledege, "FullEnumSyncKnowledge");
_LIT(KNameFullEnumSyncKnowledegeMIMEType, "");


/**
Format property definition
---------Supported Property Namespaces--------------
    FullEnum Knowledge Format   : { Generic Object Namespace Properties; }
 */

// FullEnum Sync Service > Knowledge Format GUID, specified by Spec.
const TUint32 KMTPFullEnumSyncKnowledgeFormatGUID[] = {0x221bce32, 0x221b4f45, 0xb48b80de, 0x9a93a44a};

// Generic Object Namespace GUID for Service Format, specified by Spec.
const TUint32 KMTPGenericObjectNSGUID[] = {0xEF6B490D, 0x5CD8437A, 0xAFFCDA8B, 0x60EE4A3C};

//Sync Object Namespace for Service Format, specified by Spec. Current this value comes from Simulator.
const TUint32 KMTPSyncObjcetNSGUID[] = {0x37364f58, 0x2f744981, 0x99a57ae2, 0x8aeee319};

// Generic Object Namespace prop code value defnition
typedef enum
	{
	EMTPGenObjPropCodeParentID             = EMTPObjectPropCodeParentObject,
	EMTPGenObjPropCodeName                 = EMTPObjectPropCodeName,
	EMTPGenObjPropCodePersistentUniqueObjectIdentifier = EMTPObjectPropCodePersistentUniqueObjectIdentifier,
	EMTPGenObjPropCodeObjectFormat         = EMTPObjectPropCodeObjectFormat,
	EMTPGenObjPropCodeObjectSize           = EMTPObjectPropCodeObjectSize,
	EMTPGenObjPropCodeStorageID            = EMTPObjectPropCodeStorageID,
	EMTPGenObjPropCodeObjectHidden         = EMTPObjectPropCodeHidden,
	EMTPGenObjPropCodeNonConsumable        = EMTPObjectPropCodeNonConsumable,
	EMTPGenObjPropCodeDateModified         = EMTPObjectPropCodeDateModified,
	// LastAuthorProxyID is from Simulator, and is not a Generic Object property. For conveniency, put it here.
	EMTPSvcObjPropCodeLastAuthorProxyID    = EMTPObjectPropCodeVendorExtensionStart + 1,
	} TMTPGenericObjectProperty;

// Normal service Object Extension definition should start with this value
const TUint16 KMTPNormalServiceObjExtensionStart = EMTPObjectPropCodeVendorExtensionStart + 2;

// Define Generic Object Property name string
_LIT(KObjPropNameParentID,          "ParentID");
_LIT(KObjPropNameName,              "Name");
_LIT(KObjPropNamePUOID,             "PersistentUID");
_LIT(KObjPropNameObjectFormat,      "ObjectFormat");
_LIT(KObjPropNameObjectSize,        "ObjectSize");
_LIT(KObjPropNameStorageID,         "StorageID");
_LIT(KObjPropNameHidden,            "Hidden");
_LIT(KObjPropNameNonConsumable,     "NonConsumable");
_LIT(KObjPropNameDateModified,      "DateModified");
_LIT(KObjPropNameLastAuthorProxyID, "LastAuthorProxyID");


// FullEnum Knowledge Format object prop array
const TUint16 KMTPFullEnumSyncKnowledgeObjectProperties[] =
	{
	EMTPGenObjPropCodeParentID,
	EMTPGenObjPropCodeName,
	EMTPGenObjPropCodePersistentUniqueObjectIdentifier,
	EMTPGenObjPropCodeObjectFormat,
	EMTPGenObjPropCodeObjectSize,
	EMTPGenObjPropCodeStorageID,
	EMTPGenObjPropCodeObjectHidden,
	EMTPGenObjPropCodeNonConsumable,
	EMTPGenObjPropCodeDateModified,
	EMTPSvcObjPropCodeLastAuthorProxyID // Simulator defined
	};


/*
Value Range definition
 */
// Enum value describing type of hidden.
typedef enum 
	{
	EMTPUnInitialized                 = 0xFF,
	EMTPGenObjNotHidden               = 0,
	EMTPGenObjHidden                  = 1
	} TMTPGenObjHidden;

// Enum value describing type of the filter
typedef enum
	{
	EMTPSyncSvcFilterNone                         = 0,
	EMTPSyncSvcFilterContactsWithPhone            = 1,
	EMTPSyncSvcFilterTaskActive                   = 2,
	EMTPSyncSvcFilterCalendarWindowWithRecurrence = 3,
	EMTPSyncSvcFilterUnInitialized                = ~0
	} TMTPSyncSvcFilterType;

// Boolean flag indicating whether deletion of objects on the service host
// should be treated as "local only" and not propogated to other sync
// participants.  The alternative is "true sync" in which deletes on the
// service host are propogated to all other sync participants.
typedef enum
	{
	EMTPLocalOnlyDeleteFalse          = 0,
	EMTPLocalOnlyDeleteTrue           = 1,
	EMTPLocalOnlyDeleteUnInitialized  = ~0
	} TMTPSyncSvcLocalOnlyDelete;

// Boolean flag describing whether object references should be included
// as part of the sync process or not.
typedef enum
	{
	EMTPSyncObjectReferencesDisabled = 0x00,
	EMTPSyncObjectReferencesEnabled  = 0xFF
	} TMTPSyncSvcSyncObjectReferences;


/*
Simple const value definition
 */
// Define abstract service info const variable
_LIT(KMTPExtension, "microsoft.com/DeviceServices: 1.0");

// the default value of BaseFormat in service info dataset
const TUint16 KBaseFormatCode = 0x00;

// the default value of ServiceVersion in service info dataset
const TUint32 KMTPDefaultServiceVersion = 0x64;

// the default value of ProviderVersion in service property, 0 is reserved, so start from 1.
const TUint32 KMTPDefaultProviderVersion = 0x01;

// the default value of ServiceType in service info dataset
// 0 if standard service. 1 if abstract
const TUint32 KMTPDefaultServiceType   = 0x00;

// the default value of BaseServiceID in service info dataset
const TUint32 KMTPDefaultBaseServiceID = 0x00;

// the value in spec. to sepcify to do some operation on all service properties.
const TUint32 KMTPServicePropertyAll = 0x00000000;

const TUint32 KMTPUnInitialized32 = 0xFFFFFFFF;
const TUint64 KMTPUnInitialized64 = ~0; //MAKE_TINT64(KMTPUnInitialized32, KMTPUnInitialized32);
const TUint32 KMaxSUIDLength = 50;
const TUint64 KObjectSizeNotAvaiable = ~0;
const TUint32 KLongStringMaxLength = 0x1000;

//---------------------------------------------------------------
// the old names, It'd better be renamed.

// Knowledge object prop array Group 2
const TUint16 KMTPKnowledgeObjectPropertiesGroup2[] =
	{
	EMTPGenObjPropCodeParentID,
	EMTPGenObjPropCodePersistentUniqueObjectIdentifier,
	EMTPGenObjPropCodeObjectFormat,
	EMTPGenObjPropCodeObjectSize,
	EMTPGenObjPropCodeStorageID,
	EMTPGenObjPropCodeObjectHidden,
	EMTPGenObjPropCodeDateModified,
	EMTPSvcObjPropCodeLastAuthorProxyID
	};
// Knowledge object prop array Group 5
const TUint16 KMTPKnowledgeObjectPropertiesGroup5[] =
	{
	EMTPGenObjPropCodeName
	};

// Knowledge object prop array Group 1
const TUint16 KMTPKnowledgeObjectPropertiesGroup1[] =
	{
	EMTPGenObjPropCodeNonConsumable
	};

//---------------------------------------------------------------

#endif // __MTPSVCDPCONST_H__

