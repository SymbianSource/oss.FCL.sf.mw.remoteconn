// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// Implementation of CSecureBUREncryptKeySource class.
// 
//

/**
 @file
 @released
*/
#include "sbencrypt.h"
#include "sbencryptimpl.h"
#include "OstTraceDefinitions.h"
#include "sbtrace.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "sbencryptTraces.h"
#endif

namespace conn
	{
	EXPORT_C CSecureBUREncryptKeySource* CSecureBUREncryptKeySource::NewL()
		{
		OstTraceFunctionEntry0( CSECUREBURENCRYPTKEYSOURCE_NEWL_ENTRY );
		CSecureBUREncryptKeySource* pSelf = new CSecureBUREncryptKeySource();
		CleanupStack::PushL(pSelf);
		pSelf->ConstructL();
		CleanupStack::Pop(pSelf);
		
		OstTraceFunctionExit0( CSECUREBURENCRYPTKEYSOURCE_NEWL_EXIT );
		return pSelf;
		}

	CSecureBUREncryptKeySource::CSecureBUREncryptKeySource()
	/**
	C++ constructor
	*/
		{
		OstTraceFunctionEntry0( CSECUREBURENCRYPTKEYSOURCE_CSECUREBURENCRYPTKEYSOURCE_CONS_ENTRY );
		OstTraceFunctionExit0( CSECUREBURENCRYPTKEYSOURCE_CSECUREBURENCRYPTKEYSOURCE_CONS_EXIT );
		}
		
	EXPORT_C CSecureBUREncryptKeySource::~CSecureBUREncryptKeySource()
		{
		OstTraceFunctionEntry0( CSECUREBURENCRYPTKEYSOURCE_CSECUREBURENCRYPTKEYSOURCE_DES_ENTRY );
		delete iImpl;
		iImpl = NULL;
		OstTraceFunctionExit0( CSECUREBURENCRYPTKEYSOURCE_CSECUREBURENCRYPTKEYSOURCE_DES_EXIT );
		}

	void CSecureBUREncryptKeySource::ConstructL()
	/**
	Symbian 2nd phase construction
	*/
		{
		OstTraceFunctionEntry0( CSECUREBURENCRYPTKEYSOURCE_CONSTRUCTL_ENTRY );
		iImpl = CSecureBURKeySourceImpl::NewL();
		OstTraceFunctionExit0( CSECUREBURENCRYPTKEYSOURCE_CONSTRUCTL_EXIT );
		}

	EXPORT_C void CSecureBUREncryptKeySource::GetDefaultBufferForBackupL(TDriveNumber aDrive, 
																TBool& aGotBuffer, 
																TDes& aBuffer)
		{
		OstTraceFunctionEntry0( CSECUREBURENCRYPTKEYSOURCE_GETDEFAULTBUFFERFORBACKUPL_ENTRY );
		iImpl->GetDefaultBufferForBackupL(aDrive, aGotBuffer, aBuffer);
		OstTraceFunctionExit0( CSECUREBURENCRYPTKEYSOURCE_GETDEFAULTBUFFERFORBACKUPL_EXIT );
		}



	EXPORT_C void CSecureBUREncryptKeySource::GetBackupKeyL(TDriveNumber aDrive, TSecureId aSID,
	                            		  	   TBool &aDoEncrypt, TDes8& aKey,
	                            		  	   TBool& aGotBuffer, TDes& aBuffer)
		{
		OstTraceFunctionEntry0( CSECUREBURENCRYPTKEYSOURCE_GETBACKUPKEYL_ENTRY );
		iImpl->GetBackupKeyL(aDrive, aSID, aDoEncrypt, aKey, aGotBuffer, aBuffer);
		OstTraceFunctionExit0( CSECUREBURENCRYPTKEYSOURCE_GETBACKUPKEYL_EXIT );
		}



	EXPORT_C void CSecureBUREncryptKeySource::GetRestoreKeyL(TDriveNumber aDrive, TSecureId aSID, 
	                             				TBool aGotBuffer, TDes& aBuffer,
	                             				TBool &aGotKey, TDes8& aKey)
		{
		OstTraceFunctionEntry0( CSECUREBURENCRYPTKEYSOURCE_GETRESTOREKEYL_ENTRY );
		iImpl->GetRestoreKeyL(aDrive, aSID, aGotBuffer, aBuffer, aGotKey, aKey);
		OstTraceFunctionExit0( CSECUREBURENCRYPTKEYSOURCE_GETRESTOREKEYL_EXIT );
		}
	}

