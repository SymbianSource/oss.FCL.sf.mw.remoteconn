/*
* Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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
* JavaManagerInterface plugin interface.
* Constructor.
* 
*
*/



/**
 @file
 @publishedPartner
 @released
*/
inline CJavaManagerInterface::CJavaManagerInterface()

	{		
	}
 
/** 
 * Destructor.
 * 
 */ 
inline CJavaManagerInterface::~CJavaManagerInterface()
	{
	REComSession::DestroyedImplementation(iDtor_ID_Key);
	}
    
/** 
 * static function to create and return the object.
 * 
 * @return CJavaManagerInterface* Pointer to newly instantiated object.
 */
CJavaManagerInterface* CJavaManagerInterface::NewL( const TUid& aImplementationUid )
	{
	
	RImplInfoPtrArray implementationsArray;
	CleanupResetAndDestroyPushL(implementationsArray);

	
	REComSession::ListImplementationsL(KJavaManagerInterfaceUid,implementationsArray );
	
	TInt isImplementationFound = 0;
	
	for ( TInt a=0; a<implementationsArray.Count(); a++ )
		{
		if ( aImplementationUid ==( implementationsArray.operator[](a)->ImplementationUid() ) )
			{
			isImplementationFound = 1; 
			break;
			}
		}
	
	if ( implementationsArray.Count() <= 0 || isImplementationFound == 0 )
		{
		CleanupStack::PopAndDestroy(&implementationsArray);
		return NULL;
		}	
	
	TAny* javaManagerInterface = REComSession::CreateImplementationL( implementationsArray.operator[](0)->ImplementationUid(), 
								_FOFF(CJavaManagerInterface,iDtor_ID_Key) );   
								
	CleanupStack::PopAndDestroy(&implementationsArray);								

	return reinterpret_cast<CJavaManagerInterface*>(javaManagerInterface);    		
	}
			
		
		
// End of file
