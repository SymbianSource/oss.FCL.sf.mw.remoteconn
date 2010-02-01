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
// Templated RArray for serialising and deserialising simple (flat) classes
// for transmission inside descriptors over IPC
// 
//

/**
 @file
 @publishedPartner
 @released
*/

#ifndef __SBEXTERNALISABLEARRAY_H__
#define __SBEXTERNALISABLEARRAY_H__

#include <e32std.h>
#include <e32base.h>

namespace conn
	{
	template<class T>
	class RExternalisableArray : public RArray<T>
	/**
	Template inheriting from RArray, providing functionality to externalise and then internalise 
	an RArray of flat (i.e. without pointers) objects into a descriptor for passing over IPC
	
	@publishedPartner
	@released
	*/
		{
	public:
		inline static RExternalisableArray<T>* InternaliseL(const TDesC8& aExternalisedArray);
		inline HBufC8* ExternaliseL();
		};
	}
	
#include <connect/sbexternalisablearray.inl>
#endif //__SBEXTERNALISABLEARRAY_H__
