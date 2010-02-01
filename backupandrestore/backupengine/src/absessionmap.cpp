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
#include <connect/panic.h>

namespace conn
	{
	
	CABSessionElement::CABSessionElement(TSecureId aSecureId) : iKey(aSecureId), iValue(NULL)
    /**
    Class Constructor

	@param aSecureId The secure Id of the data owner that the session has been created for    
	*/
		{
		}

	CABSessionElement::~CABSessionElement()
    /**
    Class destructor
    */
		{
		}

	CABSessionElement* CABSessionElement::NewL(TSecureId aSecureId)
	/**
	Symbian first phase constructor
	@param aSecureId The secure Id of the data owner that the session has been created for
	*/
		{
		CABSessionElement* self = new (ELeave) CABSessionElement(aSecureId);
		CleanupStack::PushL(self);
		self->ConstructL();
		CleanupStack::Pop(self);
		return self;
		}
		
	void CABSessionElement::ConstructL()
	/**
	Create the session for the data owner specified by iKey
	*/
		{
		// Note that the server takes ownership of the session, not this object
		iValue = CABSession::NewL(iKey);
		}
					
	CABSessionMap* CABSessionMap::NewL()
	/**
	Symbian first phase constructor
	
	@return Pointer to a created CABSessionMap object
	*/
		{
		CABSessionMap* self = new (ELeave) CABSessionMap;
		return self;
		}
		
	CABSession& CABSessionMap::CreateL(TSecureId aSecureId)
	/**
	Create a new element and session, returning that session if required
	
	@param aSecureId The SID to initialise the session with
	@return Reference to the created session
	*/
		{
		CABSessionElement* element = CABSessionElement::NewL(aSecureId);
		CleanupStack::PushL(element);
		iMapElements.AppendL(element);
		CleanupStack::Pop(element);
		return element->Value();
		}
		
	void CABSessionMap::Delete(TSecureId aSecureId)
	/**
	Delete the session and remove it from the map
	
	@param aSecureId The key to the session to be deleted
	*/
		{
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
		}
		
	CABSession& CABSessionMap::SessionL(TSecureId aSecureId)
	/**
	Accessor for the session using the SID as the key
	
	@param aSecureId The SID of the DO that's connected to the returned session
	@leave KErrNotFound If no session exists for that SID
	@return The session that the DO with SID aSecureId is connected to
	*/
		{
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
			User::Leave(KErrNotFound);
			}
		
		return *pSession;
		}

	CABSessionMap::CABSessionMap()
    /**
    Class Constructor
    */
		{
		}

	CABSessionMap::~CABSessionMap()
    /**
    Class destructor
    */
		{
		iMapElements.ResetAndDestroy();
		iMapElements.Close();
		}
	
	void CABSessionMap::InvalidateABSessions()
	/** 
	Set each CABSession currently hold in the map as invalid
	*/ 
		{
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
		}
	}
