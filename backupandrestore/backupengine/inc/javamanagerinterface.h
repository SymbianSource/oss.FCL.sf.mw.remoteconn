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
* Declaration of CJavaManagerInterface
* 
*
*/



/**
 @file
*/
#ifndef __JAVAMANAGERINTERFACE_H__
#define __JAVAMANAGERINTERFACE_H__

// System
#include <e32base.h>
#include <ecom/ecom.h>
#include <apmrec.h>

#include <connect/sbtypes.h>

/*
@KJavaManagerInterfaceUid, Ther JavaMangerInterface Uid.
This UID is used by the implementer of the ThirdPartyJavaManager plugin. 
This plugin is used in Backup and restore engine.
*/
const TUid KJavaManagerInterfaceUid = { 0x2000D924 };

namespace conn
	{
	
	class CDataOwnerInfo;
	class CSBGenericTransferType;
	class CSBGenericDataType;
/**
Clients creating a java backup and restore plugin should derive from and implement this interface.
@publishedPartner
@released
*/	
	class CJavaManagerInterface : public CBase
		{	
		
		public:
	
		/** 
		  static function to create and return the object.
		  
		  @return CJavaManagerInterface* Pointer to newly instantiated object.
		 */
			inline static CJavaManagerInterface* NewL( const TUid& aImplementationUid );
			    
		/** 
		  Destructor.
		  
		 */ 
			inline virtual ~CJavaManagerInterface();	    	
			
			
		public:	
		
		/**
		Method requesting information about all of the Java data owners that are represented by this object
		@param aDataOwners Array of data owner info's to populate
		*/			
		   	virtual void GetDataOwnersL(RPointerArray<CDataOwnerInfo>& aDataOwners)=0;
		/**
		Return the expected size of data to be returned to the PC client. Used for displaying progress information
		@param aGenericTransferType The type of data that the expected size is being requested for
		@param aSize Stores the expected size value to be returned to the PC side client
		*/	
		    virtual void GetExpectedDataSizeL(CSBGenericTransferType* aGenericTransferType, TUint& aSize)=0;

		/**
		Get a list of public files that the java data owners are requesting to be backed up in addition to their 
		own private data stores
		@param aDriveNumber The drive that these public files are being requested for
		@param aFiles An array of filenames that will be populated upon return of this method
		*/		    
		    virtual void GetPublicFileListL(CSBGenericDataType* aGenericDataType, TDriveNumber aDriveNumber, RFileArray& aFiles)=0;

		/**
		Get a list of public files that the java data owners are requesting to be backed up in addition to their 
		own private data stores
		@param aDriveNumber The drive that these public files are being requested for
		@param aFiles An array of filenames that will be populated upon return of this method
		*/		    
		    virtual void GetRawPublicFileListL(CSBGenericDataType* aGenericDataType, TDriveNumber aDriveNumber, RRestoreFileFilterArray& aFileFilter)=0;

		/**
		Supplies data to the particular data owners to process
		@param 	aGenericTransferType The type of data that's being supplied
		@param 	aBuffer Buffer containing the data to be supplied
		@param 	aFinished Flag determining whether or not this buffer is the last in a multipart sequence
		*/		    
		    virtual void SupplyDataL(CSBGenericTransferType* aGenericTransferType, TDesC8& aBuffer, TBool aFinished)=0;

		/**
		Request a type of data from a particular data owner
		@param 	aGenericTransferType The type of data being requested
		@param 	aBuffer The buffer to populate with data
		@param 	aFinished Flag to be set depending on whether another request is required to transfer 
					all of this data owner's data
		*/		    
		    virtual void RequestDataL(CSBGenericTransferType* aGenericTransferType, TPtr8& aBuffer, TBool& aFinished)=0;

		/**
		Called by the PC client once all registration and binary files have been restored. Used as a signal to 
		indicate that the next set of transfers will include data
		*/		    
			virtual void AllSystemFilesRestored()=0;
		/**
		Method to check the data type of the file
		@param TDesC& aFileName File name of the file to recognize
		@return TDataRecognitionResult Data Recognized Result
		*/
			virtual TDataRecognitionResult RecognizeDataTypeL(const TDesC& aFileName)=0;
		/**
		Returns the handle to File Server.
		*/	

			virtual RFs& GetRFs()=0;

		/** Returns a pointer to a specified interface extension - to allow future extension
		of this class without breaking binary compatibility

		@param aInterfaceUid Identifier of the interface to be retrieved
		@param aInterface A reference to a pointer that retrieves the specified interface.
		*/	
		virtual void GetExtendedInterface(TUid aInterfaceUid, TAny*& aInterface)=0;
					
		protected:
		/**
		Scan the SystemAMS data cage (private directory) for all registration files corresponding to installed MIDlet's
		@param aRegFileArray Array of registration files that will be populated with all reg files in the SystemAMS data cage
		**/		 
			virtual void ScanForRegistrationFilesL(RFileArray& aRegFileArray)=0;
			
		/**
		Tries Parsing the Java Data Owner at runtime.
		@param aFileName aSuiteHash the suite hash of the java data owner 
		*/			
			virtual void ParseJavaDataOwnerL(const TDesC& aSuiteHash)=0;
			
			
		protected:
		
		/** 
 		* Constructor.
 		*/
		    inline CJavaManagerInterface();		
			
		private:
			TUid iDtor_ID_Key; //ECOM Plugin identifier 
		
		};		

	/**
	 * Template classes to call ResetAndDestroy on array classes
	 * automatically, when ::PopAndDestroy'ing from the cleanup stack.
	 */
	template <class T>
	class CleanupResetAndDestroy
		{
	public:
		inline static void PushL(T& aRef);
	private:
		static void ResetAndDestroy(TAny *aPtr);
		};
	template <class T>
	inline void CleanupResetAndDestroyL(T& aRef);

	// Template class CleanupClose
	template <class T>
	inline void CleanupResetAndDestroy<T>::PushL(T& aRef)
		{CleanupStack::PushL(TCleanupItem(&ResetAndDestroy, &aRef));}
	template <class T>
	void CleanupResetAndDestroy<T>::ResetAndDestroy(TAny *aPtr)
		{static_cast<T*>(aPtr)->ResetAndDestroy();}
	template <class T>
	inline void CleanupResetAndDestroyPushL(T& aRef)
		{CleanupResetAndDestroy<T>::PushL(aRef);}


	#include <connect/javamanagerinterface.inl>
	   

	}// namespace conn
	
	
	
#endif // __JAVAMANAGERINTERFACE_H__
