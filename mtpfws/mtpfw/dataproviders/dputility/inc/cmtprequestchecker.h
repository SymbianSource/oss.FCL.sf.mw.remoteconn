// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// cmtpequestchecker.h
// 
//

/**
 @file
 @internalTechnology
*/

#ifndef __CMTPREQUESTCHECKER_H__
#define __CMTPREQUESTCHECKER_H__

#include <e32base.h>
#include <mtp/mtpprotocolconstants.h>
#include <mtp/tmtptyperequest.h>
#include "rmtpdpsingletons.h"

class CMTPObjectMetaData;
class MMTPDataProviderFramework;
class MMTPConnection;

/**
element type in the request
@internalTechnology
*/        
enum TMTPElementType
    {
    /**
    Session ID type
    */
    EMTPElementTypeSessionID,			
    
    /**
    Object Handle type
    */    
    EMTPElementTypeObjectHandle,		
    
    /**
    Format code type	
    */    
    EMTPElementTypeFormatCode,			
    
    /**
    Storeage Id type
    */    
    EMTPElementTypeStorageId,			
    }; 

/**
element attribute to check in the reqeust
@internalTechnology
*/
enum TMTPElementAttr
	{
	/**
	no attribute to check
	*/
	EMTPElementAttrNone		=  	0x0000,		
	
	/**
	the object/storage should be readable
	*/
	EMTPElementAttrRead		=	0x0001,	
	
	/**
	the object/storage should be writable
	*/		
	EMTPElementAttrWrite	=	0x0002,		
	
	/**
	the object should be a file
	*/
	EMTPElementAttrFile 	= 	0x0004,		
	
	/**
	the object should be a directory
	*/
	EMTPElementAttrDir		=	0x0008,	
	
	/**
	supported format array
	*/	
	EMTPElementAttrFormatEnums = 0x0010,
	
	/**
	The object can be either a file or directory.
	*/
	EMTPElementAttrFileOrDir = 0x0020,
	};

/**
defines the verification data structure
@internalTechnology
*/
struct TMTPRequestElementInfo
    {
    TMTPTypeRequest::TElements  iElementIndex;		//which element in the request to check
    TMTPElementType 		    iElementType;		//the type of the element
    TInt 				        iElementAttr;		//the attribute of the element
    TInt 					    iCount;				//number of special values (0, 1, 2)
    TUint32 				    iValue1;			//special value 1
    TUint32 					iValue2;			//special value 2
    }; 

/**
defines a generic request verification class.  It iterates through the verification data elements,
and performs the check based on the element type/attribute.
@internalTechnology
*/    
class CMTPRequestChecker : public CBase 
    {
public:
	IMPORT_C static CMTPRequestChecker* NewL(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);
	IMPORT_C ~CMTPRequestChecker();
      
    IMPORT_C TMTPResponseCode VerifyRequestL(const TMTPTypeRequest& aRequest, TInt aCount, const TMTPRequestElementInfo* aElementInfo);
	IMPORT_C CMTPObjectMetaData* GetObjectInfo(TUint32 aHandle) const; 

private:	    
    TMTPResponseCode CheckRequestHeader(const TMTPTypeRequest& aRequest) const;
    TMTPResponseCode VerifySessionId(TUint32 aSessionId, const TMTPRequestElementInfo& aElementInfo) const;    
    TMTPResponseCode VerifyObjectHandleL(TUint32 aHandle, const TMTPRequestElementInfo& aElementInfo);
    TMTPResponseCode VerifyStorageIdL(TUint32 aStorageId, const TMTPRequestElementInfo& aElementInfo) const;  
    TMTPResponseCode VerifyFormatCode(TUint32 aFormatCode, const TMTPRequestElementInfo& aElementInfo) const;
	TBool IsSpecialValue(TUint32 aParameter, const TMTPRequestElementInfo& aElementInfo) const;
private:
	CMTPRequestChecker(MMTPDataProviderFramework& aFramework, MMTPConnection& aConnection);	
	void ConstructL();
	
private:
    MMTPDataProviderFramework&		    iFramework;
    MMTPConnection& 				    iConnection;
	RArray<TUint32>					    iHandles;			//these two arrays contain the mapping from the handle to objectinfo 
	RPointerArray<CMTPObjectMetaData>   iObjectArray;
	RMTPDpSingletons										iDpSingletons;
    };
	
#endif 

