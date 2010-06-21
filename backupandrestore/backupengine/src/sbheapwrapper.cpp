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
// CHeapWrapper implementation
// 
//

/**
 @file
 @internalTechnology
*/

#include "sbheapwrapper.h"
#include "sbepanic.h"

namespace conn
	{
	CHeapWrapper* CHeapWrapper::NewL()
	/**
	Symbian 1st phase constructor
	
	@return pointer to a newly created CHeapWrapper object
	*/
		{
		CHeapWrapper* self = new (ELeave) CHeapWrapper();
		
		return self;
		}
	
	TPtrC8& CHeapWrapper::ReadBufferL(const RChunk& aChunk)
	/**
	Getter for the GSH descriptor
	
	@param aChunk The RChunk to interrogate for the heap information
	@return reference to the descriptor stored on the global shared heap
	*/
		{
		if (Header(aChunk).LockedFlag())
			{
			User::Leave(KErrInUse);
			}

		CleanReadBuffer();
			
		iReadBuf = new (ELeave) TPtrC8(Buffer(aChunk));

		return *iReadBuf;
		}
		
	TPtr8& CHeapWrapper::WriteBufferL(const RChunk& aChunk)
	/**
	Lock the heap and return a reference to the descriptor on the GSH
	
	@param aChunk The RChunk to interrogate for the heap information
	@return reference to the descriptor stored on the global shared heap
	*/
		{
		if (Header(aChunk).LockedFlag())
			{
			User::Leave(KErrInUse);
			}

		CleanReadBuffer();

		TPtr8& des = Buffer(aChunk);
		
		// Blank the descriptor as it's a write buffer
		des.Zero();
		
		return des;
		}
		
	THeapWrapperHeader& CHeapWrapper::Header(const RChunk& aChunk) const
	/**
	Getter for the protocol structure
	
	@param aChunk The RChunk to interrogate for the heap information
	@return Pointer to the header structure containing heap protocol information
	*/
		{
		return *reinterpret_cast<THeapWrapperHeader*>(aChunk.Base() + KHeaderOffset);
		}
		
	TInt CHeapWrapper::ResetHeap(const RChunk& aChunk)
	/**
	Reset all of the protocol data in the stack if we're the first users
	
	@param aChunk The RChunk to interrogate for the heap information
	@return KErrNone always.
	*/
		{
		TInt err = KErrNone;
		TUint8* heapPtr = aChunk.Base();
		
		new (&Header(aChunk)) THeapWrapperHeader;
		
		THeapWrapperHeader& header = Header(aChunk);
		header.iFinished = ETrue;
		header.SetLockedFlag(EFalse);
		
		// placement new a new descriptor on the heap to initialise it
		new (heapPtr + KDescriptorOffset) TPtr8(heapPtr + KDataOffset, 0, aChunk.Size() - KDataOffset);

		return err;
		}

	TPtr8& CHeapWrapper::Buffer(const RChunk& aChunk)
 	/**
 	@param aChunk The RChunk to interrogate for the heap information
 	@return reference to the descriptor stored on the global shared heap
 	*/
  		{
  		TUint8* heapPtr = aChunk.Base();
		
		// Set the descriptor
		TPtr8* descriptor = reinterpret_cast<TPtr8*>(heapPtr + KDescriptorOffset);
		const TInt length = descriptor->Length();
		const TInt maxLength = descriptor->MaxLength();
		descriptor->Set(heapPtr + KDataOffset, length, maxLength);

		return *descriptor;
		}
		
	void CHeapWrapper::CleanReadBuffer()
	/**
	Delete and NULL the read buffer if necessary
	*/
		{
		delete iReadBuf;
		iReadBuf = NULL;
		}
		
	CHeapWrapper::~CHeapWrapper()
		{
		CleanReadBuffer();
		}
	}
