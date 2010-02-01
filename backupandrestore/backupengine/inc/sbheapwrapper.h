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
* Definition of CHeapWrapper and THeapWrapperHeader
* 
*
*/



/**
 @file
 @internalTechnology
*/

#ifndef __HEAPWRAPPER_H__
#define __HEAPWRAPPER_H__

#include <e32std.h>
#include <connect/sbtypes.h>

namespace conn
	{
	/** KMaxGenericTypeSize must be a multiple of 4 to ensure that all structures are 4-byte aligned 
	@internalTechnology */
	const TInt KMaxGenericTypeSize = 1024;
	
	class THeapWrapperHeader
	/**
	This class is packed at the beginning of the heap and stores information about the data
	contained in the descriptor.
	
	Note that all structures in here must be 4-byte aligned, using dummy padding if necessary.
	
	@internalTechnology
	*/
		{
	public:
		inline TBool LockedFlag();
		inline void SetLockedFlag(TBool aLockedFlag);
		inline TBuf8<KMaxGenericTypeSize>& GenericTransferTypeBuffer();

	public:
		/** Finished flag, set to ETrue when the data in the descriptor 
		is the last in a multi-part transfer*/
		TBool iFinished;

	private:
		/** Flag to indicate that the heap has been locked for writing 
		by an instance of CHeapWrapper */
		TBool iLockedFlag;

		/** Generic transfer type */
		TBuf8<KMaxGenericTypeSize> iGenericDataTransferInfo;
		};
		
	inline TBool THeapWrapperHeader::LockedFlag()
	/** Getter for the private locked flag 
	@return The status of the locked flag */
		{
		return iLockedFlag;
		}
		
	inline void THeapWrapperHeader::SetLockedFlag(TBool aLockedFlag)
	/** Setter for the locked flag
	@param aLockedFlag The new state of the heap locked flag */
		{
		iLockedFlag = aLockedFlag;
		}

	inline TBuf8<KMaxGenericTypeSize>& THeapWrapperHeader::GenericTransferTypeBuffer()
	/** Getter for the generic type buffer
	@return Reference to generic type buffer
	*/
		{
		return iGenericDataTransferInfo;
		}

	
	// Offsets for defining pointers relative the the RChunk::Base() pointer

	/** Offset for the lock flag */
	const TInt KHeaderOffset = 0;
	
	/** Offset to the beginning of the main descriptor */
	const TInt KDescriptorOffset = KHeaderOffset + sizeof(THeapWrapperHeader); // lock flag is 1 byte
	
	/** Offset to the data that the TPtr8 will point to */
	const TInt KDataOffset = KDescriptorOffset + sizeof(TPtr8);
	
	class CHeapWrapper : public CBase
	/**
	This class is instantiated to ensure that data is passed over the Global Shared Heap 
	by a common interface
	
	@internalTechnology
	*/
		{
	public:
		static CHeapWrapper* NewL();
		TPtrC8& ReadBufferL(const RChunk& aChunk);
		TPtr8& WriteBufferL(const RChunk& aChunk);
		TInt ResetHeap(const RChunk& aChunk);
		THeapWrapperHeader& Header(const RChunk& aChunk) const;

		/** virtual C++ destructor */
		~CHeapWrapper();
	private:
		/** C++ constructor initialises read buffer pointer to NULL*/
		CHeapWrapper() : iReadBuf(NULL) {};

		TPtr8& Buffer(const RChunk& aChunk);
		void CleanReadBuffer();
	private:
		/** Pointer to the global buffer */
		TPtrC8* iReadBuf; 
		};

	} // namespace conn

#endif //__HEAPWRAPPER_H__
