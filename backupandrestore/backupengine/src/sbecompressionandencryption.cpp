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
// Implementation of compression and encryption
// 
//

/**
 @file
*/
#include "sbencrypt.h"
#include "sbecompressionandencryption.h"
#include "sblog.h"
#include "sbtypes.h"

#include <arc4.h>
#include <ezcompressor.h>
#include <ezdecompressor.h>

// Uncomment the next line if you want to turn off compression & encryption, ignore the warnings.
//#define TURN_OFF_COMPRESSION_AND_ENCRYPTION

namespace conn
	{
#ifndef TURN_OFF_COMPRESSION_AND_ENCRYPTION
	const TInt KJavaVirtualMachineSecureId = 0x102033e6;
	const TInt KMaxHeapSize = KMaxTInt / 2;
#endif
	/** Compression block size
	*/
	const TInt KCompressionBlockSize = 1024 * 64; // 64K
	/** Compression growth size
	
	This is calculated as the size of the block we want to compress * 1.01. As defined by the ZLib library.
	*/
	const TInt CSBECompressAndEncrypt::iCompressionGrowthSize = static_cast<TInt>(KCompressionBlockSize * 0.01) + 12 + sizeof(TCompressionHeader);
	

	template <class T> void ReadL(T& aT, TPtr8& aBuffer, TInt aStartAt = 0, TInt aToCopy = sizeof(T))
	/** Reads data from any given type into aBuffer
	
	@param aT type to put in buffer
	@param aBuffer the buffer
	@param aStartAt offset of the data
	@param aToCopy the amount of data to copy
	*/
		{
		TUint8* inData = const_cast<TUint8*>(aBuffer.Ptr());
		TUint8* outData = (reinterpret_cast<TUint8*>(&aT)) + aStartAt;
		for (TInt x = 0; x < aToCopy; x++)
			{
			*(outData++) = *(inData++);
			} // for
		}
	
	
	template<class T> void WriteL(T& aT, TPtr8& aBuffer, TInt aStartAt = 0, TInt aToCopy = sizeof(T))
	/** Writes data from aBuffer into any given type
	
	@param aT type to read in from buffer
	@param aBuffer the buffer
	@param aStartAt offset of the data
	@param aToCopy the amount of data to copy
	*/
		{
		TUint8* inData = reinterpret_cast<TUint8*>(&aT);
		TUint8* outData = (const_cast<TUint8*>(aBuffer.Ptr())) + aStartAt;
		for (TInt x = 0; x < aToCopy; x++)
			{
			*(outData++) = *(inData++);
			} // for
			
		aBuffer.SetLength(aBuffer.Length() + sizeof(T));
		}

	CSBECompressAndEncrypt* CSBECompressAndEncrypt::NewLC(CSBGenericTransferType*& apTransferType, TPtr8& aInputData)
	/** Standard Symbian constructor
	
	@param apTransferType transfer type of data.
	@param aInputData data block to be used. Start point will be changed to allow for compression.
	*/
		{
		CSBECompressAndEncrypt* self = new(ELeave) CSBECompressAndEncrypt();
		CleanupStack::PushL(self);
		self->ConstructL(apTransferType, aInputData);
		
		return self;
		}
			
	CSBECompressAndEncrypt::CSBECompressAndEncrypt() :
		iActualStart(NULL, 0), iCipher(NULL), iIsFreed(EFalse)
	/** Standard C++ Constructor
	*/
		{
		}
		
	CSBECompressAndEncrypt::~CSBECompressAndEncrypt()
	/** Standard C++ Destructor
	*/
		{
#ifndef TURN_OFF_COMPRESSION_AND_ENCRYPTION	

		if( ! iIsFreed )
			iOffsetStart->Set(iActualStart);	// free reserved space when leave occurs.
#endif
		delete iCipher;
		}
	
	void CSBECompressAndEncrypt::ConstructL(CSBGenericTransferType*& apTransferType, TPtr8& aOutputData)
	/**
		Standard Symbian second phase constructor
		
	@param apTransferType transfer type of data.
	@param aInputData data block to be used. Start point will be changed to allow for compression.
	*/
		{
		__LOG2("CSBECompressAndEncrypt::ConstructL() - START - aOutputData: 0x%08x (%d)", aOutputData.Ptr(), aOutputData.Length());

#ifndef TURN_OFF_COMPRESSION_AND_ENCRYPTION		
		// Do we need a key source and cipher?
		TSBDerivedType derivedType = apTransferType->DerivedTypeL();
		TInt extraToReserve = 0;
		if ((derivedType == ESIDTransferDerivedType) || (derivedType == EJavaTransferDerivedType))
			{
		 	TSecureId secureId;
		 	TDriveNumber driveNumber;
		 	if (derivedType == ESIDTransferDerivedType)
		 		{
	    		// Need the Sid transfertype
				CSBSIDTransferType* pSIDTransferType = CSBSIDTransferType::NewL(apTransferType);
				CleanupStack::PushL(pSIDTransferType);

		 		secureId = pSIDTransferType->SecureIdL();
		 		driveNumber = pSIDTransferType->DriveNumberL();
		 		
		 		CleanupStack::PopAndDestroy(pSIDTransferType);
		 		} // if
		 	else
		 		{
		 		// Java
		 		secureId.iId = KJavaVirtualMachineSecureId;
		 		driveNumber = apTransferType->DriveNumberL();
		 		} // else
		 	
		 	// We will be doing encryption. Get the key
		 	CSecureBUREncryptKeySource* keySource = CSecureBUREncryptKeySource::NewL();
		 	CleanupStack::PushL(keySource);
		 	
		 	// Get the key and the buffer?
		 	keySource->GetBackupKeyL(driveNumber, secureId,
		 							 iDoEncrypt, iKey, iGotBuffer, iBuffer);
			if (!iGotBuffer)
				{
				// See if there is a default buffer
				keySource->GetDefaultBufferForBackupL(driveNumber, iGotBuffer, iBuffer);
				} // if
				
		 	CleanupStack::PopAndDestroy(keySource);
		 	
		 	// Create the cipher, if needed.
		 	if (iDoEncrypt)
		 		{
		 		__LOG1("Key length: %d", iKey.Length() * 4);
			 	iCipher = CARC4::NewL(iKey);
			 	} // if
			 	
			extraToReserve = sizeof(TEncryptionHeader) + iBuffer.Size();
		 	} // if
	 	
		// Reserve the space required
		TInt numberBlocks = (aOutputData.MaxSize() / KCompressionBlockSize) + (((aOutputData.MaxSize() % KCompressionBlockSize) == 0) ? 0 : 1);
		TInt reservedSpace = (numberBlocks * iCompressionGrowthSize) + extraToReserve;
		__LOG2("CSBECompressAndEncrypt::ConstructL() - numberBlocks: %d, reservedSpace: %d", numberBlocks, reservedSpace);
		
		// Keep a copy of the acutual data block
		iActualStart.Set(const_cast<TUint8*>(aOutputData.Ptr()), 0, aOutputData.MaxSize());
		
		// Reserve the space in the input data
		if (reservedSpace > aOutputData.MaxSize())
			{
			User::Leave(KErrOverflow);
			}
		aOutputData.Set((const_cast<TUint8*>(aOutputData.Ptr()) + reservedSpace), 0, aOutputData.MaxSize() - reservedSpace);
		iOffsetStart = &aOutputData;
#endif		 	
		__LOG2("CSBECompressAndEncrypt::ConstructL() - END - aOutputData: 0x%08x (%d)", aOutputData.Ptr(), aOutputData.Length());
		}

	void CSBECompressAndEncrypt::PackL(TPtr8& aOutputData)
	/** Performs the compression and encryption
	
	@param aOutputData the compressed data
	*/
		{
		__LOG4("CSBECompressAndEncrypt::PackL() - START - aOutputData: 0x%08x (%d), iActualStart: 0x%08x (%d)", aOutputData.Ptr(), aOutputData.Length(), iActualStart.Ptr(), iActualStart.Length());
#ifndef TURN_OFF_COMPRESSION_AND_ENCRYPTION		
		// Add the encryption header
		TEncryptionHeader encryptionHeader;
		encryptionHeader.iEncrypted = iDoEncrypt;
		encryptionHeader.iBufferSize = iBuffer.Size();
		encryptionHeader.iTotalSize = sizeof(TEncryptionHeader) + encryptionHeader.iBufferSize;
		
		__LOG1("CSBECompressAndEncrypt::PackL() - Encryption Header: Encryption supported %d", encryptionHeader.iEncrypted);
		__LOG1("CSBECompressAndEncrypt::PackL() - Encryption Header: BufferSize %d", encryptionHeader.iBufferSize);
		__LOG1("CSBECompressAndEncrypt::PackL() - Encryption Header: Total Size %d",encryptionHeader.iTotalSize);
		
		// Move along
		TPtr8 encryptionOffset(const_cast<TUint8*>(iActualStart.Ptr()), 0, sizeof(TEncryptionHeader));
		iActualStart.SetLength(sizeof(TEncryptionHeader));
		
		// Add the encryption buffer
		if (encryptionHeader.iBufferSize > 0)
			{
			iActualStart.Append(reinterpret_cast<TUint8*>(const_cast<TUint16*>(iBuffer.Ptr())), iBuffer.Size());
			} // if
		
		// Temp buffers used for compression & encryption
		HBufC8* compressionBuffer = HBufC8::NewLC(KCompressionBlockSize + iCompressionGrowthSize);
		HBufC8* encryptionBuffer = NULL;
		if (iDoEncrypt)
			{
			encryptionBuffer = HBufC8::NewLC(KCompressionBlockSize + iCompressionGrowthSize);
			} // if
		
		// Do compression and encryption
		TInt location = 0;
		while (location < iOffsetStart->Size())
			{
			// Size of data to compress
			TInt toCompressSize = KCompressionBlockSize; 
			if ((iOffsetStart->Size() - location) < KCompressionBlockSize)
				{
				toCompressSize = iOffsetStart->Size() - location;
				}
			
			// Compress the data
			TPtr8 toCompress((const_cast<TUint8*>(iOffsetStart->Ptr()) + location), toCompressSize, toCompressSize);
			TPtr8 compress = compressionBuffer->Des();
			CEZCompressor::CompressL(compress, toCompress, Z_BEST_COMPRESSION);
			
			// Add the header information
			TCompressionHeader compressionHeader;
			compressionHeader.iCompressedSize = compressionBuffer->Size();
			compressionHeader.iUncompressedSize = toCompressSize;
			
			__LOG1("CSBECompressAndEncrypt::PackL() - Compression Header: Compressed %d", compressionHeader.iCompressedSize);
			__LOG1("CSBECompressAndEncrypt::PackL() - Compression Header: UnCompressed %d", compressionHeader.iUncompressedSize);
			
			if (iDoEncrypt)
				{
				encryptionBuffer->Des().Append(reinterpret_cast<TUint8*>(&compressionHeader), sizeof(TCompressionHeader));
				encryptionBuffer->Des().Append(compressionBuffer->Des());
				
				iCipher->Process(encryptionBuffer->Des(), iActualStart);
				} // if
			else
				{
				iActualStart.Append(reinterpret_cast<TUint8*>(&compressionHeader), sizeof(TCompressionHeader));
				iActualStart.Append(compressionBuffer->Des());
				} // else
			
			if (encryptionBuffer)
				{
				encryptionBuffer->Des().SetLength(0);
				}
			compressionBuffer->Des().SetLength(0);
			location += toCompressSize;
			encryptionHeader.iTotalSize += sizeof(TCompressionHeader) + compressionHeader.iCompressedSize;
			} // while
			
		// Add the encryption buffer
		WriteL(encryptionHeader, encryptionOffset, 0, sizeof(TEncryptionHeader));
			
		// Cleanup
		if (encryptionBuffer)
			{
			CleanupStack::PopAndDestroy(encryptionBuffer);
			} // if
		CleanupStack::PopAndDestroy(compressionBuffer);
	
		aOutputData.Set(iActualStart);
		iIsFreed = ETrue;
#endif		
		__LOG("CSBECompressAndEncrypt::PackL() - END");
		}
	void CSBECompressAndEncrypt::FreeReservedSpace(TPtr8& aOutputData)
		/*
		Free space researved in ConstructL if no data to be compressed.
		@param aOutputData the compressed data
		*/
		{
		__LOG("CSBECompressAndEncrypt::FreeReservedSpace() - START");

#ifndef TURN_OFF_COMPRESSION_AND_ENCRYPTION	
	
		__LOG1("CSBECompressAndEncrypt::FreeReservedSpace() aOutputData.Length(): %d", aOutputData.Length());

		aOutputData.Set(iActualStart);
		iIsFreed = ETrue;
#endif

		__LOG("CSBECompressAndEncrypt::FreeReservedSpace() - END");		
		}

	CSBEDecompressAndEncrypt* CSBEDecompressAndEncrypt::NewL()
	/** Standard Symbian constructor
	*/
		{
		CSBEDecompressAndEncrypt* self = CSBEDecompressAndEncrypt::NewLC();
		CleanupStack::Pop(self);
		
		return self;
		}
	
	CSBEDecompressAndEncrypt* CSBEDecompressAndEncrypt::NewLC()
	/** Standard Symbian constructor
	*/
		{
		CSBEDecompressAndEncrypt* self = new(ELeave) CSBEDecompressAndEncrypt();
		CleanupStack::PushL(self);
		
		return self;
		}
		
	CSBEDecompressAndEncrypt::CSBEDecompressAndEncrypt() :
		iType(ENotSet), iCurrentPtr(NULL, 0), iCount(0), iJavaHash(NULL), iCipher(NULL)
		/** Stanard C++ Constructor
		*/
		{
		Reset();
		}
		
	CSBEDecompressAndEncrypt::~CSBEDecompressAndEncrypt()
	/** Standard C++ Destructor 
	*/
		{
		delete iCipher;
		delete iJavaHash;
		delete iBuffer;
		}
		
	void CSBEDecompressAndEncrypt::SetBuffer(TDesC8& aOutputData)
		{
		iCurrentPtr.Set(const_cast<TUint8*>(aOutputData.Ptr()), aOutputData.Size(), aOutputData.Size());
		}
		
	void CSBEDecompressAndEncrypt::SetGenericTransferTypeL(CSBGenericTransferType*& apTransferType)
		{
		__LOG("CSBEDecompressAndEncrypt::SetGenericTransferTypeL() - START");
#ifndef TURN_OFF_COMPRESSION_AND_ENCRYPTION
		TSBDerivedType derivedType = apTransferType->DerivedTypeL();
		if (derivedType == ESIDTransferDerivedType)
			{
			// Need the Sid transfertype
			CSBSIDTransferType* pSIDTransferType = CSBSIDTransferType::NewL(apTransferType);
			CleanupStack::PushL(pSIDTransferType);
			
			// Do we need to perform a reset
			if ((iType != ESid) ||
				(iDriveNumber != pSIDTransferType->DriveNumberL()) ||
			    (iSecureId != pSIDTransferType->SecureIdL()))
				{
				Reset();
				
				iDriveNumber = pSIDTransferType->DriveNumberL();
				iSecureId = pSIDTransferType->SecureIdL();
				__LOG1("CSBEDecompressAndEncrypt::SetGenericTransferTypeL() - SecureId ID 0x%08x", iSecureId.iId);
				iType = ESid;
				} // if
				CleanupStack::PopAndDestroy(pSIDTransferType);
			} // if
		else if (derivedType == EJavaTransferDerivedType)
			{
	 		// Java
			CSBJavaTransferType* pJavaTransferType = CSBJavaTransferType::NewL(apTransferType);
			CleanupStack::PushL(pJavaTransferType);
			
			// Do we need to perform a reset
			const TDesC& javahash = pJavaTransferType->SuiteHashL();
			__LOG1("CSBEDecompressAndEncrypt::SetGenericTransferTypeL() - JavaHash %S", &javahash);
			if ((iType != EJava) ||
			    (iDriveNumber != pJavaTransferType->DriveNumberL()) ||
			    (iJavaHash->Des() != javahash))
				{
				Reset();
				
		 		// Store java hash
		 		delete iJavaHash;
		 		iJavaHash = NULL;
		 		iJavaHash = HBufC::NewL(javahash.Size());
		 		iJavaHash->Des().Append(javahash);
		 		
		 		iSecureId.iId = KJavaVirtualMachineSecureId;
		 		iDriveNumber = apTransferType->DriveNumberL();
		 		iType = EJava;
				}
	 		
	 		CleanupStack::PopAndDestroy(pJavaTransferType);
			} // else if
		else if (derivedType == EPackageTransferDerivedType)
			{
			// Package
			CSBPackageTransferType* pPackageTransferType = CSBPackageTransferType::NewL(apTransferType);
			CleanupStack::PushL(pPackageTransferType);
			
			// Do we need to perform a reset
			if ((iType != EPackage) ||
			    (iPackageId != pPackageTransferType->PackageIdL()))
				{
				Reset();
				iPackageId = pPackageTransferType->PackageIdL();
				__LOG1("CSBEDecompressAndEncrypt::SetGenericTransferTypeL() - Package ID 0x%08x", iPackageId.iUid);
				iType = EPackage;
				}
				
			CleanupStack::PopAndDestroy(pPackageTransferType);
			} // else if
		else
			{
			__LOG("CSBEDecompressAndEncrypt::SetGenericTransferTypeL() - DerivedType not supported");
			User::Leave(KErrNotSupported);
			} // else
#endif
		__LOG("CSBEDecompressAndEncrypt::SetGenericTransferTypeL() - END");
		}
		
	void CSBEDecompressAndEncrypt::Reset()
	/** Resets the data 
	*/
		{
		__LOG("CSBEDecompressAndEncrypt::Reset()");
		iCount = 0;
		iType = ENotSet;
		iDoDecrypt = EFalse;
		iCurrentPtr.Set(NULL, 0, 0);
		iGotCompressionHeader = EFalse;
		iCompressionSizeRead = 0;
		iEncryptionSizeRead = 0;
		iDoneDecompression = EFalse;
		iGotCipher = EFalse;
		delete iBuffer;
  		iBuffer = NULL;
		}
		
	void CSBEDecompressAndEncrypt::MoveAlongL(TPtr8& aPtr, TInt aAmount)
	/** Move a pointer along a given amount
	
	@param aPtr pointer to move
	@param aAmount amount to move
	*/
		{
		TInt newSize = aPtr.Size() - aAmount;
		// Check
		if (newSize < 0)
			{
			__LOG("CSBEDecompressAndEncrypt::MoveAlong() - Overflow");
			User::Leave(KErrOverflow);
			}
		
		aPtr.Set(const_cast<TUint8*>(aPtr.Ptr()) + aAmount, newSize, newSize);			
		}
	
	TBool CSBEDecompressAndEncrypt::NextLC(HBufC8*& apOutput, TBool& aMoreData)
	/** Decompress a block of the data
	
	@param apOutput on return the next block of uncompressed data. This must be deleted by the caller.
	@param aMoreData is there more data left in the compressed block.
	*/
		{
		__LOG("CSBEDecompressAndEncrypt::NextLC() - START");
#ifndef TURN_OFF_COMPRESSION_AND_ENCRYPTION
		if (!iGotCipher)
			{
			iGotCipher = CreateCipherL();
			if (!iGotCipher)
				{
				return EFalse;
				} // if
			}

		if (!iGotCompressionHeader)
			{
			__LOG("CSBEDecompressAndEncrypt::NextLC - No Header read yet");

			// Determine how much data we need to read to complete the header
			TInt dataAvail = iCurrentPtr.Size();
			if (iCompressionSizeRead + dataAvail > sizeof(TCompressionHeader))
				{
				dataAvail = sizeof(TCompressionHeader) - iCompressionSizeRead;
				}
			
			// Read data into header
			ReadL(iCompressionHeader, iCurrentPtr, iCompressionSizeRead, dataAvail);
			MoveAlongL(iCurrentPtr, dataAvail);
			iCompressionSizeRead += dataAvail;
			iCount += dataAvail;
			
			if (iCompressionSizeRead < sizeof(TCompressionHeader))
				{
				__LOG1("CSBEDecompressAndEncrypt::NextLC - Got partial compression header (%d bytes)",iCompressionSizeRead );
				return EFalse;
				}

			__LOG2("CSBEDecompressAndEncrypt::NextLC - Got compression header (compressed size=%d, uncompressed=%d)", 
				iCompressionHeader.iCompressedSize, iCompressionHeader.iUncompressedSize);
			
			// Was the header encrypted?
			if (iEncryptionHeader.iEncrypted)
				{
				__LOG("CSBEDecompressAndEncrypt::NextLC - Header Encrypted!");
				TCompressionHeader compressionHeader;
				TPtr8 inData(reinterpret_cast<TUint8*>(&iCompressionHeader), sizeof(TCompressionHeader), sizeof(TCompressionHeader));
				TPtr8 outData(reinterpret_cast<TUint8*>(&compressionHeader), 0, sizeof(TCompressionHeader));
				
				iCipher->Process(inData, outData);
				iCompressionHeader = compressionHeader;
				__LOG2("CSBEDecompressAndEncrypt::NextLC - unencrypted header, compressed size %d, uncompressed %d", iCompressionHeader.iCompressedSize, iCompressionHeader.iUncompressedSize);
				}
				
			iCompressionSizeRead = 0;
			iGotCompressionHeader = ETrue;
			} // if
			
		// Check the compression header is sensible
		if ((iCompressionHeader.iCompressedSize < 0) ||
			(iCompressionHeader.iUncompressedSize < 0) || 
			(iCompressionHeader.iCompressedSize >= KMaxHeapSize) ||
			(iCompressionHeader.iUncompressedSize >= KMaxHeapSize))
			{
			__LOG("CSBEDecompressAndEncrypt::NextLC() - Compression header is corrupt");
			User::Leave(KErrCorrupt);
			}

		if (!iDoneDecompression)
			{
			// Do we have enough data to decompress?
			TInt dataSize = iCurrentPtr.Size();
			__LOG1("CSBEDecompressAndEncrypt::NextLC() - Doing Decompression - data size %d", dataSize);
			if (iBuffer != NULL)
				{
				dataSize += iBuffer->Size();
				__LOG1("CSBEDecompressAndEncrypt::NextLC() - iBuffer not NULL new data size %d", dataSize)
				}
			if (dataSize < iCompressionHeader.iCompressedSize)
				{
				__LOG("CSBEDecompressAndEncrypt::NextLC() - data size < compressed size");
				// Need to buffer the buffer
				if (iBuffer == NULL)
  					{
  					__LOG1("CSBEDecompressAndEncrypt::NextLC() - Creating internal buffer of size %d",iCompressionHeader.iCompressedSize);
  					iBuffer = HBufC8::NewL(iCompressionHeader.iCompressedSize);	
  					}

				iBuffer->Des().Append(const_cast<TUint8*>(iCurrentPtr.Ptr()), iCurrentPtr.Size());
				__LOG("CSBEDecompressAndEncrypt::NextLC() - Appending data to internal buffer");
				return EFalse;
				} // if
				
			// Do we have a buffer?
			TPtr8 inData(NULL, 0);
			TInt toAppend = 0;
			if (iBuffer != NULL)
				{
				__LOG("CSBEDecompressAndEncrypt::NextLC() - Preparing inData from internal buffer");
				toAppend = iCompressionHeader.iCompressedSize - iBuffer->Des().Size();
				iBuffer->Des().Append(const_cast<TUint8*>(iCurrentPtr.Ptr()), toAppend);
				
				inData.Set(const_cast<TUint8*>(iBuffer->Des().Ptr()), iBuffer->Des().Size(), iBuffer->Des().Size());
				} // if
			else
				{
				__LOG("CSBEDecompressAndEncrypt::NextLC() - Preparing inData");
				inData.Set(const_cast<TUint8*>(iCurrentPtr.Ptr()), iCompressionHeader.iCompressedSize, iCompressionHeader.iCompressedSize);			
				} // else
				
			// Uncompress + Decrypt the buffer
			apOutput = HBufC8::NewLC(iCompressionHeader.iUncompressedSize);
			__LOG1("CSBEDecompressAndEncrypt::NextLC() - Allocated Output data for uncompressed data of size %d", iCompressionHeader.iUncompressedSize);
			if (iEncryptionHeader.iEncrypted)
				{
				__LOG("CSBEDecompressAndEncrypt::NextLC() Encrypted data, trying to allocate temp");
				// Need another temp buffer
				HBufC8* temp = HBufC8::NewLC(iCompressionHeader.iCompressedSize);
				
				// Decrypt
				TPtr8 ptrTemp = temp->Des();
				iCipher->Process(inData, ptrTemp);
				// Decompress
				TPtr8 ptrOutput = apOutput->Des();
				CEZDecompressor::DecompressL(ptrOutput, ptrTemp);
				
				// Cleanup
				CleanupStack::PopAndDestroy(temp);
				__LOG("CSBEDecompressAndEncrypt::NextLC() Decryption and decompresson done");
				} // if
			else
				{
				// Decompress
				TPtr8 ptrOutput = apOutput->Des();
				CEZDecompressor::DecompressL(ptrOutput, inData);
				__LOG("CSBEDecompressAndEncrypt::NextLC() decompresson done");
				} // else
				
			iCount += iCompressionHeader.iCompressedSize;
			if (toAppend != 0)
				{
				MoveAlongL(iCurrentPtr, toAppend);
				}
			else
				{
				MoveAlongL(iCurrentPtr, iCompressionHeader.iCompressedSize);
				}
			
			delete iBuffer;
			iBuffer = NULL;

			iDoneDecompression = ETrue;
			} // if
		
		__LOG2("CSBEDecompressAndEncrypt::NextLC() - encryption buffer done %d of %d", iCount, iEncryptionHeader.iTotalSize);

		// If the entire encrypted block has been read, prepare to read the next one
		if (iCount >= iEncryptionHeader.iTotalSize)
			{
			iGotCipher = EFalse;
			iEncryptionSizeRead = 0;
			iCount = 0;
			}
		
		// Is there more data available?
		if (iCurrentPtr.Size() == 0)
			{
			Reset();
			aMoreData = EFalse;
			}
		else
			{
			aMoreData = ETrue;
			}
			
		// Prepare to read the next compressed block
		iGotCompressionHeader = EFalse;
		iCompressionSizeRead = 0;
		iDoneDecompression = EFalse;

#else
		TInt size = g_CompressionBlockSize;
		if (size > iCurrentPtr.Size())
			{
			size = iCurrentPtr.Size();
			} // if
			
		apOutput = HBufC8::NewLC(size);
		apOutput->Des().Append(iCurrentPtr.Ptr(), size);
		MoveAlong(iCurrentPtr, size);

		// Is there more data available?
		if (iCurrentPtr.Size() == 0)
			{
			aMoreData = EFalse;
			}
		else
			{
			aMoreData = ETrue;
			}
#endif			
		__LOG("CSBEDecompressAndEncrypt::NextLC() - END");
		return ETrue;
		}
		
	TBool CSBEDecompressAndEncrypt::CreateCipherL()
	/**
	Creates the cipher to to use in decryption.
	
	@return ETrue if cipher created, otherwise EFalse.
	*/
		{
		__LOG("CSBEDecompressAndEncrypt::CreateCipherL() - START");
		
		TInt dataAvail = iCurrentPtr.Size();
		if (iEncryptionSizeRead + dataAvail > sizeof(TEncryptionHeader))
			{
			dataAvail = sizeof(TEncryptionHeader) - iEncryptionSizeRead;
			}
		
		// Read data into header
		ReadL(iEncryptionHeader, iCurrentPtr, iEncryptionSizeRead, dataAvail);
		MoveAlongL(iCurrentPtr, dataAvail);
		iEncryptionSizeRead += dataAvail;
		iCount += dataAvail;
		
		if (iEncryptionSizeRead < sizeof(TEncryptionHeader))
			{
			__LOG1("CSBEDecompressAndEncrypt::CreateCipherL - Got partial encryption header (%d bytes)",iEncryptionSizeRead);
			return EFalse;
			}

		__LOG3("CSBEDecompressAndEncrypt::CreateCipherL - Got encryption header (encrypted=%d, buffer size=%d, total size=%d)", 
			iEncryptionHeader.iEncrypted, iEncryptionHeader.iBufferSize, iEncryptionHeader.iTotalSize);
		
		// Check we have a sensible encryption header
		if ((iEncryptionHeader.iBufferSize < 0) || (iEncryptionHeader.iBufferSize >= KMaxTInt/2) ||
			(iEncryptionHeader.iTotalSize < 0))
			{
			__LOG("CSBEDecompressAndEncrypt::CreateCipherL() - Corrupt data");
			User::Leave(KErrCorrupt);
			}
		if (iEncryptionHeader.iEncrypted)
			{
			// Get the decryption key
			CSecureBUREncryptKeySource* keySource = CSecureBUREncryptKeySource::NewL();
			CleanupStack::PushL(keySource);
				
			HBufC* alignedBuffer = HBufC::NewLC(iEncryptionHeader.iBufferSize);
			TBool gotBuffer = (iEncryptionHeader.iBufferSize > 0);
			if (gotBuffer)
				{
				TUint8* in = reinterpret_cast<TUint8*>(const_cast<TUint16*>(alignedBuffer->Des().Ptr()));
				TUint8* out = const_cast<TUint8*>(iCurrentPtr.Ptr());
				
				for (TInt x = 0; x < iEncryptionHeader.iBufferSize; x++)
					{
					in[x] = out[x];
					} // for x
				alignedBuffer->Des().SetLength(iEncryptionHeader.iBufferSize / KCharWidthInBytes);
				}
			
			TBuf8<KKeySize> key;
			TBool gotKey = EFalse;
			TPtr16 ptrAlignedBuffer(alignedBuffer->Des());
			keySource->GetRestoreKeyL(iDriveNumber, iSecureId, gotBuffer, ptrAlignedBuffer, gotKey, key);
			if (!gotKey)
				{
				User::Leave(KErrCorrupt);
				}
				
		 	// Create the cipher
		 	if (iCipher)
		 		{
		 		delete iCipher;
		 		iCipher = NULL;
		 		}
		 	iCipher = CARC4::NewL(key);
				
			// Cleanup
			CleanupStack::PopAndDestroy(alignedBuffer);
			CleanupStack::PopAndDestroy(keySource);				
			} // if
			
		// Set iCount
		iCount += iEncryptionHeader.iBufferSize;
		// Move current pointer along
		MoveAlongL(iCurrentPtr, iEncryptionHeader.iBufferSize);
		__LOG("CSBEDecompressAndEncrypt::CreateCipherL() - END");
		return ETrue;
		}
		
	}


