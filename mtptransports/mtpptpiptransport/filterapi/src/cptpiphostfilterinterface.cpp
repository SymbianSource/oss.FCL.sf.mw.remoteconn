
// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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
//

#include "cptpiphostfilterinterface.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cptpiphostfilterinterfaceTraces.h"
#endif


/*
Creates an implementation of an ECOM plugin with the specified UID
*/
EXPORT_C CPTPIPHostFilterInterface* CPTPIPHostFilterInterface::NewL()
	{		
	OstTraceFunctionEntry0( CPTPIPHOSTFILTERINTERFACE_NEWL_ENTRY );
	const TUid KFilterImplUid ={0xA0004A5F};
	
	TAny* defaultFilter=NULL;
	
	TRAPD(error,defaultFilter=REComSession::CreateImplementationL(KFilterImplUid,_FOFF(CPTPIPHostFilterInterface,iID_offset)));
	if(error==KErrNone)
	{
	return (reinterpret_cast<CPTPIPHostFilterInterface*>(defaultFilter));
	}
	else 
		{
		OstTraceFunctionExit0( CPTPIPHOSTFILTERINTERFACE_NEWL_EXIT );
		return NULL;	
		}
	}


/*
Lists all the implementations for that Interface identified by the Interface ID
*/
EXPORT_C void CPTPIPHostFilterInterface::ListImplementations(RImplInfoPtrArray& aImplInfoArray)
	{
	OstTraceFunctionEntry0( CPTPIPHOSTFILTERINTERFACE_LISTIMPLEMENTATIONS_ENTRY );
	const TUid KFilterInterfaceUid ={0xA0004A5E};
	TRAPD(ret, REComSession::ListImplementationsL(KFilterInterfaceUid,aImplInfoArray));
	if(ret != KErrNone)
	{
		RDebug::Print(_L("CPTPIPController::ListImplementations ERROR = %d\n") ,ret);
	}

	OstTraceFunctionExit0( CPTPIPHOSTFILTERINTERFACE_LISTIMPLEMENTATIONS_EXIT );
	}


/*
Destructor
*/
EXPORT_C CPTPIPHostFilterInterface::~CPTPIPHostFilterInterface()
{
OstTraceFunctionEntry0( CPTPIPHOSTFILTERINTERFACE_CPTPIPHOSTFILTERINTERFACE_ENTRY );
OstTraceFunctionExit0( CPTPIPHOSTFILTERINTERFACE_CPTPIPHOSTFILTERINTERFACE_EXIT );
}




