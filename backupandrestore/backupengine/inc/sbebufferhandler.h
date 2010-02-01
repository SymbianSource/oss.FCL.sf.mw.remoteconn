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
* Declaration of Classes used to handle buffers
* 
*
*/



/**
 @file
*/
#ifndef __SBEBUFFERHANDLER_H__
#define __SBEBUFFERHANDLER_H__

#include <e32base.h>
#include <f32file.h>
#include <connect/sbtypes.h>
#include <badesca.h>

namespace conn
	{
	
	const TInt KDesCArrayGranularity = 8;
	
	/**
	This class defines the interface required to validate backup/restore data
	
	@internalTechnology
	*/
	class MValidationHandler
		{
	public:
		/**
		This method will check if the file name is in the 
		list of the files to be restored/backuped up
		
		@param aFileName reference to the filename to check
		@return ETrue if file to backup/restore is valid
		*/
		virtual TBool ValidFileL(const TDesC& aFileName) = 0;
		};
	
	/** Temporary path used for restoring midlets
	@internalTechnology
	*/
	_LIT(KMIDletTempRestorePath, "C:\\system\\temp\\MIDletRestore\\");


	/* Reads a fixed size object from a buffer
	@internalTechnology
	*/
	template<class T>
	TBool ReadFromBufferF(T& aT, TUint8*& appCurrent, const TUint8* apEnd);
	
	/* Reads a vairable size object from a buffer
	@internalTechnology
	*/
	TBool ReadFromBufferV(TPtr8& aT, TInt aSize, TUint8*& appCurrent,
						  const TUint8* apEnd);
						  
	/** Simple class to wrap up the fixed elements for a snapshot.
	@internalTechnology
	*/
	class TSnapshot
		{
	public:
		TSnapshot()
		/** Standard C++ Constructor.
		*/
			{
			}
		TSnapshot(const TDesC& aFileName, TInt64 aModified) :
			iFileName(aFileName), iModified(aModified)
		/** Standard C++ Constructor.
		@param aFileName name of file
		@param aModified modification attribute of file
		*/
			{
			}
	public:
		TFileName	iFileName; /*<! Filename */
		TInt64		iModified; /*<! The modified time of the file */
		};
		
	/** Simple class to store snapshot file
        @internalComponent
	*/
	class CSnapshot : public CBase
		{
	public:
		static CSnapshot* NewLC(const TInt64& aModified, const TDesC& aFileName);
		static CSnapshot* NewLC(const TSnapshot& aSnapshot);
		
		static TInt Compare(const CSnapshot& aFirst, const CSnapshot& aSecond);
		static TBool Match(const CSnapshot& aFirst, const CSnapshot& aSecond);

		
		~CSnapshot();
		const TInt64& Modified() const;
		const TDesC& FileName() const;
		void Snapshot(TSnapshot& aSnapshot);
		
	private:
		CSnapshot(const TInt64& aModified);
		void ConstructL(const TDesC& aFileName);
		
	private:
    	TInt64				iModified; /*<! The type of the selection */
    	HBufC*		       	iFileName; /*<! The actual selection */
		};
	
	/** Simple array of TSnapshot
	@internalTechnology
	*/
	typedef RPointerArray<CSnapshot> RSnapshots;

	/** Simple class to wrap up the fixed elements for a file.

	@internalComponent
	*/
	class TFileFixedHeader
		{
	public:
		TFileFixedHeader()
		/** Standard C++ constructor
		*/
			{
			}
		
		
		TFileFixedHeader(const TUint aFileNameLength, TUint aFileSize, 
						 TUint aAttributes, TInt64 aModified) :
			iFileNameLength(aFileNameLength), iFileSize(aFileSize), iAttributes(aAttributes), iModified(aModified)
		/** Standard C++ constructor
		*/
			{
			}
	public:
		TUint	iFileNameLength; /*<! The length of the name of the current file */
		TUint	iFileSize; /*<! The length of the current file */
		TUint	iAttributes; /*<! The attributes for the file */
		TInt64	iModified; /*<! The modifed time for the file */
		}; 

	/** Class to write one or more files to a memory buffer
	@internalTechnology
	*/
	class CBufferFileWriter : public CBase
		{
	public:
		// Constructors
		static CBufferFileWriter* NewL(RFs& aFs, CDesCArray* aFileNames);
		~CBufferFileWriter();
		
		// Methods
		void StartL(TPtr8& aBuffer, TBool& aCompleted);
		void ContinueL(TPtr8& aBuffer, TBool& aCompleted);
		void WriteToBufferL(TPtr8& aBuffer, TBool& aCompleted);
	private:
		// Constructors
		CBufferFileWriter(RFs& aFs, CDesCArray* aFileNames);
		void ConstructL();
		
	private:
		RFs&				iFs; /*<! File server to use */
		CDesCArray*			iFileNames; /*<! Array of files to write to the buffer */
		
		// State
		RFile				iFileHandle; /*<! File handle of current file writing to buffer */
		TBool				iFileOpen; /*<! Is the file open */
		TInt				iCurrentFile; /*<! Current file we are writing -> offset into array */
		TInt				iOffset; /*<! Offset into file to start writing from */
		TBool				iHeaderWritten; /*<! Has the header been written */
		TBool				iFileNameWritten; /*<! Have we written the filename? */
		}; // CBufferFileWriter
		
	/** Class to read one or more files from a memory buffer to disk
	@internalTechnology
	*/
	class CBufferFileReader : public CBase
		{
	public:
		// Constructors
		static CBufferFileReader* NewL(RFs& aFs, RSnapshots* aSnapshots = NULL, MValidationHandler* aValidationHandler = NULL);
		~CBufferFileReader();
		
		// Methods
		void StartL(const TDesC8& aBuffer, TBool aLastSection);
		void ContinueL(const TDesC8& aBuffer, TBool aLastSection);
		void ReadFromBufferL(const TDesC8& aBuffer, TBool aLastSection);
		void ReadMIDletsFromBufferL(const TDesC8& aBuffer, TBool aLastSection, 
			CDesCArray& aUnpackedFileNames);
		void Reset();
	private:
		// Constructors
		CBufferFileReader(RFs& aFs, RSnapshots* aSnapshots, MValidationHandler* aValidationHandler);
		void CheckFileInSnapshotL();
		void RecreateDirL();
		void RecreateFileL();
		TBool WriteToFileL(TUint8*& aCurrent, const TUint8* aEnd);
		void RedirectMIDletRestorePathL(const TDesC& aOriginal, CDesCArray& aRedirected);
	private:
		TFileFixedHeader iFixedHeader; 
	
		RFs&				iFs; /*<! File server to use */
		RSnapshots*			iSnapshots; /*<! Snapshot used to detect deleted files */
		
		// State
		TBool				iFixedHeaderRead; /*<! Have we read the fixed header? */
		TBool				iFileNameRead; /*<! Have we read the filename? */
		TBool				iSnapshotChecked; /*! Have we checked the snapshot */
		TBool				iRestore; /*<! Are we skipping the bytes are restoring them? */
		TInt				iLeftToSkip; /*<! If we are skipping data, how much is left? */
		TFileName			iFileName; /*<! The file name */
		RFile				iFileHandle; /*<! The file handle */
		TBool				iFileOpen; /*<! Is the file open? */
		MValidationHandler* iValidationHandler; /*<! Handler to Validation Implementation */
		TUint				iBytesRead; /*<! Number of bytes Read */
		}; // CBufferFileReader
		
		
	/** Class to write snapshot data to a memory buffer
	@internalTechnology
	*/
	class CBufferSnapshotWriter : public CBase
		{
	public:
		// Constructors
		static CBufferSnapshotWriter* NewL(RSnapshots* aSnapshot);
		~CBufferSnapshotWriter();
		
		// Methods
		void StartL(TPtr8& aBuffer, TBool& aCompleted);
		void ContinueL(TPtr8& aBuffer, TBool& aCompleted);
	private:
		// Constructors
		CBufferSnapshotWriter(RSnapshots* aSnapshot);
		void ConstructL();
		
		void WriteToBufferL(TPtr8& aBuffer, TBool& aCompleted);
	private:
		// Files
		RSnapshots*			iSnapshots; /*<! List of files */
	
		// State
		TInt				iCurrentSnapshot; /*<! Current file writing */
		}; // CBufferSnapshotWriter
		
	/** Class to read snapshot data from a memory buffer
	@internalTechnology
	*/
	class CBufferSnapshotReader : public CBase
		{
	public:
		// Constructors
		static CBufferSnapshotReader* NewL(RSnapshots& aSnapshots);
		~CBufferSnapshotReader();
		
		// Methods
		void StartL(const TDesC8& aBuffer, TBool aLastSection);
		void ContinueL(const TDesC8& aBuffer, TBool aLastSection);
	private:
		// Constructor
		CBufferSnapshotReader(RSnapshots& aSnapshots);
		
		void ReadFromBufferL(const TDesC8& aBuffer, TBool aLastSection);
	private:
		// Files
		RSnapshots&			iSnapshots; /*<! Build up list of files here */
		
		// State
		TSnapshot				iSnapshot; /*<! The fixed header */
		}; // CBufferSnapshotReader
	
	} // namespace


#endif // __SBEBUFFERHANDLER_H__
