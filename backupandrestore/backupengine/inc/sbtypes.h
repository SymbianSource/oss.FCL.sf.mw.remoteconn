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
* Declaration of Types and Constants for Secure Backup and Restore
* 
*
*/



/**
 @file
*/
#ifndef __SBTYPES_H__
#define __SBTYPES_H__

#include <connect/sbdefs.h>
#include <e32cmn.h>
#include <f32file.h>
#include <connect/sbexternalisablearray.h>

class RApaLsSession;

namespace conn
	{
	/** Maximum length of package name
	@released
	@publishedPartner
	*/
	const TInt KMaxPackageNameLength = 128; // Arbitrary name length

	/** Length of the 128 bit Java hash in bytes
	@released
	@publishedPartner
	*/
	const TInt KJavaIdLength = 16;

	/** Used for unpacking unicode descriptors 
	@released
	@publishedPartner
	*/
	#ifdef UNICODE
	const TInt KCharWidthInBytes = 2;
	#else
	const TInt KCharWidthInBytes = 1;
	#endif // UNICODE
	
	/**
	@released
	@publishedPartner
	*/
	typedef TBufC<KMaxPackageNameLength> TPackageName;

	enum TCommonBUROptions
		/** 
		The passive backup options

		@released
		@publishedPartner
		*/
		{
		ENoOptions 				= 0x00000000, /*!< 0x00000000: no common options selected */
		EActiveBUR				= 0x00000001, /*!< 0x00000001: Does the data owner do active backup / restore? */
		EPassiveBUR				= 0x00000002, /*!< 0x00000002: Does the data owner do passive backup / restore? */
		EHasSystemFiles 		= 0x00000004, /*!< 0x00000004: Does the data owner want system files backed up? */
		ESupportsSelective		= 0x00000008, /*!< 0x00000008: Can the data owner do BUR selectively? */
		ERequiresReboot			= 0x00000010  /*!< 0x00000010: Does the data owner require reboot after restore? */
		};
	
	enum TPassiveBUROptions
		/** 
		The passive backup options

		@released
		@publishedPartner
		*/
		{
		ENoPassiveOptions 		= 0x00000000, /*!< 0x00000000: no passive options selected */
		EHasPublicFiles			= 0x00000001, /*!< 0x00000001: data owner has public files to BUR ? */
		EDeleteBeforeRestore	= 0x00000002, /*!< 0x00000002: data owner requires a delete before restore ? */
		EPassiveSupportsInc 	= 0x00000004  /*!< 0x00000004: data owner supports incremental BUR ? */
		};
	
	enum TActiveBUROptions
		/** 
		The active backup options

		@released
		@publishedPartner
		*/
		{
		ENoActiveOptions 		= 0x00000000, /*!< 0x00000000: no active backup options selected */
		EDelayToPrepareData 	= 0x00000001, /*!< 0x00000001: data owner slow to prepare active data */
		EActiveSupportsInc		= 0x00000002  /*!< 0x00000002: data owner supports incremental BUR ? */
		};
		
	/**	Common BUR Settings
	This structure will store the common backup and restore options for a SID 
	@released
	@publishedPartner	
	*/
	typedef TUint32 TCommonBURSettings;
	
	/**	Passive BUR Settings
	This structure will store the passive backup and restore options for a SID 
	@released
	@publishedPartner	
	*/	
	typedef TUint32 TPassiveBURSettings;
	
	/**	Active BUR Settings
	This structure will store the active backup and restore options for a SID 
	@released
	@publishedPartner	
	*/
	typedef TUint32 TActiveBURSettings;

	class TRestoreFileFilter
		/**
		TRestoreFileFilter packages up a file or directory name and an include or exclude flag.

		This type is used when telling a client during a restore operation which public files are
		to be restored for a specific data owner.

		@released
		@publishedPartner
		*/
		{
	public:
		/**
		Constructor
		
		@param aInclude ETrue if the file or directory is to be
						included, EFalse for exclusion
		@param aName The name of the file or directory (directory
					 names end with a slash)
		*/
		TRestoreFileFilter(TBool aInclude, const TFileName& aName)
			: iInclude(aInclude), iName(aName) {}
	public:
		TBool iInclude ; /*!< ETrue if the file or directory is to be included, EFalse for exclusion */
		TFileName iName; /*!< The name of the file or directory (directory names end with a slash) */
		};

	/** Array of TEntry objects
	@released
	@publishedPartner
	*/
	typedef RExternalisableArray<TEntry> RFileArray;

	/** Array of TRestoreFileFilter objects 
	@released
	@publishedPartner
	*/
	typedef RExternalisableArray<TRestoreFileFilter> RRestoreFileFilterArray;

	/** Array of TSecureId objects
	@released
	@publishedPartner
	*/
	typedef RExternalisableArray<TSecureId> RSIDArray;

	enum TDataOwnerStatus
		/**
		TDataOwnerStatus encapsulates the status of a data owner.

		@released
		@publishedPartner
		*/
		{
		EUnset = 0, /*!< 0x00000000: Unset - allows creation of object without incorrect status */
		EDataOwnerNotFound = 1, /*!< 0x00000001: Data owner not found in registration files */
		EDataOwnerReady = 2, /*!< 0x00000002: Data owner ready for backup / restore */
		EDataOwnerNotReady = 3, /*!< 0x00000003: Data owner not yet ready for backup / restore */
		EDataOwnerFailed = 4, /*!< 0x00000004: Data owner failed to become ready */
		EDataOwnerNotConnected = 5, /*!< 0x00000005: Data owner alive, but not fully ready */
		EDataOwnerReadyNoImpl = 6 /*!< 0x00000006: Data owner ready for backup / restore but does not provide callback implementation */
		};

	class TDataOwnerAndStatus
		/**
		TDataOwnerAndStatus encapsulates the identity and status of a data owner.
		Access to members is direct.

		@released
		@publishedPartner
		*/
		{
	public:
		/**
		Constructor
		
		@param aSID Data owner secure id - identifies the data owner
		@param aStatus Status of the data owner
		@param aDataOwnerError Error code if data owner failed to be ready
							   or KErrNone
		*/
		TDataOwnerAndStatus(TSecureId aSID, TDataOwnerStatus aStatus, TInt aDataOwnerError)
			: iSID(aSID), iStatus(aStatus), iDataOwnerError(aDataOwnerError) {}
	public:
		TSecureId iSID; /*!< Data owner secure id - identifies the data owner */
		TDataOwnerStatus iStatus; /*!< Status of the data owner */
		TInt iDataOwnerError; /*!< Error code if data owner failed to be ready or KErrNone */
		};

	/** Array of data owners with their status
	@released
	@publishedPartner
	*/
	typedef RExternalisableArray<TDataOwnerAndStatus> RSIDStatusArray;

	enum TBackupDataType
		/**
		TBackupDataType indicates whether a specific data stream or operation relates to active or
		passive backup or restore.  A specific data owner can use both active and passive and so we
		need to differentiate between operations.

		@released
		@publishedPartner
		*/
		{
		EActive = 0, /*!< 0x00000000: The backup or restore operation is active rather than passive */
		EPassive = 1 /*!< 0x00000001: The backup or restore operation is passive rather than active */
		};

	enum TTransferDataType
		/**
		TTransferDataType indicates the type of data being transferred from the Secure Backup
		Engine to the Secure Backup Server or vice versa.

		@released
		@publishedPartner
		*/
		{
		ERegistrationData = 0, /*!< 0x00000000: deprecated */
		EPassiveSnapshotData = 1, /*!< 0x00000001: The data is a snapshot for passive backup that includes file details*/
		EPassiveBaseData = 2, /*!< 0x00000002: The data is passively backup up private data with no previous history */
		EPassiveIncrementalData = 3, /*!< 0x00000003: The data is passively backed up private data as an increment on a previous base backup */
		EActiveSnapshotData = 4, /*!< 0x00000004: The data is a snapshot for active backup that includes file details*/
		EActiveBaseData = 5, /*!< 0x00000005: The data is actively backup up private data with no previous history */
		EActiveIncrementalData = 6, /*!< 0x00000006: The data is actively backed up private data as an increment on a previous base backup */
		ECentralRepositoryData = 7, /*!< 0x00000007: deprecated */
		EPublicFileListing = 8, /*!< 0x00000008: An externalised list of public file entries belonging to a single data owner */
		};
		
	enum TSBDerivedType
		/**
		Used to identify the derived type that the base generic type represents
		
		@deprecated
		@publishedPartner
		*/
		{
		ESIDDerivedType = 0, /*!< 0x00000000: SID derived type  */
		EPackageDerivedType = 1, /*!< 0x00000001: Package derived type */
		ESIDTransferDerivedType = 2, /*!< 0x00000002: SID data transfer derived type  */
		EPackageTransferDerivedType = 3, /*!< 0x00000003: Package data transfer derived type */
		EJavaDerivedType = 4, /*!< 0x00000004: Java ID derived type */
		EJavaTransferDerivedType = 5, /*!< 0x00000005: Java ID derived type */
		};
		
		/**
		Maximum number of derived types specified in TSBDerivedType

		@released
		@publishedPartner
		*/
	const TUint KMaxDerivedTypes = 6;	
		
	enum TPackageDataType
		/**
		TPackageDataType indicates the type of package data being
		transferred from the Secure Backup Engine to the Secure Backup
		Server or vice versa.

		@deprecated
		@publishedPartner
		*/
		{
		ESystemData = 0, /*!< 0x00000000: The data is system files (typically executables or resource files) */
		ESystemSnapshotData = 1, /*!< 0x00000001: The data is a snapshot for system files */
		};
		
	enum TJavaTransferType
		/**
		This enum defines the type of Java data that is being transferred and can indicate either a MIDlet 
		suite archive (Jad/Jar etc.) or data for an installed MIDlet suite
		
		@released
		@publishedPartner
		*/
		{
		EJavaMIDlet = 0,
		EJavaMIDletData = 1,
		};

	enum TTransferDirection
		/**
		TTransferDirection indicates whether a (just completed) data transfer is from the
		Secure Backup Server to a data owner or vice versa.

		@released
		@publishedPartner
		*/
		{
		EServerToDataOwner = 0, /*!< 0x00000000: The transfer is from the Secure Backup Server to a data owner */
		EDataOwnerToServer = 1/*!< 0x00000001: The transfer is from a data owner to the Secure Backup Server */
		};
		
	/** Generic Data Type is a class for representing any type of data owner so that it 
	may be passed through a common interface
		
	<H2>Introduction</H2>
	
	The CSBEClient Secure backup interface class is designed to perform backup operations 
	on many different types of data owners. This presents a problem in that the interfaces 
	sometimes require arguments that are specific to the data type in question. In addition, 
	we can't predict the new data types that may appear in future, therefore a "generic" data 
	type was proposed that could be passed through interfaces as a base class and contained 
	enough information to enable Runtime Type Identification (RTTI) to be performed.
	
	This approach means that only one interface needs to be maintained for each backup 
	operation whilst still allowing the Secure Backup Engine to determine which type was 
	originally sent by a PC client.
	
	As well as acting as an identifier for different Data Owner's (DO's), it was decided 
	to extend the usage of the class to identify Data Owner's Data as each DO may require 
	specific information to identify data being transferred (such as finished flags etc.)
	
	The identifier is transparent to the user of CSBEClient, meaning that it can be 
	extracted directly from the message sent by the PC into a generic type and passed through 
	to the SBEngine without being instantiated as a derived class.
	
	<H2>Usage</H2>
	
	The general usage of the generic data type is to extract it from a PC sent message as a 
	CSBGenericDataType object using CSBGenericDataType::NewL(const TDesC8& aDes).
	
	There are a few conventions that have been followed within the Secure Backup component 
	that a developer must bear in mind:
	
	@li Ownership of the CSBGenericDataType, CSBGenericTransferType or derived 
		object is generally passed between interfaces and as such it is the resposibility 
		of the receiving function to delete it. Unless otherwise stated in the documentation
		for an interface, this is assumed to be the case;
		
	@li When a CSBGenericDataType or CSBGenericTransferType is to be converted into 
		a derived type, it must be passed in as a parameter to the constructor of the derived 
		type. It must not be downcasted as it may never have been a derived type, and the 
		derived types contain extra member data;
		
	In order to convert a base type to a derived type, a method CSBGenericDataType::DerivedTypeL() 
	that returns a conn::TSBDerivedType can be called to query the base object for it's derived type.
	
	A typical mechanism would be to use a switch statement, switching on the return value of 
	DerivedTypeL() and taking appropriate action with regards to creating a derived object.
		
	<H2>Internal Operation</H2>
	
	<H3>Initialisation</H3>
	
	CSBGenericDataType is underneath basically a descriptor buffer. All accessor functions 
	simply extract and pack values from this descriptor. This means that the type can be 
	safely transferred via IPC between processes and safely transferred between a device 
	and a PC independently of communications bearer or transport.
	
	The base class CSBGenericDataType has ownership of the data buffer, iDataBuffer, used 
	to store the member data in and in order to keep the derived classes atomic, a running 
	total size variable is updated by the C++ constructors of each class in the inheritence 
	chain. Because of Symbian's static NewL idiom, all C++ constructors are executed before 
	CSBGenericDataType::ConstructL() is called to instantiate the buffer, hence the size 
	can be guaranteed to be big enough to hold all of the derived type's data.
	
	In order to ensure that the derived types know whereabouts in the descriptor to extract 
	and pack values, an initialisation routine, InitialiseL(TInt& aOffset), is implemented 
	by each class. Each class is responsible for calling it's parents InitialiseL(), before 
	initialising itself and passing in an offset. The job of the initialisation method is 
	to walk the descriptor, storing the offset's of it's member data. The offset is a 
	reference and is hence updated so that the child class begins unpacking after the parents 
	data ends. In this way, each class is atomic apart from having to explicitly scope it's 
	parent's InitialiseL() method.
	
	<H3>Accessing Data</H3>
	
	Data is accessed by the use of packing and unpacking bytes directly from the descriptor 
	using the UnpackType, PackType and associated function templates. These will be unpacked 
	in the endianness of the system and therefore the PC will have to ensure that this 
	is taken into account when packing the type into the message.
	
	Although this method suffers in terms of performance, for the reasons of platform 
	independence, this is the most reliable method of packing/unpacking.
	
	<H2>Byte Structure</H2>
	
	All child classes have their data following on from the parent classes so that the parent 
	classes may remain atomic with respect to the structure of their data. The binary structure 
	of this data type inside the descriptor buffer is as follows. Note that the endianness of 
	the data types specified below is determined by the platform and the PC must pack accordingly:
		
	<table border="1">
		<tr>
			<td>TSBDerivedType [4-bytes]</td>
		</tr>
	</table>


	@deprecated
	@publishedPartner
	*/
	class CSBGenericDataType : public CBase
		{
	public:
		IMPORT_C static CSBGenericDataType* NewL(const TDesC8& aDes);
		IMPORT_C ~CSBGenericDataType();
		IMPORT_C const TDesC8& Externalise() const;
		IMPORT_C TSBDerivedType DerivedTypeL() const;
		
	protected:
		CSBGenericDataType();
		void ConstructL(const TDesC8& aDes);
		void BaseConstructL();
		virtual void InitialiseL(TInt& aOffset);
		void UnpackDescriptorTypeAdvance(TDesC16& aDes, TInt& aOffset);
		void UnpackDescriptorTypeAdvance(TDesC8& aDes, TInt& aOffset);
		void UnpackTPtrAdvance(TPtrC16& aDes, TInt& aOffset);
		void UnpackTPtrAdvance(TPtrC8& aDes, TInt& aOffset);
		void PackDescriptorTypeAdvance(const TDesC16& aDes, TInt& aOffset);
		void PackDescriptorTypeAdvance(const TDesC8& aDes, TInt& aOffset);

	protected:
		/** the data buffer */
		HBufC8* iDataBuffer;

		/** Cumulative size of the derived type. All ctors in the inheritence tree increment 
		with their size */
		TInt iSize;

		/** the derived type */
		TInt iDerivedTypeOffset;
		};
		
	template<class T> inline void UnpackType(T& aType, TDesC8& aDes, TInt aOffset)
		/**
		Templated function to copy a simple type from a specified position in aDes
		defined by aOffset. aOffset is not updated on return.
		
		@param aType This pointer is set with the pointer to the type in aDes
		@param aDes The buffer that contains the type T pointed to by aPtr
		@param aOffset The running index of where the following type begins. Updated by ExtractPointer
		*/
		{
		// Set the position at which to finish copying
		TInt endOffset = aOffset + sizeof(T);
		
		TUint8* pRawType = reinterpret_cast<TUint8*>(&aType);
		TUint8* pRawSource = const_cast<TUint8*>(aDes.Ptr()) + aOffset;
		
		for (TInt offset = aOffset; offset < endOffset; offset++)
			{
			*pRawType++ = *pRawSource++;
			}
		}
		
	template<class T> inline void UnpackTypeAdvance(T& aType, TDesC8& aDes, TInt& aOffset)
		/**
		Templated function to copy a simple type from a specified position in aDes
		defined by aOffset. aOffset is advanced so that on return, the new 
		offset is stored enabling the next call to UnpackType() to point at the position 
		following this type.
		
		@param aType This pointer is set with the pointer to the type in aDes
		@param aDes The buffer that contains the type T pointed to by aPtr
		@param aOffset The running index of where the following type begins. Updated by ExtractPointer
		*/
		{
		// Set the position at which to finish copying
		TInt endOffset = aOffset + sizeof(T);
		
		TUint8* pRawType = reinterpret_cast<TUint8*>(&aType);
		TUint8* pRawSource = const_cast<TUint8*>(aDes.Ptr()) + aOffset;
		
		for (; aOffset < endOffset; aOffset++)
			{
			*pRawType++ = *pRawSource++;
			}
		}

	template<> inline void UnpackTypeAdvance(TDriveList& aType, TDesC8& aDes, TInt& aOffset)
		/**
		Templated function to copy a simple type from a specified position in aDes
		defined by aOffset. aOffset is advanced so that on return, the new 
		offset is stored enabling the next call to UnpackType() to point at the position 
		following this type.
		
		@param aType This pointer is set with the pointer to the type in aDes
		@param aDes The buffer that contains the type T pointed to by aPtr
		@param aOffset The running index of where the following type begins. Updated by ExtractPointer
		*/
		{
		TUint8* pRawSource = const_cast<TUint8*>(aDes.Ptr()) + aOffset;
		
		aType.SetMax();
		
		for (TInt index = 0; index < KMaxDrives; index++)
			{
			aType[index] = *pRawSource++;
			}

		aOffset += KMaxDrives;
		}
		
	template<class T> inline void PackType(T& aType, TDesC8& aDes, TInt aOffset)
		/**
		Templated function to copy a simple type into a specified position in aDes
		defined by aOffset. aOffset is not advanced as a result of this operation
		
		@param aType This object is overwritten with the flattened type in aDes
		@param aDes The buffer that contains the type T pointed to by aPtr
		@param aOffset The running index of where the following type begins. Updated by ExtractPointer

		@return The new offset after packing the data type
		*/
		{
		// Set the position at which to finish copying
		TInt endOffset = aOffset + sizeof(T);		
		
		TUint8* pRawType = reinterpret_cast<TUint8*>(&aType);
		TUint8* pRawBuf = const_cast<TUint8*>(aDes.Ptr()) + aOffset;
		
		for (TInt offset = aOffset; offset < endOffset; offset++)
			{
			*pRawBuf++ = *pRawType++;
			}
		}

	template<class T> inline void PackTypeAdvance(T& aType, TDesC8& aDes, TInt& aOffset)
		/**
		Templated function to copy a simple type into a specified position in aDes
		defined by aOffset. aOffset is updated to reflect the advance along aDes
		
		@param aType This object is overwritten with the flattened type in aDes
		@param aDes The buffer that contains the type T pointed to by aPtr
		@param aOffset The running index of where the following type begins. Updated by ExtractPointer

		@return The new offset after packing the data type
		*/
		{
		// Set the position at which to finish copying
		TInt endOffset = aOffset + sizeof(T);		
		
		TUint8* pRawType = reinterpret_cast<TUint8*>(&aType);
		TUint8* pRawBuf = const_cast<TUint8*>(aDes.Ptr() + aOffset);
		
		for (; aOffset < endOffset; aOffset++)
			{
			*pRawBuf++ = *pRawType++;
			}
		}
		

	/** Generic Data Type representing a data owner identified by a SecureId
	
	<H2>Byte Structure</H2>
	
	All child classes have their data following on from the parent classes so that the parent 
	classes may remain atomic with respect to the structure of their data. The binary structure 
	of this data type inside the descriptor buffer is as follows. Note that the endianness of 
	the data types specified below is determined by the platform and the PC must pack accordingly:
		
	<table border="1">
		<tr>
			<td>TSBDerivedType [4-bytes]</td>
			<td>TSecureId [4-bytes]</td>
		</tr>
	</table>
		
	@deprecated
	@publishedPartner
	*/
	class CSBSecureId : public CSBGenericDataType
		{
	public:
		IMPORT_C static CSBSecureId* NewL(TSecureId aSecureId);
		IMPORT_C static CSBSecureId* NewL(CSBGenericDataType* aGenericDataType);
		IMPORT_C ~CSBSecureId();
		IMPORT_C TSecureId SecureIdL() const;
		
	protected:
		void InitialiseL(TInt& aOffset);

	private:
		CSBSecureId();
		void ConstructL(TSecureId aSecureId);
		void ConstructL(CSBGenericDataType* aGenericDataType);
		
	private:
		/** the secure identifier */
		TInt iSecureIdOffset;
		};
		
	/** Generic Data Type representing a data owner identified by a Package Id, Secure Id and 
	Package Name. Package Id's are used to represent multiple SID's installed under a single 
	package. There would potentially be several CSBPackageId's associated with a package, each 
	detailing a single SID.

	<H2>Byte Structure</H2>
	
	All child classes have their data following on from the parent classes so that the parent 
	classes may remain atomic with respect to the structure of their data. The binary structure 
	of this data type inside the descriptor buffer is as follows. Note that the endianness of 
	the data types specified below is determined by the platform and the PC must pack accordingly:

	<table border="1">
		<tr>
			<td>TSBDerivedType [4-bytes]</td>
			<td>TUid [4-bytes]</td>
			<td>TSecureId [4-bytes]</td>
			<td>Pkg name length [4-bytes]</td>
			<td>Pkg name data [variable]</td>
		</tr>
	</table>

	@deprecated
	@publishedPartner
	*/
	class CSBPackageId : public CSBGenericDataType
		{
	public:
		IMPORT_C static CSBPackageId* NewL(TUid aPackageId, TSecureId aSecureId, const TDesC& aPackageName);
		IMPORT_C static CSBPackageId* NewL(CSBGenericDataType* aGenericDataType);
		IMPORT_C ~CSBPackageId();
		IMPORT_C TUid PackageIdL() const;
		IMPORT_C TSecureId SecureIdL() const;
		IMPORT_C TPackageName PackageNameL() const;
		
	protected:
		void InitialiseL(TInt& aOffset);
		void InitialiseL(TInt& aOffset, TInt aPackageNameLength);

	private:
		CSBPackageId();
		void ConstructL(TUid aPackageId, TSecureId aSecureId, const TDesC& aPackageName);
		void ConstructL(CSBGenericDataType* aGenericDataType);
		
	private:
		/** the package identifier */
		TInt iPackageIdOffset;
		/** the secure identifier */
		TInt iSecureIdOffset;
		/** user-readable name for the package */
		TInt iPackageNameOffset;
		};

	/** Generic Transfer Type
	
	This is also a semi-abstract class similar to the CSBGenericDataType in that logically it may 
	only be instantiated by being internalised from a descriptor. This class and classes that derive 
	from it are intended to identify a data transfer to or from a particular data owner.
	
	<H2>Byte Structure</H2>
	
	All child classes have their data following on from the parent classes so that the parent 
	classes may remain atomic with respect to the structure of their data. The binary structure 
	of this data type inside the descriptor buffer is as follows. Note that the endianness of 
	the data types specified below is determined by the platform and the PC must pack accordingly:

	<table border="1">
		<tr>
			<td>TSBDerivedType [4-bytes]</td>
			<td>TDriveNumber [4-bytes]</td>
		</tr>
	</table>
	
	@deprecated
	@publishedPartner
	*/	
	class CSBGenericTransferType : public CSBGenericDataType
		{
	public:
		IMPORT_C static CSBGenericTransferType* NewL(const TDesC8& aDes);
		IMPORT_C ~CSBGenericTransferType();
		IMPORT_C TDriveNumber DriveNumberL() const;
		
	protected:
		CSBGenericTransferType();
		virtual void InitialiseL(TInt& aOffset);
	
	protected:
		/** the drive number */
		TInt iDriveNumberOffset;		
		};

	/** SID Transfer type is used to identify data associated with a particular SID
	
	<H2>Byte Structure</H2>
	
	All child classes have their data following on from the parent classes so that the parent 
	classes may remain atomic with respect to the structure of their data. The binary structure 
	of this data type inside the descriptor buffer is as follows. Note that the endianness of 
	the data types specified below is determined by the platform and the PC must pack accordingly:

	<table border="1">
		<tr>
			<td>TSBDerivedType [4-bytes]</td>
			<td>TDriveNumber [4-bytes]</td>
			<td>TSecureId [4-bytes]</td>
			<td>TTransferDataType [4-bytes]</td>
		</tr>
	</table>

	@deprecated
	@publishedPartner
	*/
	class CSBSIDTransferType : public CSBGenericTransferType
		{
	public:
		IMPORT_C static CSBSIDTransferType* NewL(TSecureId aSecureId, TDriveNumber aDriveNumber, TTransferDataType aTransferDataType);
		IMPORT_C static CSBSIDTransferType* NewL(CSBGenericTransferType* aGenericTransferType);
		IMPORT_C ~CSBSIDTransferType();
		IMPORT_C TSecureId SecureIdL() const;
		IMPORT_C TTransferDataType DataTypeL() const;
		
	protected:
		void InitialiseL(TInt& aOffset);

	private:
		CSBSIDTransferType();
		void ConstructL(TSecureId aSecureId, TDriveNumber aDriveNumber, TTransferDataType aTransferDataType);
		void ConstructL(CSBGenericTransferType* aGenericTransferType);
		
	private:
		/** the secure identifier */
		TInt iSecureIdOffset;
		/** the transfer data type */
		TInt iTransferDataTypeOffset;
		};

	/** Package Transfer Type is used to identify data related to a particular Package ID
	
	<H2>Byte Structure</H2>
	
	All child classes have their data following on from the parent classes so that the parent 
	classes may remain atomic with respect to the structure of their data. The binary structure 
	of this data type inside the descriptor buffer is as follows. Note that the endianness of 
	the data types specified below is determined by the platform and the PC must pack accordingly:

	<table border="1">
		<tr>
			<td>TSBDerivedType [4-bytes]</td>
			<td>TDriveNumber [4-bytes]</td>
			<td>TUid [4-bytes]</td>
			<td>TPackageDataType [4-bytes]</td>
		</tr>
	</table>

	@deprecated
	@publishedPartner
	*/
	class CSBPackageTransferType : public CSBGenericTransferType
		{
	public:
		IMPORT_C static CSBPackageTransferType* NewL(TUid aPackageId, TDriveNumber aDriveNUmber, TPackageDataType aPackageDataType);
		IMPORT_C static CSBPackageTransferType* NewL(CSBGenericTransferType* aGenericTransferType);
		IMPORT_C ~CSBPackageTransferType();
		IMPORT_C TUid PackageIdL() const;
		IMPORT_C TPackageDataType DataTypeL() const;
		
	protected:
		void InitialiseL(TInt& aOffset);

	private:
		CSBPackageTransferType();
		void ConstructL(TUid aPackageId, TDriveNumber aDriveNumber, TPackageDataType aPackageDataType);
		void ConstructL(CSBGenericTransferType* aGenericTransferType);
		
	private:
		/** the package identifier */
		TInt iPackageIdOffset;
		/** the package data type */
		TInt iPackageDataTypeOffset;
		};
		
	class CDataOwnerInfo : public CBase
		/** 
		CDataOwnerInfo packages up information for the backup / restore client on an
		individual data owner.

		Access to the data members is direct.

		@deprecated
		@publishedPartner
		*/
		{
	public:								  
		IMPORT_C static CDataOwnerInfo* NewL( CSBGenericDataType* aGenericDataType,
											  TCommonBURSettings aCommonSettings,
											  TPassiveBURSettings aPassiveSettings,
											  TActiveBURSettings aActiveSettings,
											  const TDriveList& aDriveList );
											  
		IMPORT_C static CDataOwnerInfo* NewL ( const TDesC8& aFlatDataOwnerInfo );
		IMPORT_C ~CDataOwnerInfo();
		IMPORT_C CSBGenericDataType& Identifier();
		IMPORT_C const CSBGenericDataType& Identifier() const;
		IMPORT_C TCommonBURSettings CommonSettings() const;
		IMPORT_C TPassiveBURSettings PassiveSettings() const;
		IMPORT_C TActiveBURSettings ActiveSettings() const;
		IMPORT_C TDriveList& DriveList();
		IMPORT_C const TDriveList& DriveList() const;
		IMPORT_C HBufC8* ExternaliseL();
		IMPORT_C TInt Size() const;
		IMPORT_C HBufC8* ExternaliseLC();

	private:
		CDataOwnerInfo();
		void ConstructL(CSBGenericDataType* aGenericDataType, TCommonBURSettings aCommonSettings,
						TPassiveBURSettings aPassiveSettings, TActiveBURSettings aActiveSettings,
						const TDriveList& aDriveList);
		void ConstructL(const TDesC8& aFlatDataOwnerInfo);

	private:
		CSBGenericDataType* iGenericDataType; /*!< generic data type object */
		TCommonBURSettings iCommonBURSettings; /*!< flags for storing common backup and restore settings */
		TPassiveBURSettings iPassiveBURSettings; /*!< flags for storing passive backup and restore settings */
		TActiveBURSettings iActiveBURSettings; /*!< flags for storing active backup and restore settings */
		TDriveList iDriveList; /*!< List of drives that the data owner has data on */
		};

	/** Generic Data Type representing a MIDlet data owner identified by a Suite Name, Vendor Name, 
	Version and a unique MIDlet Suite Hash. The Suite Hash should be used as the unique identifier for 
	the MIDlet suite.

	<H2>Byte Structure</H2>
	
	All child classes have their data following on from the parent classes so that the parent 
	classes may remain atomic with respect to the structure of their data. The binary structure 
	of this data type inside the descriptor buffer is as follows. Note that the endianness of 
	the data types specified below is determined by the platform and the PC must pack accordingly:

	<table border="1">
		<tr>
			<td>TSBDerivedType [4-bytes]</td>
			<td>MIDlet suite name length [4-bytes]</td>
			<td>MIDlet suite name [2*length]</td>
			<td>MIDlet suite vendor length [4-bytes]</td>
			<td>MIDlet suite vendor [2*length]</td>
			<td>MIDlet suite version length [4-bytes]</td>
			<td>MIDlet suite version [2*length]</td>
			<td>MIDlet suite hash length [4-bytes] (should be 32)</td>
			<td>MIDlet suite hash [2*length]</td>
		</tr>
	</table>

	@deprecated
	@publishedPartner
	*/
	class CSBJavaId : public CSBGenericDataType
		{
	public:
		IMPORT_C static CSBJavaId* NewL(const TDesC& aSuiteName, const TDesC& aSuiteVendor,const TDesC& aSuiteVersion, const TDesC& aSuiteHash);
		IMPORT_C static CSBJavaId* NewL(CSBGenericDataType* aGenericDataType);
		IMPORT_C ~CSBJavaId();
		IMPORT_C const TDesC& SuiteNameL() const;
		IMPORT_C const TDesC& SuiteVendorL() const;
		IMPORT_C const TDesC& SuiteVersionL() const;
		IMPORT_C const TDesC& SuiteHashL() const;
		
	protected:
		void InitialiseL(TInt& aOffset);
		void InitialiseL(TInt& aOffset, const TDesC& aSuiteName, const TDesC& aSuiteVendor,const TDesC& aSuiteVersion, const TDesC& aSuiteHash);

	private:
		CSBJavaId();
		CSBJavaId(const TDesC& aSuiteName, const TDesC& aSuiteVendor,const TDesC& aSuiteVersion, const TDesC& aSuiteHash);
		void ConstructL(const TDesC& aSuiteName, const TDesC& aSuiteVendor,const TDesC& aSuiteVersion, const TDesC& aSuiteHash);
		void ConstructFromExistingL(CSBGenericDataType* aGenericDataType);
		
	private:
		/** The MIDlet suite Name */
		TPtrC iSuiteName;
		
		/** The MIDlet suite Vendor */
		TPtrC iSuiteVendor;
		
		/** The MIDlet suite Version */
		TPtrC iSuiteVersion;

		/** The MIDlet suite Hash */
		TPtrC iSuiteHash;
		};

	/** The Java Transfer Type identifies types of data belonging to particular MIDlet suites (identified 
	by the Suite Hash)
	
	<H2>Byte Structure</H2>
	
	All child classes have their data following on from the parent classes so that the parent 
	classes may remain atomic with respect to the structure of their data. The binary structure 
	of this data type inside the descriptor buffer is as follows. Note that the endianness of 
	the data types specified below is determined by the platform and the PC must pack accordingly:

	<table border="1">
		<tr>
			<td>TSBDerivedType [4-bytes]</td>
			<td>TDriveNumber [4-bytes]</td>
			<td>TJavaTransferType [4-bytes]</td>
			<td>MIDlet suite hash length [4-bytes] (should be 32)</td>
			<td>MIDlet suite hash [2*length]</td>
		</tr>
	</table>

	@deprecated
	@publishedPartner
	*/
	class CSBJavaTransferType : public CSBGenericTransferType
		{
	public:
		IMPORT_C static CSBJavaTransferType* NewL(const TDesC& aSuiteHash, TDriveNumber aDriveNumber, TJavaTransferType aTransferDataType);
		IMPORT_C static CSBJavaTransferType* NewL(CSBGenericTransferType* aGenericTransferType);
		IMPORT_C ~CSBJavaTransferType();
		IMPORT_C const TDesC& SuiteHashL() const;
		IMPORT_C TJavaTransferType DataTypeL() const;
		
	protected:
		void InitialiseL(TInt& aOffset);
		void InitialiseL(TInt& aOffset, const TDesC& aSuiteHash, TDriveNumber aDriveNumber,
			TJavaTransferType aTransferDataType);

	private:
		CSBJavaTransferType();
		CSBJavaTransferType(const TDesC& aSuiteHash);
		void ConstructL(const TDesC& aSuiteHash, TDriveNumber aDriveNumber, TJavaTransferType aTransferDataType);
		void ConstructL(CSBGenericTransferType* aGenericTransferType);
		
	private:
		/** The MIDlet suite Hash */
		TPtrC iSuiteHash;

		/** the transfer data type */
		TInt iTransferDataTypeOffset;
		};
 
	class CSBEFileEntry : public CBase
	/** This class is a more memory efficient heap based externalisable version of 
	TEntry containing just the information required for public backup. 
	TEntry contains a TFileName which is a modifiable descriptor of 512 bytes
	and therefore takes up needless memory. CSBEFileEntry on the other hand occupies
	only as much RAM as is required to represent the name and path of the file that
	it represents.
	@deprecated
	@publishedPartner
	*/
		{
	public:
		IMPORT_C static CSBEFileEntry* NewLC(const TEntry& aEntry, RApaLsSession& aSession);
		IMPORT_C static CSBEFileEntry* NewLC(const TDesC8& aStream, TInt& aBytesRead);
		IMPORT_C ~CSBEFileEntry();
		IMPORT_C HBufC8* ExternaliseLC() const;
		IMPORT_C TUint FileAttributes() const;
		IMPORT_C TInt FileSize() const;
		IMPORT_C TTime LastModified() const;
		IMPORT_C TDesC16& Filename() const;
		IMPORT_C TDesC16& MIMEType() const;
		IMPORT_C TUidType& MIMEUid();
	private:
		void ConstructL(const TEntry& aEntry, RApaLsSession& aSession);
		void InternaliseL(const TDesC8& aStream, TInt& aBytesRead);
		CSBEFileEntry();
		CSBEFileEntry(const TEntry& aEntry);
		TUint16 CalculatePackedSize() const;
	private:
	    /**
	    The individual bits within this byte indicate which attributes
	    have been set.
	    
	    @see KEntryAttNormal
		@see KEntryAttReadOnly
		@see KEntryAttHidden
	    @see KEntryAttSystem
	    */
		TUint iAtt;
		
		/**
		The size of the file in bytes.
		*/
		TInt iSize;
		
		/**
		The system time of last modification, in universal time.
		*/
		TTime iModified;

		/**
		The textual version of the file's UIDtype
		*/
		HBufC* iType;
		
		/**
		Array of UID Types corresponding to this file
		*/
		TUidType iUidType;
		
		/**
		Path and name of the file
		*/
		HBufC* iFilename;
		};
	} // end namespace
#endif
