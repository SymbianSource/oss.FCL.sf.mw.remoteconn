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
// Implementation of CSecureBURKeySourceImpl
// 
//

/**
 @file
*/
#include "sbencryptimpl.h"

// If you want this code to actually test encryption then uncomment the next line
//#define __TEST_ENCRYPTION__

namespace conn
	{
	/**
	Symbian OS constructor
	*/
    CSecureBURKeySourceImpl* CSecureBURKeySourceImpl::NewL()
    	{
    	CSecureBURKeySourceImpl* pSelf = new(ELeave) CSecureBURKeySourceImpl();
    	return pSelf;
    	}
    
    /**
    Standard C++ constructor 
    */
    CSecureBURKeySourceImpl::CSecureBURKeySourceImpl()
    	{
    	}
    
    /**
    Standard C++ destructor
    */
    CSecureBURKeySourceImpl::~CSecureBURKeySourceImpl()
    	{
    	}

	/**
	See sbencrypt.h
	*/
    void CSecureBURKeySourceImpl::GetDefaultBufferForBackupL(TDriveNumber /*aDrive*/, 
    														 TBool& aGotBuffer, 
    														 TDes& /*aBuffer*/)
    	{
    	#ifdef __TEST_ENCRYPTION__
    		_LIT(KTestBuffer, "TEST_BUFFER");
    		
    		aGotBuffer = ETrue;
    		aBuffer = KTestBuffer;
    	#else
    		aGotBuffer = EFalse;
    	#endif
    	}
    	
	/**
	See sbencrypt.h
	*/
    void CSecureBURKeySourceImpl::GetBackupKeyL(TDriveNumber /*aDrive*/, TSecureId /*aSID*/,
                       							TBool& aDoEncrypt, TDes8& /*aKey*/,
                       							TBool& aGotBuffer, TDes& /*aBuffer*/)
    	{
    	#ifdef __TEST_ENCRYPTION__
    		_LIT(KTestBuffer, "TEST_BUFFER");
    		
    		aDoEncrypt = ETrue;
    		aKey.AppendNum(aSID);
    		aGotBuffer = ETrue;
    		aBuffer = KTestBuffer;
    	#else
    		aDoEncrypt = EFalse;
    		aGotBuffer = EFalse;
    	#endif
    	}
    	
	/**
	See sbencrypt.h
	*/
    void CSecureBURKeySourceImpl::GetRestoreKeyL(TDriveNumber /*aDrive*/, TSecureId /*aSID*/, 
                        						 TBool /*aGotBuffer*/, TDes& /*aBuffer*/,
                        						 TBool &aGotKey, TDes8& /*aKey*/)
    	{
    	#ifdef __TEST_ENCRYPTION__
    		aGotKey = ETrue;
    		aKey.AppendNum(aSID);
    	#else
    		aGotKey = EFalse;
    	#endif
    	}

	}

