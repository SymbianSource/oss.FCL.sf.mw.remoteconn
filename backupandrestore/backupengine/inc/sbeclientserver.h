/**
* Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:
* Contains defines shared between the Secure Backup Engine client and Server.
* 
*
*/



/**
 @file
*/

#ifndef __SBECLIENTSERVER_H__
#define __SBECLIENTSERVER_H__

#include <connect/tserverstart.h>

namespace conn
	{
	/** The name of the SBE server execuatable.
	@ingroup Client
	@internalComponent
	*/
	_LIT(KSBEServerName,"!SBEServer");

	/** Number of times to attempt connection to server.
	@ingroup Client
	@internalComponent
	*/
	const TInt KSBERetryCount = 4;

	/** Number of async message slots.
	@ingroup Client
	@internalComponent
	*/
	const TInt KSBEASyncMessageSlots = 3;


	/** The SBE major version number.
	@ingroup Client
	@internalComponent
	*/
	const TUint KSBEMajorVersionNumber = 1;

    /** The SBE minor version number.
	@ingroup Client
	@internalComponent
	*/
	const TUint KSBEMinorVersionNumber = 0;

    /** The SBE build number
	@ingroup Client
	@internalComponent
	*/
	const TUint KSBEBuildVersionNumber = 0;

	enum TSBEMessages
	/** SBE Client/Server Messages
	@internalComponent
	*/
		{
		ESBEMsgGetGSHHandle,			/*!< Pass the the Global Anonymous Shared Heap handle to client */
		ESBEMsgPrepDataOwnerInfo,		/*!< Prepare the data owner information for a specific SID */
		ESBEMsgGetDataOwnerInfo,		/*!< Transfer the data owner information for a specific SID */
		ESBEMsgPrepPublicFiles,			/*!< Prepare the data owner information for a specific SID */
		ESBEMsgGetPublicFiles,			/*!< Transfer the data owner information for a specific SID */
		ESBEMsgPrepPublicFilesRaw,		/*!< Prepare the data owner information for a specific SID */
		ESBEMsgGetPublicFilesRaw,		/*!< Transfer the data owner information for a specific SID */
		ESBEMsgPrepPublicFilesXML,		/*!< Prepare the data owner information for a specific SID */
		ESBEMsgGetPublicFilesXML,		/*!< Transfer the data owner information for a specific SID */
		ESBEMsgSetBURMode,				/*!< Set the Backup and Restore mode and associatedial BUR */
		ESBEMsgPrepSIDStatus,			/*!< Get the returned descriptor length from a SIDS params */
		ESBEMsgSetSIDListPartial,		/*!< Set the list of data owners affected in a parttatus array */
		ESBEMsgGetSIDStatus,			/*!< Transfer the array of SID's and statuses */
		ESBEMsgRequestDataSync,			/*!< Request that the SBE transfers some data to the client */
		ESBEMsgSupplyDataSync,			/*!< Instruct the SBE that data has been supplied ine client */
		ESBEMsgRequestDataAsync,		/*!< Request that the SBE transfers some data to th the GSH */
		ESBEMsgSupplyDataAsync,			/*!< Instruct the SBE that data has been supplied in the GSH */
		ESBEMsgAllSnapshotsSupplied,	/*!< Request that supplied SID's are subject to a Base backup */
		ESBEMsgGetExpectedDataSize,		/*!< Ask the SBE for the expected size of backup data */
		ESBEMsgAllSystemFilesRestored,	/*!< Instruct the SBE that all system files have now been restored */
		ESBEMsgGetDataChecksum,			/*!< For testing - Request the checksum of a SID's backup data */
		ESBEMsgPrepLargePublicFiles,	/*!< Instruct the SBE to build the public file list and place in GSH */
		ESBEMsgGetLargePublicFiles		/*!< Retrieve parameters such as the finished flag from SBE */
		};

	}

#endif //__SBECLIENTSERVER_H__
