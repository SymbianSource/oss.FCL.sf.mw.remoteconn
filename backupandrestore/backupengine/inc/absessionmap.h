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
* Declaration of CABSession class.
* 
*
*/



/**
 @file
*/
#ifndef __ABSESSIONMAP_H__
#define __ABSESSIONMAP_H__

#include <e32std.h>

namespace conn
	{
	class CABSession;
	
	class CABSessionElement : public CBase
	/**
	@internalComponent
	@ingroup Server
	
	The elements hold the TSecureId (key) and own the CABSession (value)
	*/
		{
		public:
			static CABSessionElement* NewL(TSecureId aSecureId);
			TSecureId Key() const;
			CABSession& Value() const;
			~CABSessionElement();
		private:
			CABSessionElement(TSecureId aSecureId);
			void ConstructL();
			
		private:
			TSecureId iKey;
			CABSession* iValue;
		};
		
	inline TSecureId CABSessionElement::Key() const
	/**
	Return the key of this element
	
	@return The key
	*/
		{
		return iKey;
		}

	inline CABSession& CABSessionElement::Value() const
	/**
	Return a reference to the session
	
	@return Reference to the session object that this element owns
	*/
		{
		return *iValue;
		}
	




	class CABSessionMap : public CBase
	/**
    @internalComponent
	@ingroup Server

	An AB client Session map for holding key-value records to map between SID's and their sessions
	*/
		{
		public:
			static CABSessionMap* NewL();
			CABSession& CreateL(TSecureId aSecureId);
			void Delete(TSecureId aSecureId);
			CABSession& SessionL(TSecureId aSecureId);

			~CABSessionMap();
			void InvalidateABSessions();
			
		private:
			CABSessionMap();

		private:
			RPointerArray<CABSessionElement> iMapElements;
		};

	}

#endif //__ABSESSIONMAP_H__
