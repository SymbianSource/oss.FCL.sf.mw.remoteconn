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
* Declaration of CSecureBURKeySourceImpl
* 
*
*/



/**
 @file
*/
#ifndef __SBENCRYPTIMPL_H__
#define __SBENCRYPTIMPL_H__
#include <e32base.h>
#include <f32file.h>

namespace conn
	{
	class CSecureBURKeySourceImpl : public CBase
	/**
	Implementation for the CSecureBURKeySourceImpl
	
	@internalTechnology
	*/
		{
	public:
	    // Construction
	    static CSecureBURKeySourceImpl* NewL();
	    virtual ~CSecureBURKeySourceImpl();

	    // Methods
	    void GetDefaultBufferForBackupL(TDriveNumber aDrive, TBool& aGotBuffer, TDes& aBuffer);
	    void GetBackupKeyL(TDriveNumber aDrive, TSecureId aSID,
	                       TBool& aDoEncrypt, TDes8& aKey,
	                       TBool& aGotBuffer, TDes& aBuffer);
	    void GetRestoreKeyL(TDriveNumber aDrive, TSecureId aSID, 
	                        TBool aGotBuffer, TDes& aBuffer,
	                        TBool &aGotKey, TDes8& aKey);

	private:
	    CSecureBURKeySourceImpl();
		};
	}
#endif __SBENCRYPTIMPL_H__
