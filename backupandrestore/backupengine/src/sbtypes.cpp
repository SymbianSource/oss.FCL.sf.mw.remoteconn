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
// Implementation of sbtypes
// 
//

/**
 @file
*/
#include <e32std.h>
#include "sbepanic.h"
#include "sbtypes.h"
#include <s32mem.h>
#include <apgcli.h>
#include <apmstd.h>
#include "OstTraceDefinitions.h"
#include "sbtrace.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "sbtypesTraces.h"
#endif

namespace conn
	{	
	EXPORT_C CDataOwnerInfo* CDataOwnerInfo::NewL( CSBGenericDataType* aGenericDataType,
										  TCommonBURSettings aCommonSettings,
										  TPassiveBURSettings aPassiveSettings,
										  TActiveBURSettings aActiveSettings,
										  const TDriveList& aDriveList )
	/**
	Symbian constructor. This constructor is used to create a CDataOwnerInfo from it's 
	constituent parts, rather than from an externalised class etc.

	@param aGenericDataType pointer to CSBGenericDataType object
	@param aCommonSettings flags for storing common backup and restore settings
	@param aPassiveSettings flags for storing passive backup and restore settings
	@param aActiveSettings flags for storing active backup and restore settings
	@param aDriveList the array of drives the data owner has data on
	@return A pointer to the CDataOwnerInfo object
	*/
		{
		OstTraceFunctionEntry0( CDATAOWNERINFO_NEWL_ENTRY );
		CDataOwnerInfo* self = new(ELeave) CDataOwnerInfo();
		CleanupStack::PushL(self);
		self->ConstructL(aGenericDataType, aCommonSettings, aPassiveSettings, aActiveSettings, aDriveList);
		CleanupStack::Pop(self);
		OstTraceFunctionExit0( CDATAOWNERINFO_NEWL_EXIT );
		return self;
		}

	EXPORT_C CDataOwnerInfo* CDataOwnerInfo::NewL( const TDesC8& aFlatDataOwnerInfo )
	/**
	Symbian constructor for constructing a CDataOwnerInfo object from an externalised CDataOwnerInfo 
	object. This may be used when taking an externalised class out of a PC message or over IPC.

	@param aFlatDataOwnerInfo a flat data owner info returned from IPC
	@return A pointer to the CDataOwnerInfo object
	*/
		{
		OstTraceFunctionEntry0( DUP1_CDATAOWNERINFO_NEWL_ENTRY );
		CDataOwnerInfo* self = new(ELeave) CDataOwnerInfo();
		CleanupStack::PushL(self);
		self->ConstructL(aFlatDataOwnerInfo);
		CleanupStack::Pop(self);
		OstTraceFunctionExit0( DUP1_CDATAOWNERINFO_NEWL_EXIT );
		return self;
		}
	
	CDataOwnerInfo::CDataOwnerInfo()
	/**
	C++ Constructor 
	*/
		{
		OstTraceFunctionEntry0( CDATAOWNERINFO_CDATAOWNERINFO_CONS_ENTRY );
		OstTraceFunctionExit0( CDATAOWNERINFO_CDATAOWNERINFO_CONS_EXIT );
		}

	EXPORT_C CDataOwnerInfo::~CDataOwnerInfo()
	/**
	C++ Destructor
	*/
		{
		OstTraceFunctionEntry0( CDATAOWNERINFO_CDATAOWNERINFO_DES_ENTRY );
		delete iGenericDataType;
		iGenericDataType = NULL;
		OstTraceFunctionExit0( CDATAOWNERINFO_CDATAOWNERINFO_DES_EXIT );
		}

	void CDataOwnerInfo::ConstructL(CSBGenericDataType* aGenericDataType,
									TCommonBURSettings aCommonSettings,
									TPassiveBURSettings aPassiveSettings,
									TActiveBURSettings aActiveSettings,
									const TDriveList& aDriveList)
	/**
	Symbian OS 2nd phase constructor.

	@param aGenericDataType pointer to CSBGenericDataType object
	@param aCommonSettings flags for storing common backup and restore settings
	@param aPassiveSettings flags for storing passive backup and restore settings
	@param aActiveSettings flags for storing active backup and restore settings
	@param aDriveList the array of drives the data owner has data on
	*/
		{
		OstTraceFunctionEntry0( CDATAOWNERINFO_CONSTRUCTL_ENTRY );
		if (aGenericDataType == NULL)
			{
		    OstTrace0(TRACE_ERROR, CDATAOWNERINFO_CONSTRUCTL, "Leave: KErrArgument");
			User::Leave(KErrArgument);
			}
		iGenericDataType = aGenericDataType;
		iCommonBURSettings = aCommonSettings;
		iPassiveBURSettings = aPassiveSettings;
		iActiveBURSettings = aActiveSettings;
		iDriveList = aDriveList;
		OstTraceFunctionExit0( CDATAOWNERINFO_CONSTRUCTL_EXIT );
		}

	void CDataOwnerInfo::ConstructL(const TDesC8& aFlatDataOwnerInfo)
	/**
	Symbian OS 2nd phase constructor. Internalise a descriptor containing a flattened 
	object of this type

	@param aFlatDataOwnerInfo a flat data owner info returned from IPC
	*/
		{
		OstTraceFunctionEntry0( DUP1_CDATAOWNERINFO_CONSTRUCTL_ENTRY );
		// TPtr8 that points to aFlatDataOwnerInfo (which remains const)
		TPtr8 pSource(const_cast<TUint8*>(aFlatDataOwnerInfo.Ptr()), aFlatDataOwnerInfo.Size());
		pSource.SetMax();
		
		// Position that we've read up to in the externalised class buffer
		TInt sourcePos = 0;
		
		TInt32 genericDataTypeSize;
		
		// Extract the CSBGenericDataType's size
		UnpackTypeAdvance(genericDataTypeSize, pSource, sourcePos);

		// Construct a new CSBGenericDataType from the message
		iGenericDataType = CSBGenericDataType::NewL(pSource.MidTPtr(sourcePos, genericDataTypeSize));
		sourcePos += genericDataTypeSize;

		UnpackTypeAdvance(iCommonBURSettings, pSource, sourcePos);
		UnpackTypeAdvance(iPassiveBURSettings, pSource, sourcePos);
		UnpackTypeAdvance(iActiveBURSettings, pSource, sourcePos);
		UnpackTypeAdvance(iDriveList, pSource, sourcePos);
		OstTraceFunctionExit0( DUP1_CDATAOWNERINFO_CONSTRUCTL_EXIT );
		}
		
	EXPORT_C CSBGenericDataType& CDataOwnerInfo::Identifier()
	/**
	Getter method for returning a reference to the CSBGenericDataType
	object
	
	@return Reference to the CSBGenericDataType object
	*/
		{
		return *iGenericDataType;
		}
		
	EXPORT_C const CSBGenericDataType& CDataOwnerInfo::Identifier() const
	/**
	Getter method for returning a reference to a const CSBGenericDataType
	object, containing information including 
	
	@return Reference to the const CSBGenericDataType object
	*/
		{
		return *iGenericDataType;
		}
		
	EXPORT_C TCommonBURSettings CDataOwnerInfo::CommonSettings() const
	/**
	Getter method for returning the Backup and Restore flags that
	the were specified in the Data Owner's backup registration file
	that relate to both Active and Passive backup
	
	@return The Common backup and restore settings of the Data Owner
	*/
		{
		return iCommonBURSettings;
		}
		
	EXPORT_C TPassiveBURSettings CDataOwnerInfo::PassiveSettings() const
	/**
	Getter method for returning the Passive Backup and Restore flags that
	the were specified in the Data Owner's backup registration file
	
	@return The passive backup and restore settings of the data owner
	*/
		{
		return iPassiveBURSettings;
		}
		
	EXPORT_C TActiveBURSettings CDataOwnerInfo::ActiveSettings() const
	/**
	Getter method for returning the Active Backup and Restore flags that
	the were specified in the Data Owner's backup registration file
	
	@return The active backup and restore settings of the data owner
	*/
		{
		return iActiveBURSettings;
		}
		
	EXPORT_C TDriveList& CDataOwnerInfo::DriveList()
	/**
	Getter method for returning the list of drives on which the Data Owner stores
	data to be backed up
	
	@return The drive list of the data owner
	*/
		{
		return iDriveList;
		}
		
	EXPORT_C const TDriveList& CDataOwnerInfo::DriveList() const
	/**
	Const getter method for returning the list of drives on which the Data Owner stores
	data to be backed up
	
	@return The drive list of the data owner
	*/
		{
		return iDriveList;
		}
		
	EXPORT_C HBufC8* CDataOwnerInfo::ExternaliseL()
	/**
	Method to externalise the data owner so that it can be passed flat over the IPC interface 
	or appended to a PC bound message, pops the pointer off the cleanup stack before it is
	returned
	
	@return The externalised buffer
	*/
		{
		OstTraceFunctionEntry0( CDATAOWNERINFO_EXTERNALISEL_ENTRY );
		HBufC8* dataOwnerBuffer = ExternaliseLC();
		CleanupStack::Pop(dataOwnerBuffer);
		
		OstTraceFunctionExit0( CDATAOWNERINFO_EXTERNALISEL_EXIT );
		return dataOwnerBuffer;
		}
		
	EXPORT_C HBufC8* CDataOwnerInfo::ExternaliseLC()
	/**
	Method to externalise the data owner so that it can be passed flat over the IPC interface 
	or appended to a PC bound message, leaves the pointer on the cleanup stack.
	
	@return The externalised buffer
	*/
		{
		OstTraceFunctionEntry0( CDATAOWNERINFO_EXTERNALISELC_ENTRY );
		HBufC8* dataBuffer = HBufC8::NewLC(Size());				
		TPtr8 bufferPtr(dataBuffer->Des());
						
		iDriveList.SetMax();	// Ensure that the drivelist array is set to max
		const TDesC8& dataType = iGenericDataType->Externalise();
		TInt32 typeSize = dataType.Size();
		
		bufferPtr.Append(reinterpret_cast<TUint8*>(&typeSize),sizeof(TInt32));
		bufferPtr.Append(dataType);
		bufferPtr.Append(reinterpret_cast<TUint8*>(&iCommonBURSettings), sizeof(TCommonBURSettings));
		bufferPtr.Append(reinterpret_cast<TUint8*>(&iPassiveBURSettings), sizeof(TPassiveBURSettings));
		bufferPtr.Append(reinterpret_cast<TUint8*>(&iActiveBURSettings), sizeof(TActiveBURSettings));
		bufferPtr.Append(iDriveList);
		
		OstTraceFunctionExit0( CDATAOWNERINFO_EXTERNALISELC_EXIT );
		return dataBuffer;
		}
		
	EXPORT_C TInt CDataOwnerInfo::Size() const
	/**
	Getter for returning the total flattened size of the object. Used for calculating the 
	size of a descriptor to fit the externalised instance of this object
	
	@return Size of class in bytes once flattened
	*/
		{
		OstTraceFunctionEntry0( CDATAOWNERINFO_SIZE_ENTRY );
		TInt size = 	sizeof(TCommonBURSettings) +
						sizeof(TPassiveBURSettings) +
						sizeof(TActiveBURSettings) +
						sizeof(TInt32) + 	// Buffer size is stored as a TInt32
						iGenericDataType->Externalise().Size() +
						iDriveList.Size();
						
		OstTraceFunctionExit0( CDATAOWNERINFO_SIZE_EXIT );
		return size;
		}
		
	// CSBGenericDataType			
	EXPORT_C CSBGenericDataType* CSBGenericDataType::NewL(const TDesC8& aDes)
	/**
	Symbian constructor. A base CSBGenericDataType may only be instantiated from a previously 
	externalised data type. This is the method that is used to take a generic data type from 
	either an IPC response or from a message sent by the PC. This method will not take ownership 
	of the descriptor aDes

	@param aDes descriptor containing the buffer of data
	@return A pointer to the CSBGenericDataType object
	*/
		{
		OstTraceFunctionEntry0( CSBGENERICDATATYPE_NEWL_ENTRY );
		CSBGenericDataType* self = new(ELeave) CSBGenericDataType();
		CleanupStack::PushL(self);
		self->ConstructL(aDes);
		CleanupStack::Pop(self);
		OstTraceFunctionExit0( CSBGENERICDATATYPE_NEWL_EXIT );
		return self;
		}
	
	CSBGenericDataType::CSBGenericDataType(): iDataBuffer(NULL)
	/**
	C++ Constructor. Any derived types must increment iSize in their C++ constructors in order 
	that when CSBGenericDataType::BaseConstructL() is called, the data buffer is initialised 
	to be large enough to accomodate the data stored by the derived type
	*/
		{
		OstTraceFunctionEntry0( CSBGENERICDATATYPE_CSBGENERICDATATYPE_CONS_ENTRY );
		// Initialise iSize
		iSize = sizeof(TSBDerivedType);
		OstTraceFunctionExit0( CSBGENERICDATATYPE_CSBGENERICDATATYPE_CONS_EXIT );
		}

	EXPORT_C CSBGenericDataType::~CSBGenericDataType()
	/**
	C++ Destructor
	*/
		{
		OstTraceFunctionEntry0( CSBGENERICDATATYPE_CSBGENERICDATATYPE_DES_ENTRY );
		delete iDataBuffer;
		iDataBuffer = NULL;
		OstTraceFunctionExit0( CSBGENERICDATATYPE_CSBGENERICDATATYPE_DES_EXIT );
		}

	void CSBGenericDataType::ConstructL(const TDesC8& aDes)
	/**
	Symbian OS 2nd phase constructor. Called when creating a base class from raw data
	i.e. off the wire/IPC. Creates the base type data buffer to store object data in

	@param aDes descriptor containing the buffer of data
	*/
		{
		OstTraceFunctionEntry0( CSBGENERICDATATYPE_CONSTRUCTL_ENTRY );
		// Store the length of the descriptor as our max length
		iSize = aDes.Size();
			
		// Create a new buffer and copy the passed one into it
		iDataBuffer = HBufC8::NewL(iSize);
		*iDataBuffer = aDes;

		TInt offset = 0;
		
		// Initialise member pointers of all classes in the inheritence tree
		InitialiseL(offset);
		
		// The base type should be as long as the des passed in (more info in a derived)
		iDataBuffer->Des().SetLength(iSize);
		OstTraceFunctionExit0( CSBGENERICDATATYPE_CONSTRUCTL_EXIT );
		}
		
	void CSBGenericDataType::InitialiseL(TInt& aOffset)
	/**
	Initialise all of member data offsets inside the base and derived classes
	
	@param 	aOffset The running offset indicating the position in the descriptor that has 
			been parsed up to
	*/
		{
		OstTraceFunctionEntry0( CSBGENERICDATATYPE_INITIALISEL_ENTRY );
		if (iSize < sizeof(TSBDerivedType))
			{
		    OstTrace0(TRACE_ERROR, CSBGENERICDATATYPE_INITIALISEL, "Leave: KErrCorrupt");
			User::Leave(KErrCorrupt);
			}
		
		// Throwaway type used to determine the offset advance
		TSBDerivedType derivedType;

		iDerivedTypeOffset = aOffset;
		UnpackTypeAdvance(derivedType, *iDataBuffer, aOffset);
		
		if (derivedType < 0 || derivedType > KMaxDerivedTypes)
			{
		    OstTrace0(TRACE_ERROR, DUP1_CSBGENERICDATATYPE_INITIALISEL, "Leave: KErrCorrupt");
			User::Leave(KErrCorrupt);
			}
				
		iDataBuffer->Des().SetLength(aOffset);
		
		OstTraceFunctionExit0( CSBGENERICDATATYPE_INITIALISEL_EXIT );
		}
	
	void CSBGenericDataType::BaseConstructL()
	/**
	Symbian OS 2nd phase constructor. Called by derived classes when creating an end 
	derived class i.e. CSBSecureId. Constructs base class data buffer to store object
	data in. Size of buffer has been determined by C++ constructors of all classes 
	in inheritence tree incrementing iSize member so that a buffer big enough to store
	the data of all classes is created.
	*/
		{
		OstTraceFunctionEntry0( CSBGENERICDATATYPE_BASECONSTRUCTL_ENTRY );
		// All derived classes should += their size to iSize
		iDataBuffer = HBufC8::NewL(iSize);
		OstTraceFunctionExit0( CSBGENERICDATATYPE_BASECONSTRUCTL_EXIT );
		}
		
	EXPORT_C const TDesC8& CSBGenericDataType::Externalise() const
	/**
	Getter for a reference to the buffer containing the data
		
	@return  Reference to the buffer containing the data
	*/
		{
		return *iDataBuffer;
		}
		
	EXPORT_C TSBDerivedType CSBGenericDataType::DerivedTypeL() const
	/**
	Getter method for returning the type of the derived object. Used to determine at run time 
	which derived type this base type contains
	
	@return The type of the derived object
	*/
		{
		OstTraceFunctionEntry0( CSBGENERICDATATYPE_DERIVEDTYPEL_ENTRY );
		TSBDerivedType derivedType;
		
		UnpackType(derivedType, *iDataBuffer, iDerivedTypeOffset);
		
		OstTraceFunctionExit0( CSBGENERICDATATYPE_DERIVEDTYPEL_EXIT );
		return derivedType;
		}

	void CSBGenericDataType::UnpackDescriptorTypeAdvance(TDesC16& aDes, TInt& aOffset)
		/**
		Function to copy a simple type from a specified position in aDes
		defined by aOffset. aOffset is advanced so that on return, the new 
		offset is stored enabling the next call to UnpackType() to point at the position 
		following this type.
		
		@param aDes The buffer that contains the type T pointed to by aPtr
		@param aOffset The running index of where the following type begins. Updated by ExtractPointer
		*/
		{
		OstTraceFunctionEntry0( CSBGENERICDATATYPE_UNPACKDESCRIPTORTYPEADVANCE_ENTRY );
		// Set length of the unicode string in characters
		TUint32 length;
	
		UnpackTypeAdvance(length, *iDataBuffer, aOffset);
		
		TUint8* pRawSource = reinterpret_cast<TUint8*>(const_cast<TUint16*>(aDes.Ptr()));
		TUint8* pRawTarget = const_cast<TUint8*>(static_cast<const TUint8*>(iDataBuffer->Ptr() + aOffset));
		
		for (TInt32 index = 0; index < (length * KCharWidthInBytes) ; index++)
			{
			*pRawTarget++ = *pRawSource++;
			aOffset++;
			}
		OstTraceFunctionExit0( CSBGENERICDATATYPE_UNPACKDESCRIPTORTYPEADVANCE_EXIT );
		}

	void CSBGenericDataType::UnpackDescriptorTypeAdvance(TDesC8& aDes, TInt& aOffset)
		/**
		Function to copy a simple type from a specified position in aDes
		defined by aOffset. aOffset is advanced so that on return, the new 
		offset is stored enabling the next call to UnpackType() to point at the position 
		following this type.
		
		@param aDes The buffer that contains the descriptor to copy from
		@param aOffset The running index of where the following type begins. Updated by ExtractPointer
		*/
		{
		OstTraceFunctionEntry0( DUP1_CSBGENERICDATATYPE_UNPACKDESCRIPTORTYPEADVANCE_ENTRY );
		// Set length of the string in 8-bit byte characters
		TUint32 length;
	
		UnpackTypeAdvance(length, *iDataBuffer, aOffset);
		
		TUint8* pRawSource = const_cast<TUint8*>(aDes.Ptr());
		TUint8* pRawTarget = const_cast<TUint8*>(iDataBuffer->Ptr() + aOffset);
		
		for (TInt32 index = 0; index < length ; index++)
			{
			*pRawTarget++ = *pRawSource++;
			aOffset++;
			}
		OstTraceFunctionExit0( DUP1_CSBGENERICDATATYPE_UNPACKDESCRIPTORTYPEADVANCE_EXIT );
		}

	void CSBGenericDataType::UnpackTPtrAdvance(TPtrC16& aDes, TInt& aOffset)
		/**
		Function to copy a simple type from a specified position in aDes
		defined by aOffset. aOffset is advanced so that on return, the new 
		offset is stored enabling the next call to UnpackType() to point at the position 
		following this type.
		
		@param aDes The buffer that contains the type T pointed to by aPtr
		@param aOffset The running index of where the following type begins. Updated by ExtractPointer
		*/
		{
		OstTraceFunctionEntry0( CSBGENERICDATATYPE_UNPACKTPTRADVANCE_ENTRY );
		// Set length of the unicode string in characters
		TInt32 length;
	
		UnpackTypeAdvance(length, *iDataBuffer, aOffset);
		
		aDes.Set(reinterpret_cast<const TUint16*>(iDataBuffer->Ptr() + aOffset), length);

		aOffset += aDes.Size();
		OstTraceFunctionExit0( CSBGENERICDATATYPE_UNPACKTPTRADVANCE_EXIT );
		}

	void CSBGenericDataType::UnpackTPtrAdvance(TPtrC8& aDes, TInt& aOffset)
		/**
		Function to copy a simple type from a specified position in aDes
		defined by aOffset. aOffset is advanced so that on return, the new 
		offset is stored enabling the next call to UnpackType() to point at the position 
		following this type.
		
		@param aDes The buffer that contains the descriptor to copy from
		@param aOffset The running index of where the following type begins. Updated by ExtractPointer
		*/
		{
		OstTraceFunctionEntry0( DUP1_CSBGENERICDATATYPE_UNPACKTPTRADVANCE_ENTRY );
		// Set length of the string in 8-bit byte characters
		TInt32 length;
	
		UnpackTypeAdvance(length, *iDataBuffer, aOffset);
		
		aDes.Set(iDataBuffer->Ptr() + aOffset, length);

		aOffset += aDes.Size();
		OstTraceFunctionExit0( DUP1_CSBGENERICDATATYPE_UNPACKTPTRADVANCE_EXIT );
		}

	void CSBGenericDataType::PackDescriptorTypeAdvance(const TDesC16& aDes, TInt& aOffset)
		/**
		Templated function to copy a simple type into a specified position in aDes
		defined by aOffset. aOffset is not advanced as a result of this operation
		
		@param aDes The buffer to copy from
		@param aOffset The running index of where the following type begins. Updated by ExtractPointer
		*/
		{
		OstTraceFunctionEntry0( CSBGENERICDATATYPE_PACKDESCRIPTORTYPEADVANCE_ENTRY );
		TInt32 length = aDes.Length();

		PackTypeAdvance(length, *iDataBuffer, aOffset);

 		TUint8* pRawSource = const_cast<TUint8*>(reinterpret_cast<const TUint8*>(aDes.Ptr()));
		TUint8* pRawTarget = const_cast<TUint8*>(reinterpret_cast<const TUint8*>(iDataBuffer->Ptr())) + aOffset;
		
		for (TInt32 index = 0; index < (length * KCharWidthInBytes); index++)
			{
			*pRawTarget++ = *pRawSource++;
			aOffset++;
			}
		OstTraceFunctionExit0( CSBGENERICDATATYPE_PACKDESCRIPTORTYPEADVANCE_EXIT );
		}

	void CSBGenericDataType::PackDescriptorTypeAdvance(const TDesC8& aDes, TInt& aOffset)
		/**
		Templated function to copy a simple type into a specified position in aDes
		defined by aOffset. aOffset is not advanced as a result of this operation
		
		@param aDes The buffer to copy from
		@param aOffset The running index of where the following type begins. Updated by ExtractPointer
		*/
		{
		OstTraceFunctionEntry0( DUP1_CSBGENERICDATATYPE_PACKDESCRIPTORTYPEADVANCE_ENTRY );
		TInt32 length = aDes.Size();

		PackTypeAdvance(length, *iDataBuffer, aOffset);

		TUint8* pRawSource = const_cast<TUint8*>(reinterpret_cast<const TUint8*>(aDes.Ptr()));
		TUint8* pRawTarget = const_cast<TUint8*>(iDataBuffer->Ptr() + aOffset);
		
		for (TInt32 index = 0; index < length; index++)
			{
			*pRawTarget++ = *pRawSource++;
			aOffset++;
			}
		OstTraceFunctionExit0( DUP1_CSBGENERICDATATYPE_PACKDESCRIPTORTYPEADVANCE_EXIT );
		}
		
		
// CSBSecureId
	EXPORT_C CSBSecureId* CSBSecureId::NewL(TSecureId aSecureId)
	/**
	Symbian constructor used for creating a derived class from it's constituent parameters. Used 
	for example when creating a derived type from scratch

	@param aSecureId the secure identifier
	@return A pointer to the CSBSecureId object
	*/
		{
		OstTraceFunctionEntry0( CSBSECUREID_NEWL_ENTRY );
		CSBSecureId* self = new(ELeave) CSBSecureId();
		CleanupStack::PushL(self);
		self->ConstructL(aSecureId);
		CleanupStack::Pop(self);
		OstTraceFunctionExit0( CSBSECUREID_NEWL_EXIT );
		return self;
		}
	
	EXPORT_C CSBSecureId* CSBSecureId::NewL(CSBGenericDataType* aGenericDataType)
	/**
	Symbian constructor used when creating a derived type from a base type, typically used 
	post-IPC in SBEngine to create a derived type from a generic type originally passed from 
	the PC

	@param aGenericDataType pointer to a CSBGenericDataType object
	@return A pointer to the CSBSecureId object
	*/
		{
		OstTraceFunctionEntry0( DUP1_CSBSECUREID_NEWL_ENTRY );
		CSBSecureId* self = new(ELeave) CSBSecureId();
		CleanupStack::PushL(self);
		self->ConstructL(aGenericDataType);
		CleanupStack::Pop(self);
		OstTraceFunctionExit0( DUP1_CSBSECUREID_NEWL_EXIT );
		return self;
		}
		
	CSBSecureId::CSBSecureId()
	/**
	C++ Constructor. Any derived types must increment iSize in their C++ constructors in order 
	that when CSBGenericDataType::BaseConstructL() is called, the data buffer is initialised 
	to be large enough to accomodate the data stored by the derived type
	*/
		{
		OstTraceFunctionEntry0( CSBSECUREID_CSBSECUREID_CONS_ENTRY );
		iSize += sizeof(TSecureId);
		OstTraceFunctionExit0( CSBSECUREID_CSBSECUREID_CONS_EXIT );
		}

	EXPORT_C CSBSecureId::~CSBSecureId()
	/**
	C++ Destructor
	*/
		{
		OstTraceFunctionEntry0( CSBSECUREID_CSBSECUREID_DES_ENTRY );
		OstTraceFunctionExit0( CSBSECUREID_CSBSECUREID_DES_EXIT );
		}

	void CSBSecureId::InitialiseL(TInt& aOffset)
	/**
	Initialise all member pointers to data
	
	@param aOffset The running offset of where the descriptor has been parsed up to
	*/
		{
		OstTraceFunctionEntry0( CSBSECUREID_INITIALISEL_ENTRY );
		// Throwaway type used for sizeof information only
		TSecureId sid;
		
		CSBGenericDataType::InitialiseL(aOffset);
		iSecureIdOffset = aOffset;

		UnpackTypeAdvance(sid, *iDataBuffer, aOffset);
		
		// Increment the size of the data buffer
		iDataBuffer->Des().SetLength(aOffset);
		OstTraceFunctionExit0( CSBSECUREID_INITIALISEL_EXIT );
		}

	void CSBSecureId::ConstructL(TSecureId aSecureId)
	/**
	Symbian OS 2nd phase constructor.

	@param aSecureId the secure identifier
	*/
		{
		OstTraceFunctionEntry0( CSBSECUREID_CONSTRUCTL_ENTRY );
		TInt offset = 0;
		// Call the Base ConstructL in order to allocate the buffer
		BaseConstructL();
		
		TSBDerivedType derivedType = ESIDDerivedType;
		PackType(derivedType, *iDataBuffer, offset);
		
		InitialiseL(offset);
		
		TSecureId sid = aSecureId;
		
		PackType(sid, *iDataBuffer, iSecureIdOffset);
		OstTraceFunctionExit0( CSBSECUREID_CONSTRUCTL_EXIT );
		}
		
	void CSBSecureId::ConstructL(CSBGenericDataType* aGenericDataType)
	/**
	Symbian 2nd phase constructor. Validates the externalised type, then passes it to the base ConstructL

	@param aGenericDataType pointer to a CSBGenericDataType object
	*/
		{
		OstTraceFunctionEntry0( DUP1_CSBSECUREID_CONSTRUCTL_ENTRY );
		if (aGenericDataType == NULL)
			{
		    OstTrace0(TRACE_ERROR, CSBSECUREID_CONSTRUCTL, "Leave: KErrArgument");
			User::Leave(KErrArgument);
			}
		if (ESIDDerivedType != aGenericDataType->DerivedTypeL())
			{
		    OstTrace0(TRACE_ERROR, DUP1_CSBSECUREID_CONSTRUCTL, "Leave: KErrArgument");
			User::Leave(KErrArgument);
			}
		// If the descriptor is different to the size we're expecting then it's not correct
		const TDesC8& des = aGenericDataType->Externalise();
		if (des.Size() != iSize)
			{
		    OstTrace0(TRACE_ERROR, DUP2_CSBSECUREID_CONSTRUCTL, "Leave: KErrArgument");
			User::Leave(KErrArgument);
			}
		
		// Call the base class ConstructL
		CSBGenericDataType::ConstructL(des);
		OstTraceFunctionExit0( DUP1_CSBSECUREID_CONSTRUCTL_EXIT );
		}
		
	EXPORT_C TSecureId CSBSecureId::SecureIdL() const
	/**
	Getter for the secure identifier
		
	@return  The secure identifier
	*/
		{
		OstTraceFunctionEntry0( CSBSECUREID_SECUREIDL_ENTRY );
		TSecureId sid;
		
		UnpackType(sid, *iDataBuffer, iSecureIdOffset);
		
		OstTraceFunctionExit0( CSBSECUREID_SECUREIDL_EXIT );
		return sid;
		}
	
	
	// CSBPackageId
	EXPORT_C CSBPackageId* CSBPackageId::NewL(TUid aPackageId, TSecureId aSecureId, const TDesC& aPackageName)
	/**
	Symbian constructor used for creating a derived class from it's constituent parameters. Used 
	for example when creating a derived type from scratch

	@param aPackageId  the package identifier
	@param aSecureId  the secure identifier
	@param aPackageName  the name of the package
	@return A pointer to the CSBPackageId object
	*/
		{
		OstTraceFunctionEntry0( CSBPACKAGEID_NEWL_ENTRY );
		CSBPackageId* self = new(ELeave) CSBPackageId();
		CleanupStack::PushL(self);
		self->ConstructL(aPackageId, aSecureId, aPackageName);
		CleanupStack::Pop(self);
		OstTraceFunctionExit0( CSBPACKAGEID_NEWL_EXIT );
		return self;
		}
	
	EXPORT_C CSBPackageId* CSBPackageId::NewL(CSBGenericDataType* aGenericDataType)
	/**
	Symbian constructor used when creating a derived type from a base type, typically used 
	post-IPC in SBEngine to create a derived type from a generic type originally passed from 
	the PC

	@param aGenericDataType  pointer to a CSBGenericDataType object
	@return A pointer to the CSBPackageId object
	*/
		{
		OstTraceFunctionEntry0( DUP1_CSBPACKAGEID_NEWL_ENTRY );
		CSBPackageId* self = new(ELeave) CSBPackageId();
		CleanupStack::PushL(self);
		self->ConstructL(aGenericDataType);
		CleanupStack::Pop(self);
		OstTraceFunctionExit0( DUP1_CSBPACKAGEID_NEWL_EXIT );
		return self;
		}	
	
	CSBPackageId::CSBPackageId()
	/**
	C++ Constructor. Any derived types must increment iSize in their C++ constructors in order 
	that when CSBGenericDataType::BaseConstructL() is called, the data buffer is initialised 
	to be large enough to accomodate the data stored by the derived type
	*/
		{
		OstTraceFunctionEntry0( CSBPACKAGEID_CSBPACKAGEID_CONS_ENTRY );
		iSize += (sizeof(TUid) + sizeof(TSecureId) + sizeof(TPackageName));
		OstTraceFunctionExit0( CSBPACKAGEID_CSBPACKAGEID_CONS_EXIT );
		}

	EXPORT_C CSBPackageId::~CSBPackageId()
	/**
	C++ Destructor
	*/
		{
		OstTraceFunctionEntry0( CSBPACKAGEID_CSBPACKAGEID_DES_ENTRY );
		OstTraceFunctionExit0( CSBPACKAGEID_CSBPACKAGEID_DES_EXIT );
		}


	void CSBPackageId::InitialiseL(TInt& aOffset, TInt aPackageNameLength)
	/**
	Initialise all member pointers to data
	
	@param aOffset The running offset of where the descriptor has been parsed up to
	@param aPackageNameLength The lenght of the package name
	*/
		{
		OstTraceFunctionEntry0( CSBPACKAGEID_INITIALISEL_ENTRY );
		TUid pkgId;
		TSecureId sid;
		TPackageName pkgName;
		
		CSBGenericDataType::InitialiseL(aOffset);

		iPackageIdOffset = aOffset;
		// Assign the pointer to the package Id position in the buffer
		UnpackTypeAdvance(pkgId, *iDataBuffer, aOffset);

		iSecureIdOffset = aOffset;
		// Assign the pointer to the secure Id position in the buffer
		UnpackTypeAdvance(sid, *iDataBuffer, aOffset);
		
		iPackageNameOffset = aOffset;
		aOffset += sizeof(TInt);
		aOffset += aPackageNameLength * KCharWidthInBytes;
		
		// Increment the size of the data buffer
		iDataBuffer->Des().SetLength(aOffset);
		OstTraceFunctionExit0( CSBPACKAGEID_INITIALISEL_EXIT );
		}
		
	void CSBPackageId::InitialiseL(TInt& aOffset)
	/**
	Initialise all member pointers to data
	
	@param aOffset The running offset of where the descriptor has been parsed up to
	*/
		{
		OstTraceFunctionEntry0( DUP1_CSBPACKAGEID_INITIALISEL_ENTRY );
		TUid pkgId;
		TSecureId sid;
		TPackageName pkgName;
		
		CSBGenericDataType::InitialiseL(aOffset);

		iPackageIdOffset = aOffset;
		// Assign the pointer to the package Id position in the buffer
		UnpackTypeAdvance(pkgId, *iDataBuffer, aOffset);

		iSecureIdOffset = aOffset;
		// Assign the pointer to the secure Id position in the buffer
		UnpackTypeAdvance(sid, *iDataBuffer, aOffset);
		
		iPackageNameOffset = aOffset;
		// Assign the pointer to the package name position
		
		TInt size = *(const_cast<TUint8*>(iDataBuffer->Des().Ptr()) + aOffset);
		aOffset += 4;
		aOffset += size * KCharWidthInBytes;
		
		// Increment the size of the data buffer
		iDataBuffer->Des().SetLength(aOffset);
		OstTraceFunctionExit0( DUP1_CSBPACKAGEID_INITIALISEL_EXIT );
		}
		

	void CSBPackageId::ConstructL(TUid aPackageId, TSecureId aSecureId,
								  const TDesC& aPackageName)
	/**
	Symbian OS 2nd phase constructor.

	@param aPackageId  the package identifier
	@param aSecureId  the secure identifier
	@param aPackageName  the name of the package
	*/
		{
		OstTraceFunctionEntry0( CSBPACKAGEID_CONSTRUCTL_ENTRY );
		// Call the Base ConstructL in order to allocate the buffer
		BaseConstructL();
		
		TInt offset = 0;
		
		TSBDerivedType derivedType = EPackageDerivedType;
		PackType(derivedType, *iDataBuffer, offset);
		
		// Initialise all member pointers right up the inheritence tree
		InitialiseL(offset, aPackageName.Length());

		// Set the passed in data into the data buffer
		TUid packageId = aPackageId;
		TSecureId secureId = aSecureId;
		TPackageName packageName(aPackageName);
		
		PackType(packageId, *iDataBuffer, iPackageIdOffset);
		PackType(secureId, *iDataBuffer, iSecureIdOffset);
		PackType(packageName, *iDataBuffer, iPackageNameOffset);
		OstTraceFunctionExit0( CSBPACKAGEID_CONSTRUCTL_EXIT );
		}

	void CSBPackageId::ConstructL(CSBGenericDataType* aGenericDataType)
	/**
	Symbian 2nd phase constructor. Validates the externalised type, then passes it to the base ConstructL

	@param aGenericDataType  pointer to a CSBGenericDataType object
	*/
		{
		OstTraceFunctionEntry0( DUP1_CSBPACKAGEID_CONSTRUCTL_ENTRY );
		if (aGenericDataType == NULL)
			{
		    OstTrace0(TRACE_ERROR, CSBPACKAGEID_CONSTRUCTL, "Leave: KErrArgument");
			User::Leave(KErrArgument);
			}
		if (EPackageDerivedType != aGenericDataType->DerivedTypeL())
			{
		    OstTrace0(TRACE_ERROR, DUP1_CSBPACKAGEID_CONSTRUCTL, "Leave: KErrArgument");
			User::Leave(KErrArgument);
			}
		const TDesC8& des = aGenericDataType->Externalise();
		
		if (des.Size() > iSize)
			{
		    OstTrace0(TRACE_ERROR, DUP2_CSBPACKAGEID_CONSTRUCTL, "Leave: KErrArgument");
			User::Leave(KErrArgument);
			}
		// Call the base class ConstructL
		CSBGenericDataType::ConstructL(des);
		OstTraceFunctionExit0( DUP1_CSBPACKAGEID_CONSTRUCTL_EXIT );
		}
		
	EXPORT_C TUid CSBPackageId::PackageIdL() const
	/**
	Getter for the package identifier
		
	@return  The package identifier
	*/
		{
		OstTraceFunctionEntry0( CSBPACKAGEID_PACKAGEIDL_ENTRY );
		TUid pkgId;
		
		UnpackType(pkgId, *iDataBuffer, iPackageIdOffset);
		
		OstTraceFunctionExit0( CSBPACKAGEID_PACKAGEIDL_EXIT );
		return pkgId;
		}
		
	EXPORT_C TSecureId CSBPackageId::SecureIdL() const
	/**
	Getter for the secure identifier
		
	@return  The secure identifier
	*/
		{
		OstTraceFunctionEntry0( CSBPACKAGEID_SECUREIDL_ENTRY );
		TSecureId secureId;
		
		UnpackType(secureId, *iDataBuffer, iSecureIdOffset);
		
		OstTraceFunctionExit0( CSBPACKAGEID_SECUREIDL_EXIT );
		return secureId;
		}

	EXPORT_C TPackageName CSBPackageId::PackageNameL() const
	/**
	Getter for the package name
		
	@return  Reference to the package name
	*/
		{
		OstTraceFunctionEntry0( CSBPACKAGEID_PACKAGENAMEL_ENTRY );
		TPackageName pkgName;
		
		UnpackType(pkgName, *iDataBuffer, iPackageNameOffset);
		
		OstTraceFunctionExit0( CSBPACKAGEID_PACKAGENAMEL_EXIT );
		return pkgName;
		}


	// CSBGenericDataType
	EXPORT_C CSBGenericTransferType* CSBGenericTransferType::NewL(const TDesC8& aDes)
	/**
	Symbian constructor

	@param aDes descriptor containing the buffer of data
	@return A pointer to the CSBGenericTransferType object
	*/
		{
		OstTraceFunctionEntry0( CSBGENERICTRANSFERTYPE_NEWL_ENTRY );
		CSBGenericTransferType* self = new(ELeave) CSBGenericTransferType();
		CleanupStack::PushL(self);
		self->CSBGenericDataType::ConstructL(aDes);
		CleanupStack::Pop(self);
		OstTraceFunctionExit0( CSBGENERICTRANSFERTYPE_NEWL_EXIT );
		return self;
		}

	void CSBGenericTransferType::InitialiseL(TInt& aOffset)
	/**
	Initialise all member pointers to data
	
	@param aOffset The running offset of where the descriptor has been parsed up to
	*/
		{
		OstTraceFunctionEntry0( CSBGENERICTRANSFERTYPE_INITIALISEL_ENTRY );
		CSBGenericDataType::InitialiseL(aOffset);
		
		if (iSize < (aOffset + sizeof(TDriveNumber)))
			{
		    OstTrace0(TRACE_ERROR, CSBGENERICTRANSFERTYPE_INITIALISEL, "Leave: KErrCorrupt");
			User::Leave(KErrCorrupt);
			}
		
		// Dummy data to get sizeof information from
		TDriveNumber driveNum;
		
		iDriveNumberOffset = aOffset;
		UnpackTypeAdvance(driveNum, *iDataBuffer, aOffset);
		
		// Increment the size of the data buffer
		iDataBuffer->Des().SetLength(aOffset);
		OstTraceFunctionExit0( CSBGENERICTRANSFERTYPE_INITIALISEL_EXIT );
		}
	
	CSBGenericTransferType::CSBGenericTransferType()
	/**
	C++ Constructor. Any derived types must increment iSize in their C++ constructors in order 
	that when CSBGenericDataType::BaseConstructL() is called, the data buffer is initialised 
	to be large enough to accomodate the data stored by the derived type
	*/
		{
		OstTraceFunctionEntry0( CSBGENERICTRANSFERTYPE_CSBGENERICTRANSFERTYPE_CONS_ENTRY );
		iSize += sizeof(TDriveNumber);
		OstTraceFunctionExit0( CSBGENERICTRANSFERTYPE_CSBGENERICTRANSFERTYPE_CONS_EXIT );
		}

	EXPORT_C CSBGenericTransferType::~CSBGenericTransferType()
	/**
	C++ Destructor
	*/
		{
		OstTraceFunctionEntry0( CSBGENERICTRANSFERTYPE_CSBGENERICTRANSFERTYPE_DES_ENTRY );
		OstTraceFunctionExit0( CSBGENERICTRANSFERTYPE_CSBGENERICTRANSFERTYPE_DES_EXIT );
		}

	EXPORT_C TDriveNumber CSBGenericTransferType::DriveNumberL() const
	/**
	Getter method for returning the drive number
	
	@return The drive number
	*/
		{
		OstTraceFunctionEntry0( CSBGENERICTRANSFERTYPE_DRIVENUMBERL_ENTRY );
		TDriveNumber driveNum;
		
		UnpackType(driveNum, *iDataBuffer, iDriveNumberOffset);
		
		OstTraceFunctionExit0( CSBGENERICTRANSFERTYPE_DRIVENUMBERL_EXIT );
		return driveNum;
		}
		
	// CSBSIDTransferType
	EXPORT_C CSBSIDTransferType* CSBSIDTransferType::NewL(TSecureId aSecureId, TDriveNumber aDriveNumber, TTransferDataType aTransferDataType)
	/**
	Symbian constructor used for creating a derived class from it's constituent parameters. Used 
	for example when creating a derived type from scratch

	@param aSecureId the secure identifier
	@param aDriveNumber the drive that contains the data
	@param aTransferDataType the type of the data you wish to transfer
	@return A pointer to the CSBSIDTransferType object
	*/
		{
		OstTraceFunctionEntry0( CSBSIDTRANSFERTYPE_NEWL_ENTRY );
		CSBSIDTransferType* self = new(ELeave) CSBSIDTransferType();
		CleanupStack::PushL(self);
		self->ConstructL(aSecureId, aDriveNumber, aTransferDataType);
		CleanupStack::Pop(self);
		OstTraceFunctionExit0( CSBSIDTRANSFERTYPE_NEWL_EXIT );
		return self;
		}
	
	EXPORT_C CSBSIDTransferType* CSBSIDTransferType::NewL(CSBGenericTransferType* aGenericTransferType)
	/**
	Symbian constructor used when creating a derived type from a base type, typically used 
	post-IPC in SBEngine to create a derived type from a generic type originally passed from 
	the PC

	@param aGenericTransferType pointer to a CSBGenericTransferType object
	@return A pointer to the CSBSIDTransferType object
	*/
		{
		OstTraceFunctionEntry0( DUP1_CSBSIDTRANSFERTYPE_NEWL_ENTRY );
		CSBSIDTransferType* self = new(ELeave) CSBSIDTransferType();
		CleanupStack::PushL(self);
		self->ConstructL(aGenericTransferType);
		CleanupStack::Pop(self);
		OstTraceFunctionExit0( DUP1_CSBSIDTRANSFERTYPE_NEWL_EXIT );
		return self;
		}	
	
	CSBSIDTransferType::CSBSIDTransferType()
	/**
	C++ Constructor. Any derived types must increment iSize in their C++ constructors in order 
	that when CSBGenericDataType::BaseConstructL() is called, the data buffer is initialised 
	to be large enough to accomodate the data stored by the derived type
	*/
		{
		OstTraceFunctionEntry0( CSBSIDTRANSFERTYPE_CSBSIDTRANSFERTYPE_CONS_ENTRY );
		iSize += (sizeof(TSecureId) + sizeof(TTransferDataType));
		OstTraceFunctionExit0( CSBSIDTRANSFERTYPE_CSBSIDTRANSFERTYPE_CONS_EXIT );
		}

	EXPORT_C CSBSIDTransferType::~CSBSIDTransferType()
	/**
	C++ Destructor
	*/
		{
		OstTraceFunctionEntry0( CSBSIDTRANSFERTYPE_CSBSIDTRANSFERTYPE_DES_ENTRY );
		OstTraceFunctionExit0( CSBSIDTRANSFERTYPE_CSBSIDTRANSFERTYPE_DES_EXIT );
		}

	void CSBSIDTransferType::InitialiseL(TInt& aOffset)
	/**
	Initialise all member pointers to data
	
	@param aOffset The running offset of where the descriptor has been parsed up to
	*/
		{
		OstTraceFunctionEntry0( CSBSIDTRANSFERTYPE_INITIALISEL_ENTRY );
		CSBGenericTransferType::InitialiseL(aOffset);

		TSecureId sid;
		TTransferDataType transType;

		iSecureIdOffset = aOffset;
		UnpackTypeAdvance(sid, *iDataBuffer, aOffset);
		
		iTransferDataTypeOffset = aOffset;
		UnpackTypeAdvance(transType, *iDataBuffer, aOffset);
		
		// Increment the size of the data buffer
		iDataBuffer->Des().SetLength(aOffset);
		OstTraceFunctionExit0( CSBSIDTRANSFERTYPE_INITIALISEL_EXIT );
		}

	void CSBSIDTransferType::ConstructL(TSecureId aSecureId, TDriveNumber aDriveNumber,
										TTransferDataType aTransferDataType)
	/**
	Symbian OS 2nd phase constructor.

	@param aSecureId the secure identifier
	@param aDriveNumber the drive that contains the data
	@param aTransferDataType the type of the data you wish to transfer
	*/
		{
		OstTraceFunctionEntry0( CSBSIDTRANSFERTYPE_CONSTRUCTL_ENTRY );
		TInt offset = 0;

		BaseConstructL();

		TSBDerivedType derivedType = ESIDTransferDerivedType;
		PackType(derivedType, *iDataBuffer, offset);
				
		InitialiseL(offset);

		TDriveNumber driveNum = aDriveNumber;
		TSecureId sid = aSecureId;
		TTransferDataType transType = aTransferDataType;

		PackType(driveNum, *iDataBuffer, iDriveNumberOffset);
		PackType(sid, *iDataBuffer, iSecureIdOffset);
		PackType(transType, *iDataBuffer, iTransferDataTypeOffset);
		OstTraceFunctionExit0( CSBSIDTRANSFERTYPE_CONSTRUCTL_EXIT );
		}

	void CSBSIDTransferType::ConstructL(CSBGenericTransferType* aGenericTransferType)
	/**
	Symbian 2nd phase constructor. Validates the externalised type, then passes it to the base ConstructL

	@param aGenericTransferType pointer to a CSBGenericTransferType object
	*/
		{
		OstTraceFunctionEntry0( DUP1_CSBSIDTRANSFERTYPE_CONSTRUCTL_ENTRY );
		if (aGenericTransferType == NULL)
			{
		    OstTrace0(TRACE_ERROR, CSBSIDTRANSFERTYPE_CONSTRUCTL, "Leave: KErrArgument");
			User::Leave(KErrArgument);
			}
		if (ESIDTransferDerivedType != aGenericTransferType->DerivedTypeL())
			{
		    OstTrace0(TRACE_ERROR, DUP1_CSBSIDTRANSFERTYPE_CONSTRUCTL, "Leave: KErrArgument");
			User::Leave(KErrArgument);
			}
		// If the descriptor is different to the size we're expecting then it's not correct
		const TDesC8& des = aGenericTransferType->Externalise();
		if (des.Size() != iSize)
			{
		    OstTrace0(TRACE_ERROR, DUP2_CSBSIDTRANSFERTYPE_CONSTRUCTL, "Leave: KErrArgument");
			User::Leave(KErrArgument);
			}
		
		// Call the base class ConstructL
		CSBGenericDataType::ConstructL(des);
		OstTraceFunctionExit0( DUP1_CSBSIDTRANSFERTYPE_CONSTRUCTL_EXIT );
		}
		
	EXPORT_C TSecureId CSBSIDTransferType::SecureIdL() const
	/**
	Getter for the secure identifier
		
	@return  The secure identifier
	*/
		{
		OstTraceFunctionEntry0( CSBSIDTRANSFERTYPE_SECUREIDL_ENTRY );
		TSecureId sid;
		
		UnpackType(sid, *iDataBuffer, iSecureIdOffset);
		
		OstTraceFunctionExit0( CSBSIDTRANSFERTYPE_SECUREIDL_EXIT );
		return sid;
		}

	EXPORT_C TTransferDataType CSBSIDTransferType::DataTypeL() const
	/**
	Getter for the transfer data type
		
	@return  The transfer data type
	*/
		{
		OstTraceFunctionEntry0( CSBSIDTRANSFERTYPE_DATATYPEL_ENTRY );
		TTransferDataType transType;
		
		UnpackType(transType, *iDataBuffer, iTransferDataTypeOffset);

		OstTraceFunctionExit0( CSBSIDTRANSFERTYPE_DATATYPEL_EXIT );
		return transType;
		}
		
		
	// CSBPackageTransferType
	EXPORT_C CSBPackageTransferType* CSBPackageTransferType::NewL(TUid aPackageId, TDriveNumber aDriveNumber,
																  TPackageDataType aPackageDataType)
	/**
	Symbian constructor used for creating a derived class from it's constituent parameters. Used 
	for example when creating a derived type from scratch

	@param aPackageId the package identifier
	@param aDriveNumber the drive that contains the data
	@param aPackageDataType the type of the package data
	@return A pointer to the CSBPackageTransferType object
	*/
		{
		OstTraceFunctionEntry0( CSBPACKAGETRANSFERTYPE_NEWL_ENTRY );
		CSBPackageTransferType* self = new(ELeave) CSBPackageTransferType();
		CleanupStack::PushL(self);
		self->ConstructL(aPackageId, aDriveNumber, aPackageDataType);
		CleanupStack::Pop(self);
		OstTraceFunctionExit0( CSBPACKAGETRANSFERTYPE_NEWL_EXIT );
		return self;
		}
	
	EXPORT_C CSBPackageTransferType* CSBPackageTransferType::NewL(CSBGenericTransferType* aGenericTransferType)
	/**
	Symbian constructor used when creating a derived type from a base type, typically used 
	post-IPC in SBEngine to create a derived type from a generic type originally passed from 
	the PC

	@param aGenericTransferType pointer to a CSBGenericTransferType object
	@return A pointer to the CSBPackageTransferType object
	*/
		{
		OstTraceFunctionEntry0( DUP1_CSBPACKAGETRANSFERTYPE_NEWL_ENTRY );
		CSBPackageTransferType* self = new(ELeave) CSBPackageTransferType();
		CleanupStack::PushL(self);
		self->ConstructL(aGenericTransferType);
		CleanupStack::Pop(self);
		OstTraceFunctionExit0( DUP1_CSBPACKAGETRANSFERTYPE_NEWL_EXIT );
		return self;
		}	
	
	CSBPackageTransferType::CSBPackageTransferType()
	/**
	C++ Constructor. Any derived types must increment iSize in their C++ constructors in order 
	that when CSBGenericDataType::BaseConstructL() is called, the data buffer is initialised 
	to be large enough to accomodate the data stored by the derived type
	*/
		{
		OstTraceFunctionEntry0( CSBPACKAGETRANSFERTYPE_CSBPACKAGETRANSFERTYPE_CONS_ENTRY );
		iSize += (sizeof(TUid) + sizeof(TPackageDataType));
		OstTraceFunctionExit0( CSBPACKAGETRANSFERTYPE_CSBPACKAGETRANSFERTYPE_CONS_EXIT );
		}

	void CSBPackageTransferType::InitialiseL(TInt& aOffset)
	/**
	Initialise all member pointers to data
	
	@param aOffset The running offset of where the descriptor has been parsed up to
	*/
		{
		OstTraceFunctionEntry0( CSBPACKAGETRANSFERTYPE_INITIALISEL_ENTRY );
		CSBGenericTransferType::InitialiseL(aOffset);

		TUid pkgId;
		TPackageDataType pkgDataType;
		
		iPackageIdOffset = aOffset;
		UnpackTypeAdvance(pkgId, *iDataBuffer, aOffset);
		
		iPackageDataTypeOffset = aOffset;
		UnpackTypeAdvance(pkgDataType, *iDataBuffer, aOffset);
		
		// Increment the size of the data buffer
		iDataBuffer->Des().SetLength(aOffset);
		OstTraceFunctionExit0( CSBPACKAGETRANSFERTYPE_INITIALISEL_EXIT );
		}

	EXPORT_C CSBPackageTransferType::~CSBPackageTransferType()
	/**
	C++ Destructor
	*/
		{
		OstTraceFunctionEntry0( CSBPACKAGETRANSFERTYPE_CSBPACKAGETRANSFERTYPE_DES_ENTRY );
		OstTraceFunctionExit0( CSBPACKAGETRANSFERTYPE_CSBPACKAGETRANSFERTYPE_DES_EXIT );
		}

	void CSBPackageTransferType::ConstructL(TUid aPackageId, TDriveNumber aDriveNumber,
										TPackageDataType aPackageDataType)
	/**
	Symbian OS 2nd phase constructor.

	@param aPackageId the secure identifier
	@param aDriveNumber the drive that contains the data
	@param aPackageDataType the type of the data you wish to transfer
	*/
		{
		OstTraceFunctionEntry0( CSBPACKAGETRANSFERTYPE_CONSTRUCTL_ENTRY );
		TInt offset = 0;

		BaseConstructL();

		TSBDerivedType derivedType = EPackageTransferDerivedType;
		PackType(derivedType, *iDataBuffer, offset);
				
		InitialiseL(offset);

		TPackageDataType pkgDataType = aPackageDataType;
		TDriveNumber driveNum = aDriveNumber;
		TUid pkgId = aPackageId;
		
		PackType(driveNum, *iDataBuffer, iDriveNumberOffset);
		PackType(pkgId, *iDataBuffer, iPackageIdOffset);
		PackType(pkgDataType, *iDataBuffer, iPackageDataTypeOffset);
		OstTraceFunctionExit0( CSBPACKAGETRANSFERTYPE_CONSTRUCTL_EXIT );
		}

	void CSBPackageTransferType::ConstructL(CSBGenericTransferType* aGenericTransferType)
	/**
	Symbian 2nd phase constructor. Validates the externalised type, then passes it to the base ConstructL

	@param aGenericTransferType pointer to a CSBGenericTransferType object
	*/
		{
		OstTraceFunctionEntry0( DUP1_CSBPACKAGETRANSFERTYPE_CONSTRUCTL_ENTRY );
		if (aGenericTransferType == NULL)
			{
		    OstTrace0(TRACE_ERROR, CSBPACKAGETRANSFERTYPE_CONSTRUCTL, "Leave: KErrArgument");
			User::Leave(KErrArgument);
			}
		if (EPackageTransferDerivedType != aGenericTransferType->DerivedTypeL())
			{
		    OstTrace0(TRACE_ERROR, DUP1_CSBPACKAGETRANSFERTYPE_CONSTRUCTL, "Leave: KErrArgument");
			User::Leave(KErrArgument);
			}
		// If the descriptor is different to the size we're expecting then it's not correct
		const TDesC8& des = aGenericTransferType->Externalise();
		
		if (des.Size() != iSize)
			{
		    OstTrace0(TRACE_ERROR, DUP2_CSBPACKAGETRANSFERTYPE_CONSTRUCTL, "Leave: KErrArgument");
			User::Leave(KErrArgument);
			}
		
		// Call the base class ConstructL
		CSBGenericDataType::ConstructL(des);
		OstTraceFunctionExit0( DUP1_CSBPACKAGETRANSFERTYPE_CONSTRUCTL_EXIT );
		}
		
	EXPORT_C TUid CSBPackageTransferType::PackageIdL() const
	/**
	Getter for the package identifier
		
	@return  The package identifier
	*/
		{
		OstTraceFunctionEntry0( CSBPACKAGETRANSFERTYPE_PACKAGEIDL_ENTRY );
		TUid pkgId;
		
		UnpackType(pkgId, *iDataBuffer, iPackageIdOffset);
		
		OstTraceFunctionExit0( CSBPACKAGETRANSFERTYPE_PACKAGEIDL_EXIT );
		return pkgId;
		}

	EXPORT_C TPackageDataType CSBPackageTransferType::DataTypeL() const
	/**
	Getter for the package data type
		
	@return  The package data type
	*/
		{
		OstTraceFunctionEntry0( CSBPACKAGETRANSFERTYPE_DATATYPEL_ENTRY );
		TPackageDataType pkgDataType;
		
		UnpackType(pkgDataType, *iDataBuffer, iPackageDataTypeOffset);
		
		OstTraceFunctionExit0( CSBPACKAGETRANSFERTYPE_DATATYPEL_EXIT );
		return pkgDataType;
		}
		
// JavaID

	EXPORT_C CSBJavaId* CSBJavaId::NewL(const TDesC& aSuiteName, const TDesC& aSuiteVendor,
		const TDesC& aSuiteVersion, const TDesC& aSuiteHash)
	/**
	Symbian constructor used for creating a derived class from it's constituent parameters. Used 
	for example when creating a derived type from scratch

	@param aSuiteName  the name of the java suite
	@param aSuiteVendor  the suite vendor
	@param aSuiteVersion  the version of the java suite
	@param aSuiteHash the hash of the java suite
	@return A pointer to the CSBJavaId object
	*/
		{
		OstTraceFunctionEntry0( CSBJAVAID_NEWL_ENTRY );
		CSBJavaId* self = new(ELeave) CSBJavaId(aSuiteName, aSuiteVendor, aSuiteVersion, aSuiteHash);
		CleanupStack::PushL(self);
		self->ConstructL(aSuiteName, aSuiteVendor, aSuiteVersion, aSuiteHash);
		CleanupStack::Pop(self);
		OstTraceFunctionExit0( CSBJAVAID_NEWL_EXIT );
		return self;
		}
	
	EXPORT_C CSBJavaId* CSBJavaId::NewL(CSBGenericDataType* aGenericDataType)
	/**
	Symbian constructor used when creating a derived type from a base type, typically used 
	post-IPC in SBEngine to create a derived type from a generic type originally passed from 
	the PC

	@param aGenericDataType  pointer to a CSBGenericDataType object
	@return A pointer to the CSBJavaId object
	*/
		{
		OstTraceFunctionEntry0( DUP1_CSBJAVAID_NEWL_ENTRY );
		CSBJavaId* self = new(ELeave) CSBJavaId;
		CleanupStack::PushL(self);
		self->ConstructFromExistingL(aGenericDataType);
		CleanupStack::Pop(self);
		OstTraceFunctionExit0( DUP1_CSBJAVAID_NEWL_EXIT );
		return self;
		}	
		
	CSBJavaId::CSBJavaId()
		{
		OstTraceFunctionEntry0( CSBJAVAID_CSBJAVAID_CONS_ENTRY );
		OstTraceFunctionExit0( CSBJAVAID_CSBJAVAID_CONS_EXIT );
		}
	
	CSBJavaId::CSBJavaId(const TDesC& aSuiteName, const TDesC& aSuiteVendor,
		const TDesC& aSuiteVersion, const TDesC& aSuiteHash)
	/**
	C++ Constructor. Any derived types must increment iSize in their C++ constructors in order 
	that when CSBGenericDataType::BaseConstructL() is called, the data buffer is initialised 
	to be large enough to accomodate the data stored by the derived type
	
	@param aSuiteName The name of the MIDlet suite
	@param aSuiteVendor The name of the vendor of the MIDlet suiet
	@param aSuiteHash The Java MIDlet suite hash
	@param aSuiteVersion The version of the MIDlet suite
	*/
		{
		OstTraceFunctionEntry0( DUP1_CSBJAVAID_CSBJAVAID_CONS_ENTRY );
		iSize += (4 * sizeof(TUint32)) + aSuiteName.Size() + aSuiteVendor.Size() + aSuiteVersion.Size() 
			+ aSuiteHash.Size();
		OstTraceFunctionExit0( DUP1_CSBJAVAID_CSBJAVAID_CONS_EXIT );
		}

	EXPORT_C CSBJavaId::~CSBJavaId()
	/**
	C++ Destructor
	*/
		{
		OstTraceFunctionEntry0( CSBJAVAID_CSBJAVAID_DES_ENTRY );
		OstTraceFunctionExit0( CSBJAVAID_CSBJAVAID_DES_EXIT );
		}

	void CSBJavaId::InitialiseL(TInt& aOffset)
	/**
	Initialise all member pointers to data
	
	@param aOffset The running offset of where the descriptor has been parsed up to
	*/
		{
		OstTraceFunctionEntry0( CSBJAVAID_INITIALISEL_ENTRY );
		CSBGenericDataType::InitialiseL(aOffset);
		
		UnpackTPtrAdvance(iSuiteName, aOffset);
		UnpackTPtrAdvance(iSuiteVendor, aOffset);
		UnpackTPtrAdvance(iSuiteVersion, aOffset);
		UnpackTPtrAdvance(iSuiteHash, aOffset);

		// Increment the size of the data buffer
		iDataBuffer->Des().SetLength(aOffset);
		OstTraceFunctionExit0( CSBJAVAID_INITIALISEL_EXIT );
		}

	void CSBJavaId::InitialiseL(TInt& aOffset, const TDesC& aSuiteName, const TDesC& aSuiteVendor,
		const TDesC& aSuiteVersion, const TDesC& aSuiteHash)
	/**
	Initialise all member pointers to data
	
	@param aOffset The running offset of where the descriptor has been parsed up to
	@param aSuiteName The name of the MIDlet suite
	@param aSuiteVendor The name of the vendor of the MIDlet suiet
	@param aSuiteHash The Java MIDlet suite hash
	@param aSuiteVersion The version of the MIDlet suite
	*/
		{
		OstTraceFunctionEntry0( DUP1_CSBJAVAID_INITIALISEL_ENTRY );
		TSBDerivedType derivedType = EJavaDerivedType;
		
		PackType(derivedType, *iDataBuffer, aOffset);
		
		CSBGenericDataType::InitialiseL(aOffset);

		// Set the derived type in the buffer
		
		TInt ptrOffset = aOffset;

		PackDescriptorTypeAdvance(aSuiteName, aOffset);
		PackDescriptorTypeAdvance(aSuiteVendor, aOffset);
		PackDescriptorTypeAdvance(aSuiteVersion, aOffset);
		PackDescriptorTypeAdvance(aSuiteHash, aOffset);
		
		UnpackTPtrAdvance(iSuiteName, ptrOffset);
		UnpackTPtrAdvance(iSuiteVendor, ptrOffset);
		UnpackTPtrAdvance(iSuiteVersion, ptrOffset);
		UnpackTPtrAdvance(iSuiteHash, ptrOffset);
		
		// Increment the size of the data buffer
		iDataBuffer->Des().SetLength(aOffset);
		OstTraceFunctionExit0( DUP1_CSBJAVAID_INITIALISEL_EXIT );
		}
		
	void CSBJavaId::ConstructL(const TDesC& aSuiteName, const TDesC& aSuiteVendor, 
		const TDesC& aSuiteVersion, const TDesC& aSuiteHash)
	/**
	Symbian OS 2nd phase constructor.

	@param aSuiteName The name of the MIDlet suite
	@param aSuiteVendor The name of the vendor of the MIDlet suiet
	@param aSuiteHash The Java MIDlet suite hash
	@param aSuiteVersion The version of the MIDlet suite
	*/
		{
		OstTraceFunctionEntry0( CSBJAVAID_CONSTRUCTL_ENTRY );
		// Call the Base ConstructL in order to allocate the buffer
		BaseConstructL();
		
		TInt offset = 0;
		
		// Initialise all member pointers right up the inheritence tree
		InitialiseL(offset, aSuiteName, aSuiteVendor, aSuiteVersion, aSuiteHash);
		OstTraceFunctionExit0( CSBJAVAID_CONSTRUCTL_EXIT );
		}

	void CSBJavaId::ConstructFromExistingL(CSBGenericDataType* aGenericDataType)
	/**
	Symbian 2nd phase constructor. Validates the externalised type, then passes it to the base ConstructL

	@param aGenericDataType  pointer to a CSBGenericDataType object
	*/
		{
		OstTraceFunctionEntry0( CSBJAVAID_CONSTRUCTFROMEXISTINGL_ENTRY );
		if (aGenericDataType == NULL)
			{
		    OstTrace0(TRACE_ERROR, CSBJAVAID_CONSTRUCTFROMEXISTINGL, "Leave: KErrArgument");
			User::Leave(KErrArgument);
			}
		if (EJavaDerivedType != aGenericDataType->DerivedTypeL())
			{
		    OstTrace0(TRACE_ERROR, DUP1_CSBJAVAID_CONSTRUCTFROMEXISTINGL, "Leave: KErrArgument");
			User::Leave(KErrArgument);
			}
		// Call the base class ConstructL
		CSBGenericDataType::ConstructL(aGenericDataType->Externalise());
		OstTraceFunctionExit0( CSBJAVAID_CONSTRUCTFROMEXISTINGL_EXIT );
		}
		
	EXPORT_C const TDesC& CSBJavaId::SuiteNameL() const
	/**
	Getter for the MIDlet suite name
		
	@return  The package identifier
	*/
		{
		return iSuiteName;
		}

	EXPORT_C const TDesC& CSBJavaId::SuiteVendorL() const
	/**
	Getter for the MIDlet suite vendor
		
	@return  The package identifier
	*/
		{
		return iSuiteVendor;
		}

	EXPORT_C const TDesC& CSBJavaId::SuiteVersionL() const
	/**
	Getter for the MIDlet suite version
		
	@return  The package identifier
	*/
		{
		return iSuiteVersion;
		}

	EXPORT_C const TDesC& CSBJavaId::SuiteHashL() const
	/**
	Getter for the MIDlet suite hash
		
	@return  The package identifier
	*/
		{
		return iSuiteHash;
		}

// CSBJavaTransferType
	EXPORT_C CSBJavaTransferType* CSBJavaTransferType::NewL(const TDesC& aSuiteHash, TDriveNumber aDriveNumber, TJavaTransferType aTransferDataType)
	/**
	Symbian constructor used for creating a derived class from it's constituent parameters. Used 
	for example when creating a derived type from scratch

	@param aSecureId the secure identifier
	@param aDriveNumber the drive that contains the data
	@param aTransferDataType the type of the data you wish to transfer
	@return A pointer to the CSBJavaTransferType object
	*/
		{
		OstTraceFunctionEntry0( CSBJAVATRANSFERTYPE_NEWL_ENTRY );
		CSBJavaTransferType* self = new(ELeave) CSBJavaTransferType(aSuiteHash);
		CleanupStack::PushL(self);
		self->ConstructL(aSuiteHash, aDriveNumber, aTransferDataType);
		CleanupStack::Pop(self);
		OstTraceFunctionExit0( CSBJAVATRANSFERTYPE_NEWL_EXIT );
		return self;
		}
	
	EXPORT_C CSBJavaTransferType* CSBJavaTransferType::NewL(CSBGenericTransferType* aGenericTransferType)
	/**
	Symbian constructor used when creating a derived type from a base type, typically used 
	post-IPC in SBEngine to create a derived type from a generic type originally passed from 
	the PC

	@param aGenericTransferType pointer to a CSBGenericTransferType object
	@return A pointer to the CSBJavaTransferType object
	*/
		{
		OstTraceFunctionEntry0( DUP1_CSBJAVATRANSFERTYPE_NEWL_ENTRY );
		CSBJavaTransferType* self = new(ELeave) CSBJavaTransferType;
		CleanupStack::PushL(self);
		self->ConstructL(aGenericTransferType);
		CleanupStack::Pop(self);
		OstTraceFunctionExit0( DUP1_CSBJAVATRANSFERTYPE_NEWL_EXIT );
		return self;
		}
		
	CSBJavaTransferType::CSBJavaTransferType()
	/**
	C++ Constructor
	*/
		{
		OstTraceFunctionEntry0( CSBJAVATRANSFERTYPE_CSBJAVATRANSFERTYPE_CONS_ENTRY );
		OstTraceFunctionExit0( CSBJAVATRANSFERTYPE_CSBJAVATRANSFERTYPE_CONS_EXIT );
		}
	
	CSBJavaTransferType::CSBJavaTransferType(const TDesC& aSuiteHash)
	/**
	C++ Constructor. Any derived types must increment iSize in their C++ constructors in order 
	that when CSBGenericDataType::BaseConstructL() is called, the data buffer is initialised 
	to be large enough to accomodate the data stored by the derived type
	*/
		{
		OstTraceFunctionEntry0( DUP1_CSBJAVATRANSFERTYPE_CSBJAVATRANSFERTYPE_CONS_ENTRY );
		iSize += aSuiteHash.Size() + sizeof(TUint32) + sizeof(TJavaTransferType);
		OstTraceFunctionExit0( DUP1_CSBJAVATRANSFERTYPE_CSBJAVATRANSFERTYPE_CONS_EXIT );
		}

	EXPORT_C CSBJavaTransferType::~CSBJavaTransferType()
	/**
	C++ Destructor
	*/
		{
		OstTraceFunctionEntry0( CSBJAVATRANSFERTYPE_CSBJAVATRANSFERTYPE_DES_ENTRY );
		OstTraceFunctionExit0( CSBJAVATRANSFERTYPE_CSBJAVATRANSFERTYPE_DES_EXIT );
		}

	void CSBJavaTransferType::InitialiseL(TInt& aOffset)
	/**
	Initialise all member pointers to data
	
	@param aOffset The running offset of where the descriptor has been parsed up to
	*/
		{
		OstTraceFunctionEntry0( CSBJAVATRANSFERTYPE_INITIALISEL_ENTRY );
		CSBGenericTransferType::InitialiseL(aOffset);
		
		iTransferDataTypeOffset = aOffset;
		TJavaTransferType transType;
		UnpackTypeAdvance(transType, *iDataBuffer, aOffset);
		
		UnpackTPtrAdvance(iSuiteHash, aOffset);

		// Increment the size of the data buffer
		iDataBuffer->Des().SetLength(aOffset);
		OstTraceFunctionExit0( CSBJAVATRANSFERTYPE_INITIALISEL_EXIT );
		}

	void CSBJavaTransferType::InitialiseL(TInt& aOffset, const TDesC& aSuiteHash, TDriveNumber aDriveNumber,
										TJavaTransferType aTransferDataType)
	/**
	Initialise all member pointers to data
	
	@param aOffset The running offset of where the descriptor has been parsed up to
	@param aSuiteHash The Java MIDlet suite hash
	@param aSuiteVersion The version of the MIDlet suite
	*/
		{
		OstTraceFunctionEntry0( DUP1_CSBJAVATRANSFERTYPE_INITIALISEL_ENTRY );
		TSBDerivedType derivedType = EJavaTransferDerivedType;
		PackType(derivedType, *iDataBuffer, aOffset);
		
		CSBGenericTransferType::InitialiseL(aOffset);

		// Set the derived type in the buffer
		TDriveNumber driveNum = aDriveNumber;
		TJavaTransferType transType = aTransferDataType;

		PackType(driveNum, *iDataBuffer, iDriveNumberOffset);
		iTransferDataTypeOffset = aOffset;

		PackTypeAdvance(transType, *iDataBuffer, aOffset);

		TInt ptrOffset = aOffset;
		
		// Pack the descriptor in raw
		PackDescriptorTypeAdvance(aSuiteHash, aOffset);
		
		// Make the member TPtr point to it
		UnpackTPtrAdvance(iSuiteHash, ptrOffset);
		
		// Increment the size of the data buffer
		iDataBuffer->Des().SetLength(aOffset);
		OstTraceFunctionExit0( DUP1_CSBJAVATRANSFERTYPE_INITIALISEL_EXIT );
		}

	void CSBJavaTransferType::ConstructL(const TDesC& aSuiteHash, TDriveNumber aDriveNumber,
										TJavaTransferType aTransferDataType)
	/**
	Symbian OS 2nd phase constructor.

	@param aJavaId the Java MIDlet suite hash
	@param aDriveNumber the drive that contains the data
	@param aTransferDataType the type of the data you wish to transfer
	*/
		{
		OstTraceFunctionEntry0( CSBJAVATRANSFERTYPE_CONSTRUCTL_ENTRY );
		TInt offset = 0;

		BaseConstructL();

		InitialiseL(offset, aSuiteHash, aDriveNumber, aTransferDataType);
		OstTraceFunctionExit0( CSBJAVATRANSFERTYPE_CONSTRUCTL_EXIT );
		}

	void CSBJavaTransferType::ConstructL(CSBGenericTransferType* aGenericTransferType)
	/**
	Symbian 2nd phase constructor. Validates the externalised type, then passes it to the base ConstructL

	@param aGenericTransferType pointer to a CSBGenericTransferType object
	*/
		{
		OstTraceFunctionEntry0( DUP1_CSBJAVATRANSFERTYPE_CONSTRUCTL_ENTRY );
		if (aGenericTransferType == NULL)
			{
		    OstTrace0(TRACE_ERROR, CSBJAVATRANSFERTYPE_CONSTRUCTL, "Leave: KErrArgument");
			User::Leave(KErrArgument);
			}
		if (EJavaTransferDerivedType != aGenericTransferType->DerivedTypeL())
			{
		    OstTrace0(TRACE_ERROR, DUP1_CSBJAVATRANSFERTYPE_CONSTRUCTL, "Leave: KErrArgument");
			User::Leave(KErrArgument);
			}
		// Call the base class ConstructL
		CSBGenericDataType::ConstructL(aGenericTransferType->Externalise());
		OstTraceFunctionExit0( DUP1_CSBJAVATRANSFERTYPE_CONSTRUCTL_EXIT );
		}
		
	EXPORT_C const TDesC& CSBJavaTransferType::SuiteHashL() const
	/**
	Getter for the secure identifier
		
	@return  The Java MIDlet suite hash
	*/
		{
		return iSuiteHash;
		}

	EXPORT_C TJavaTransferType CSBJavaTransferType::DataTypeL() const
	/**
	Getter for the transfer data type
		
	@return  The transfer data type
	*/
		{
		OstTraceFunctionEntry0( CSBJAVATRANSFERTYPE_DATATYPEL_ENTRY );
		TJavaTransferType transType;
		
		UnpackType(transType, *iDataBuffer, iTransferDataTypeOffset);

		OstTraceFunctionExit0( CSBJAVATRANSFERTYPE_DATATYPEL_EXIT );
		return transType;
		}
		
		
		
	EXPORT_C CSBEFileEntry* CSBEFileEntry::NewLC(const TEntry& aEntry, RApaLsSession& aSession)
	/** Constructor for creating a CSBEFileEntry from an existing TEntry (as returned
	from RFs' directory listing mechanisms)
	
	@param aEntry Information about the file that this object will represent
	@param aSession Required to map from TEntry's UID MIME type into a textual representation
	@return Instance of CSBEFileEntry created from data supplied in aEntry
	*/
		{
		OstTraceFunctionEntry0( CSBEFILEENTRY_NEWLC_ENTRY );
		CSBEFileEntry* self = new (ELeave) CSBEFileEntry(aEntry);
		CleanupStack::PushL(self);
		self->ConstructL(aEntry, aSession);
		OstTraceFunctionExit0( CSBEFILEENTRY_NEWLC_EXIT );
		return self;
		}
		
	EXPORT_C CSBEFileEntry* CSBEFileEntry::NewLC(const TDesC8& aStream, TInt& aBytesRead)
	/** Constructor designed to internalise an instance of CSBEFileEntry from a previously 
	ExternaliseL'd version.
	@param aStream The descriptor containing the internalised version of this object
	@param aBytesRead Upon return from this function, this contains the number of bytes read 
						from the descriptor for use in streaming multiple objects from one 
						stream
	@return Instance of CSBEFileEntry containing the information previously packed into aStream
	*/
		{
		OstTraceFunctionEntry0( DUP1_CSBEFILEENTRY_NEWLC_ENTRY );
		CSBEFileEntry* self = new (ELeave) CSBEFileEntry;
		CleanupStack::PushL(self);
		self->InternaliseL(aStream, aBytesRead);
		OstTraceFunctionExit0( DUP1_CSBEFILEENTRY_NEWLC_EXIT );
		return self;
		}
		
	CSBEFileEntry::CSBEFileEntry()
	/** C++ ctor */
		{
		OstTraceFunctionEntry0( CSBEFILEENTRY_CSBEFILEENTRY_CONS_ENTRY );
		OstTraceFunctionExit0( CSBEFILEENTRY_CSBEFILEENTRY_CONS_EXIT );
		}

	CSBEFileEntry::CSBEFileEntry(const TEntry& aEntry)
	/** C++ ctor that copies out the parts of aEntry that don't require heap allocation 
	@param aEntry The entry to copy most of the settings from
	*/
		: iAtt(aEntry.iAtt), iSize(aEntry.iSize), iModified(aEntry.iModified), iUidType(aEntry.iType)
		{
		OstTraceFunctionEntry0( DUP1_CSBEFILEENTRY_CSBEFILEENTRY_CONS_ENTRY );
		OstTraceFunctionExit0( DUP1_CSBEFILEENTRY_CSBEFILEENTRY_CONS_EXIT );
		}
		
	EXPORT_C CSBEFileEntry::~CSBEFileEntry()
	/** C++ dtor */
		{
		OstTraceFunctionEntry0( CSBEFILEENTRY_CSBEFILEENTRY_DES_ENTRY );
		delete iType;
		iType = NULL;
		delete iFilename;
		iFilename = NULL;
		OstTraceFunctionExit0( CSBEFILEENTRY_CSBEFILEENTRY_DES_EXIT );
		}
		
	void CSBEFileEntry::ConstructL(const TEntry& aEntry, RApaLsSession& aSession)
	/** Symbian 2nd phase constructor used when instantiating an instance from component parts
	rather than from a previously externalised stream
	@param aEntry Information about the file that this object will represent
	@param aSession Required to map from TEntry's UID MIME type into a textual representation
	*/
		{
		OstTraceFunctionEntry0( CSBEFILEENTRY_CONSTRUCTL_ENTRY );
		// Look up the textual mime type of the file instead of the numerical one given by TEntry
		TUid uidForType;
		TDataType appDataType;
		aSession.AppForDocument(aEntry.iName,uidForType,appDataType);
		iType = appDataType.Des().AllocL();
		iFilename = aEntry.iName.AllocL();
		OstTraceFunctionExit0( CSBEFILEENTRY_CONSTRUCTL_EXIT );
		}
		
	void CSBEFileEntry::InternaliseL(const TDesC8& aStream, TInt& aBytesRead)
	/** Symbian 2nd phase constructor used when instantiating an instance from a previously 
	externalised stream of bytes. aStream must begin at the beginning of the internalised
	form of this object and upon return, aBytesRead will contain the number of bytes read from
	that stream which allows multiple objects to be packed into a stream and read sequentially
	@param aStream The stream containing at least one instance of CSBEFileEntry
	@param aBytesRead Upon return, this will contain the number of bytes read from aStream
						in order to internalise this single instance of CSBEFileEntry
	*/
		{
		OstTraceFunctionEntry0( CSBEFILEENTRY_INTERNALISEL_ENTRY );
		RDesReadStream stream(aStream);
		CleanupClosePushL(stream);
		
		// Unpack the trivial types
		iAtt = stream.ReadUint32L();
		iSize = stream.ReadInt32L();
		iModified = MAKE_TINT64(stream.ReadUint32L(), stream.ReadUint32L());
		
		// TUidType is a C style array
		iUidType = TUidType(TUid::Uid(stream.ReadUint32L()),
							TUid::Uid(stream.ReadUint32L()),
							TUid::Uid(stream.ReadUint32L()));

		// Unpack the strings
		delete iType;
		iType = NULL;
		delete iFilename;
		iFilename = NULL;
		
		TInt typeLength = stream.ReadUint16L();
		iType = HBufC::NewL(typeLength);
		TPtr typeBuf(iType->Des());
		stream.ReadL(typeBuf, typeLength);
		
		TInt filenameLength = stream.ReadUint16L();
		iFilename = HBufC::NewL(filenameLength);
		TPtr filenameBuf(iFilename->Des());
		stream.ReadL(filenameBuf, filenameLength);
		
		aBytesRead = CalculatePackedSize();
		CleanupStack::PopAndDestroy(&stream);
		OstTraceFunctionExit0( CSBEFILEENTRY_INTERNALISEL_EXIT );
		}
		
	TUint16 CSBEFileEntry::CalculatePackedSize() const
	/** calculate the size of this object when externalised
	@return the size in bytes of the externalised representation of this object
	*/
		{
		OstTraceFunctionEntry0( CSBEFILEENTRY_CALCULATEPACKEDSIZE_ENTRY );
		TInt total = 
			sizeof(TUint32) +		// Length of attribute field
			sizeof(TInt32) +		// Length of file size
			8 +	// Length of last modified time
			sizeof(TUidType) +	// Length of MIME type uids
			sizeof(TUint16) + 	// Text MIME type length
			iType->Size() +		// Text MIME type data
			sizeof(TUint16) + 	// Filename length
			iFilename->Size();	// Filename data
		OstTraceFunctionExit0( CSBEFILEENTRY_CALCULATEPACKEDSIZE_EXIT );
		return total;
		}

	EXPORT_C HBufC8* CSBEFileEntry::ExternaliseLC() const
	/** Return a pointer to a buffer descriptor containing an externalised representation of
	the data stored by this object. The returned stream is suitable for unpacking an identical 
	instance of this object by using the NewLC that accepts a descriptor as an argument
	@return An externalised instance of this object suitable for Internalising
	*/
		{
		OstTraceFunctionEntry0( CSBEFILEENTRY_EXTERNALISELC_ENTRY );
		TUint16 packedSize = CalculatePackedSize();
		HBufC8* pBuf = HBufC8::NewLC(packedSize);
		TPtr8 buf(pBuf->Des());
		RDesWriteStream stream(buf);
		CleanupClosePushL(stream);
		
		stream.WriteUint32L(iAtt);
		stream.WriteInt32L(iSize);
		stream.WriteUint32L(I64HIGH(iModified.Int64()));
		stream.WriteUint32L(I64LOW(iModified.Int64()));
		stream.WriteUint32L(iUidType[0].iUid);
		stream.WriteUint32L(iUidType[1].iUid);
		stream.WriteUint32L(iUidType[2].iUid);
		stream.WriteUint16L(iType->Length());
		stream.WriteL(*iType);
		stream.WriteUint16L(iFilename->Length());
		stream.WriteL(*iFilename);
		
		CleanupStack::PopAndDestroy(&stream);
		OstTraceFunctionExit0( CSBEFILEENTRY_EXTERNALISELC_EXIT );
		return pBuf;
		}
		
	EXPORT_C TUint CSBEFileEntry::FileAttributes() const
	/** Accessor function returning the packed bitmask representing the attributes
	of the file that this object represents in the format defined by TEntry
	@return Packed bits representing the file attributes of the file represented by this object
	@see TEntry::iAtt
	*/
		{
		return iAtt;
		}
		
	EXPORT_C TInt CSBEFileEntry::FileSize() const
	/** Accessor function returning the size in bytes of the file represented by this 
	object
	@return Size in bytes of the file represented by this object
	@see TEntry::iSize
	*/
		{
		return iSize;
		}
		
	EXPORT_C TTime CSBEFileEntry::LastModified() const
	/** Accessor function returning the time that the file represented by this object#
	was last modified
	@return Time of last modification of this file
	@see TEntry::iModified
	*/
		{
		return iModified;
		}
		
	EXPORT_C TDesC16& CSBEFileEntry::Filename() const
	/** Accessor function returning the path an name of the file represented by this object.
	@return Filename including path of the file represented by this object
	@see TEntry::iName
	*/
		{
		return *iFilename;
		}
		
	EXPORT_C TDesC16& CSBEFileEntry::MIMEType() const
	/** Accessor function returning a textual representation of the MIME type
	of the file represented by this object.
	@return textual representation of the MIME type	of the file represented by this object
	*/
		{
		return *iType;
		}
		
	EXPORT_C TUidType& CSBEFileEntry::MIMEUid()
	/** Accessor function returning the UID of the MIME type of the file represented 
	by this object.
	@return textual representation of the MIME type	of the file represented by this object
	@see TEntry::iType
	*/
		{
		return iUidType;
		}
		
	}
