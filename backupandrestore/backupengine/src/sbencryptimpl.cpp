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
#include "OstTraceDefinitions.h"
#include "sbtrace.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "sbencryptimplTraces.h"
#endif

// If you want this code to actually test encryption then uncomment the next line
//#define __TEST_ENCRYPTION__

namespace conn
	{
	/**
	Symbian OS constructor
	*/
    CSecureBURKeySourceImpl* CSecureBURKeySourceImpl::NewL()
    	{
    	OstTraceFunctionEntry0( CSECUREBURKEYSOURCEIMPL_NEWL_ENTRY );
    	CSecureBURKeySourceImpl* pSelf = new(ELeave) CSecureBURKeySourceImpl();
    	OstTraceFunctionExit0( CSECUREBURKEYSOURCEIMPL_NEWL_EXIT );
    	return pSelf;
    	}
    
    /**
    Standard C++ constructor 
    */
    CSecureBURKeySourceImpl::CSecureBURKeySourceImpl()
    	{
    	OstTraceFunctionEntry0( CSECUREBURKEYSOURCEIMPL_CSECUREBURKEYSOURCEIMPL_CONS_ENTRY );
    	OstTraceFunctionExit0( CSECUREBURKEYSOURCEIMPL_CSECUREBURKEYSOURCEIMPL_CONS_EXIT );
    	}
    
    /**
    Standard C++ destructor
    */
    CSecureBURKeySourceImpl::~CSecureBURKeySourceImpl()
    	{
    	OstTraceFunctionEntry0( CSECUREBURKEYSOURCEIMPL_CSECUREBURKEYSOURCEIMPL_DES_ENTRY );
    	OstTraceFunctionExit0( CSECUREBURKEYSOURCEIMPL_CSECUREBURKEYSOURCEIMPL_DES_EXIT );
    	}

	/**
	See sbencrypt.h
	*/
    void CSecureBURKeySourceImpl::GetDefaultBufferForBackupL(TDriveNumber /*aDrive*/, 
    														 TBool& aGotBuffer, 
    														 TDes& /*aBuffer*/)
    	{
    	OstTraceFunctionEntry0( CSECUREBURKEYSOURCEIMPL_GETDEFAULTBUFFERFORBACKUPL_ENTRY );
    	#ifdef __TEST_ENCRYPTION__
    		_LIT(KTestBuffer, "TEST_BUFFER");
    		
    		aGotBuffer = ETrue;
    		aBuffer = KTestBuffer;
    	#else
    		aGotBuffer = EFalse;
    	#endif
    	OstTraceFunctionExit0( CSECUREBURKEYSOURCEIMPL_GETDEFAULTBUFFERFORBACKUPL_EXIT );
    	}
    	
	/**
	See sbencrypt.h
	*/
    void CSecureBURKeySourceImpl::GetBackupKeyL(TDriveNumber /*aDrive*/, TSecureId /*aSID*/,
                       							TBool& aDoEncrypt, TDes8& /*aKey*/,
                       							TBool& aGotBuffer, TDes& /*aBuffer*/)
    	{
    	OstTraceFunctionEntry0( CSECUREBURKEYSOURCEIMPL_GETBACKUPKEYL_ENTRY );
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
    	OstTraceFunctionExit0( CSECUREBURKEYSOURCEIMPL_GETBACKUPKEYL_EXIT );
    	}
    	
	/**
	See sbencrypt.h
	*/
    void CSecureBURKeySourceImpl::GetRestoreKeyL(TDriveNumber /*aDrive*/, TSecureId /*aSID*/, 
                        						 TBool /*aGotBuffer*/, TDes& /*aBuffer*/,
                        						 TBool &aGotKey, TDes8& /*aKey*/)
    	{
    	OstTraceFunctionEntry0( CSECUREBURKEYSOURCEIMPL_GETRESTOREKEYL_ENTRY );
    	#ifdef __TEST_ENCRYPTION__
    		aGotKey = ETrue;
    		aKey.AppendNum(aSID);
    	#else
    		aGotKey = EFalse;
    	#endif
    	OstTraceFunctionExit0( CSECUREBURKEYSOURCEIMPL_GETRESTOREKEYL_EXIT );
    	}

	}

