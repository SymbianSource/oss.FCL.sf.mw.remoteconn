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

namespace conn
	{
	EXPORT_C CSecureBUREncryptKeySource* CSecureBUREncryptKeySource::NewL()
		{
		CSecureBUREncryptKeySource* pSelf = new CSecureBUREncryptKeySource();
		CleanupStack::PushL(pSelf);
		pSelf->ConstructL();
		CleanupStack::Pop(pSelf);
		
		return pSelf;
		}

	CSecureBUREncryptKeySource::CSecureBUREncryptKeySource()
	/**
	C++ constructor
	*/
		{
		}
		
	EXPORT_C CSecureBUREncryptKeySource::~CSecureBUREncryptKeySource()
		{
		delete iImpl;
		}

	void CSecureBUREncryptKeySource::ConstructL()
	/**
	Symbian 2nd phase construction
	*/
		{
		iImpl = CSecureBURKeySourceImpl::NewL();
		}

	EXPORT_C void CSecureBUREncryptKeySource::GetDefaultBufferForBackupL(TDriveNumber aDrive, 
																TBool& aGotBuffer, 
																TDes& aBuffer)
		{
		iImpl->GetDefaultBufferForBackupL(aDrive, aGotBuffer, aBuffer);
		}



	EXPORT_C void CSecureBUREncryptKeySource::GetBackupKeyL(TDriveNumber aDrive, TSecureId aSID,
	                            		  	   TBool &aDoEncrypt, TDes8& aKey,
	                            		  	   TBool& aGotBuffer, TDes& aBuffer)
		{
		iImpl->GetBackupKeyL(aDrive, aSID, aDoEncrypt, aKey, aGotBuffer, aBuffer);
		}



	EXPORT_C void CSecureBUREncryptKeySource::GetRestoreKeyL(TDriveNumber aDrive, TSecureId aSID, 
	                             				TBool aGotBuffer, TDes& aBuffer,
	                             				TBool &aGotKey, TDes8& aKey)
		{
		iImpl->GetRestoreKeyL(aDrive, aSID, aGotBuffer, aBuffer, aGotKey, aKey);
		}
	}

