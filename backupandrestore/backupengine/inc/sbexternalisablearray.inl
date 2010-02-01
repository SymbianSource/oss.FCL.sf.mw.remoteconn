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
// externalisablearray.inl
// Templated RArray for serialising and deserialising simple (flat) classes
// for transmission inside descriptors over IPC
// 
//

/**
 @file
 @publishedPartner
 @released
*/

namespace conn
	{
	template<class T> inline RExternalisableArray<T>* RExternalisableArray<T>::InternaliseL(const TDesC8& aExternalisedArray)
	/**
	Internalises an RArray of objects passed via IPC from a descriptor
	
	@param aExternalisedArray The descriptor passed via IPC that contains the flattened array to internalise
	@return Pointer to a newly created RExternalisableArray
	*/
		{
		RExternalisableArray<T>* self = new (ELeave) RExternalisableArray<T>;
		
		CleanupStack::PushL(self);
		
		TUint8* unpackPtr = const_cast<TUint8*>(aExternalisedArray.Ptr());
		TInt elementCount = *reinterpret_cast<TInt*>(unpackPtr);
		
		// point to the start of the packed elements
		unpackPtr += sizeof(TInt);
		
		for (TInt index = 0; index < elementCount; index++)
			{
			User::LeaveIfError(self->Append(*reinterpret_cast<T*>(unpackPtr)));
			unpackPtr += sizeof(T);
			}
			
		CleanupStack::Pop(self);
			
		return self;
		}
	
	template<class T> inline HBufC8* RExternalisableArray<T>::ExternaliseL()
	/**
	Externalises an RArray of objects (not pointers) into a descriptor so that the RArray
	may be passed via IPC
	
	@return Pointer to the created HBufC8
	*/
		{
		// calculate the flattened size of the array
		TInt elementSize = sizeof(T);
		TInt count = RArray<T>::Count();
		TInt totalPackedSize = sizeof(TInt) + (sizeof(T) * count);	// TInt stores the number of elements to be packed
	
		// create the HBufC8 to be returned
		HBufC8* externalisedArray = HBufC8::NewL(totalPackedSize);
	
		TPtr8 buffer = externalisedArray->Des();
	
		// Append the number of entries to the beginning of the array
		buffer.Append(reinterpret_cast<TUint8*>(&count), sizeof(TInt));
	
		// bitwise copy each entry into the buffer
		for (TInt index = 0; index < count; index++)
			{
			buffer.Append(reinterpret_cast<TUint8*>(&(*this)[index]), elementSize);
			}
		
		return externalisedArray;
		}
	}
