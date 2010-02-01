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
* Declaration of CSBECompressionAndEncryption class.
* 
*
*/



/**
 @file
*/
#ifndef __SBECOMPRESSIONANDENCRYPTION_H__
#define __SBECOMPRESSIONANDENCRYPTION_H__

#include <e32base.h>

#include "sbtypes.h"

// Forwards
class CARC4;

namespace conn
	{
	// Forwards
	class CSecureBUREncryptKeySource;
	
	/** Constants used in the class below
	@internalTechnology
	*/	
	const TInt KKeySize = 32;
	/** Constants used in the class below
	@internalTechnology
	*/	
	const TInt KEncryptionBufferSize = 256;
	
	// Template classes for handling byte alignment issues
	template<class T> void ReadL(T& aT, TPtr8& aBuffer);
	template<class T> void WriteL(T& aT, TPtr8& aBuffer);
	
	/** Type: Compression Header
	@internalTechnology
	*/
	class TCompressionHeader
		{
	public:
		TInt	iCompressedSize;
		TInt	iUncompressedSize;
		};
		
	/** Type: Encryption Header
	@internalTechnology
	*/
	class TEncryptionHeader 
		{
	public:
		TBool	iEncrypted;
		TInt	iBufferSize;
		TInt	iTotalSize;
		};
	
	/** Handles compression and encryption
	@internalTechnology
	*/
	class CSBECompressAndEncrypt : public CBase
		{
		public:
			// Construtors
			static CSBECompressAndEncrypt* NewLC(CSBGenericTransferType*& apTransferType, TPtr8& aInputBlock);
			~CSBECompressAndEncrypt();
			
			// Methods
			void PackL(TPtr8& aOutputData);
			void FreeReservedSpace(TPtr8& aOutputData);
		private:
			// Constructors
			CSBECompressAndEncrypt();
			void ConstructL(CSBGenericTransferType*& apTransferType, TPtr8& aInputBlock);
		private:
			// Compression statics
			const static TInt iCompressionGrowthSize;
			
			// Members
			TBool							iDoEncrypt; /*<! To encryption */
			TBool							iGotBuffer; /*<! Do we have an encryption buffer */
			TBuf8<KKeySize>					iKey; /*<! Encryption key */
			TBuf<KEncryptionBufferSize>		iBuffer; /*<! Encryption buffer */
			TPtr8							iActualStart; /*<! Real start of data block */
			TPtr8*							iOffsetStart; /*<! Star of data */
		
			// Encryption
			CARC4*							iCipher; /*<! Used for encryption */
			TBool							iIsFreed;	/*Is freed reserved memory*/
		};
		
	/** Handles decompression and decryption
	@internalTechnology
	*/
	class CSBEDecompressAndEncrypt : public CBase
		{
		public:
			// Constructors
			static CSBEDecompressAndEncrypt* NewL();
			static CSBEDecompressAndEncrypt* NewLC();
			~CSBEDecompressAndEncrypt();
			
			// Methods
			void SetGenericTransferTypeL(CSBGenericTransferType*& apTransferType);
			void SetBuffer(TDesC8& aOutputData);
			TBool NextLC(HBufC8*& apOutput, TBool& aFinished);
		private:
			// Constructors
			CSBEDecompressAndEncrypt();

			// Methods
			void Reset();
			TBool CreateCipherL();
			void MoveAlongL(TPtr8& aPtr, TInt aAmount);
		private:
			// Enum
			enum TTransferType {ENotSet, ESid, EJava, EPackage};
			
			
			// Members
			TBool 							iDoDecrypt; /*<! Do decryption */
			TTransferType					iType;
			TPtr8							iCurrentPtr; /*<! Pointer to the current work position */
			TEncryptionHeader				iEncryptionHeader; /*<! An encryption headear */
			TDriveNumber					iDriveNumber; /*<! Drive number */
			TSecureId						iSecureId; /*<! Secure Id */
			TInt							iCount; /*<! Count of data unpacked */
			TCompressionHeader				iCompressionHeader; /*<! A compression header */
			TUid							iPackageId; /*<! A package header */
			HBufC*							iJavaHash; /*<! For storing the java hash */
			
			// Buffering
			TBool							iGotCompressionHeader; /*<! Have we got the compression header */
			TBool							iDoneDecompression;
			TBool							iGotCipher;
			TInt							iCompressionSizeRead; /*<! Used for buffering */
			TInt							iEncryptionSizeRead; 
			HBufC8*							iBuffer;
		
			// Encryption
			CARC4*							iCipher; /*<! Used for encryption */
		};
	}
#endif // __SBECOMPRESSIONANDENCRYPTION_H__
