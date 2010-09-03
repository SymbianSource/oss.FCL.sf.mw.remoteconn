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
// Implementations of CABSessionMap and CABSessionElement classes.
// 
//

/**
 @file
*/

#include "absession.h"
#include "absessionmap.h"
#include "sbedataowner.h"
#include "sbepanic.h"
#include "OstTraceDefinitions.h"
#include "sbtrace.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "absessionmapTraces.h"
#endif

namespace conn
	{	
	CABSessionElement::CABSessionElement(TSecureId aSecureId) : iKey(aSecureId), iValue(NULL)
    /**
    Class Constructor

	@param aSecureId The secure Id of the data owner that the session has been created for    
	*/
		{
		OstTraceFunctionEntry0( CABSESSIONELEMENT_CABSESSIONELEMENT_CONS_ENTRY );
		OstTraceFunctionExit0( CABSESSIONELEMENT_CABSESSIONELEMENT_CONS_EXIT );
		}

	CABSessionElement::~CABSessionElement()
    /**
    Class destructor
    */
		{
		OstTraceFunctionEntry0( CABSESSIONELEMENT_CABSESSIONELEMENT_DES_ENTRY );
		OstTraceFunctionExit0( CABSESSIONELEMENT_CABSESSIONELEMENT_DES_EXIT );
		}

	CABSessionElement* CABSessionElement::NewL(TSecureId aSecureId)
	/**
	Symbian first phase constructor
	@param aSecureId The secure Id of the data owner that the session has been created for
	*/
		{
		OstTraceFunctionEntry0( CABSESSIONELEMENT_NEWL_ENTRY );
		CABSessionElement* self = new (ELeave) CABSessionElement(aSecureId);
		CleanupStack::PushL(self);
		self->ConstructL();
		CleanupStack::Pop(self);
		OstTraceFunctionExit0( CABSESSIONELEMENT_NEWL_EXIT );
		return self;
		}
		
	void CABSessionElement::ConstructL()
	/**
	Create the session for the data owner specified by iKey
	*/
		{
		OstTraceFunctionEntry0( CABSESSIONELEMENT_CONSTRUCTL_ENTRY );
		// Note that the server takes ownership of the session, not this object
		iValue = CABSession::NewL(iKey);
		OstTraceFunctionExit0( CABSESSIONELEMENT_CONSTRUCTL_EXIT );
		}
					
	CABSessionMap* CABSessionMap::NewL()
	/**
	Symbian first phase constructor
	
	@return Pointer to a created CABSessionMap object
	*/
		{
		OstTraceFunctionEntry0( CABSESSIONMAP_NEWL_ENTRY );
		CABSessionMap* self = new (ELeave) CABSessionMap;
		OstTraceFunctionExit0( CABSESSIONMAP_NEWL_EXIT );
		return self;
		}
		
	CABSession& CABSessionMap::CreateL(TSecureId aSecureId)
	/**
	Create a new element and session, returning that session if required
	
	@param aSecureId The SID to initialise the session with
	@return Reference to the created session
	*/
		{
		OstTraceFunctionEntry0( CABSESSIONMAP_CREATEL_ENTRY );
		CABSessionElement* element = CABSessionElement::NewL(aSecureId);
		CleanupStack::PushL(element);
		iMapElements.AppendL(element);
		CleanupStack::Pop(element);
		
		CABSession& session = element->Value();
		OstTraceFunctionExit0( CABSESSIONMAP_CREATEL_EXIT );
		return session;
		}
		
	void CABSessionMap::Delete(TSecureId aSecureId)
	/**
	Delete the session and remove it from the map
	
	@param aSecureId The key to the session to be deleted
	*/
		{
		OstTraceFunctionEntry0( CABSESSIONMAP_DELETE_ENTRY );
		TInt count = iMapElements.Count();
		
		for (TInt index = 0; index < count; index++)
			{
			if (iMapElements[index]->Key() == aSecureId)
				{
				delete iMapElements[index];
				iMapElements.Remove(index);
				
				break;
				}
			}
		OstTraceFunctionExit0( CABSESSIONMAP_DELETE_EXIT );
		}
		
	CABSession& CABSessionMap::SessionL(TSecureId aSecureId)
	/**
	Accessor for the session using the SID as the key
	
	@param aSecureId The SID of the DO that's connected to the returned session
	@leave KErrNotFound If no session exists for that SID
	@return The session that the DO with SID aSecureId is connected to
	*/
		{
		OstTraceFunctionEntry0( CABSESSIONMAP_SESSIONL_ENTRY );
		TInt count = iMapElements.Count();
		CABSession* pSession = NULL;
		
		for (TInt index = 0; index < count; index++)
			{
			if (iMapElements[index]->Key() == aSecureId)
				{
				pSession = &iMapElements[index]->Value();
				
				break;
				}
			}
			
		if (!pSession)
			{
		    OstTrace0(TRACE_ERROR, CABSESSIONMAP_SESSIONL, "Leave: KErrNotFound");
			User::Leave(KErrNotFound);
			}
		
		OstTraceFunctionExit0( CABSESSIONMAP_SESSIONL_EXIT );
		return *pSession;
		}

	CABSessionMap::CABSessionMap()
    /**
    Class Constructor
    */
		{
		OstTraceFunctionEntry0( CABSESSIONMAP_CABSESSIONMAP_CONS_ENTRY );
		OstTraceFunctionExit0( CABSESSIONMAP_CABSESSIONMAP_CONS_EXIT );
		}

	CABSessionMap::~CABSessionMap()
    /**
    Class destructor
    */
		{
		OstTraceFunctionEntry0( CABSESSIONMAP_CABSESSIONMAP_DES_ENTRY );
		iMapElements.ResetAndDestroy();
		iMapElements.Close();
		OstTraceFunctionExit0( CABSESSIONMAP_CABSESSIONMAP_DES_EXIT );
		}
	
	void CABSessionMap::InvalidateABSessions()
	/** 
	Set each CABSession currently hold in the map as invalid
	*/ 
		{
		OstTraceFunctionEntry0( CABSESSIONMAP_INVALIDATEABSESSIONS_ENTRY );
		TInt count = iMapElements.Count();
		CABSession* pSession = NULL;
					
		for (TInt index = 0; index < count; index++)
			{
			pSession = &iMapElements[index]->Value();
			if (pSession)
				{
				pSession->SetInvalid();
				}
			}
		OstTraceFunctionExit0( CABSESSIONMAP_INVALIDATEABSESSIONS_EXIT );
		}
	}
