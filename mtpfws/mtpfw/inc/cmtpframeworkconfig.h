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
//

/**
 @file
 @internalTechnology
*/

#ifndef CMTPFRAMEWORKCONFIG_H
#define CMTPFRAMEWORKCONFIG_H

#include <e32base.h>
#include <mtp/mmtpframeworkconfig.h>

class CRepository;

/**
Implements the framework configurability parameter data interface. 

The MTP framework implements a number of configurability parameters using a 
central repository. The initial set of configurability parameters are loaded 
from a compiled (binary) initialisation file. Read only access to the MTP 
framework configurability parameter data is unrestricted. Write access is 
restricted to the MTP framework. 
@internalTechnology
 
*/
class CMTPFrameworkConfig : 
    public CBase,
    public MMTPFrameworkConfig
    {
public:

    static CMTPFrameworkConfig* NewL();
    ~CMTPFrameworkConfig();
		
public:

    IMPORT_C void GetValueL(TParameter aParam, TDes& aValue) const;
    IMPORT_C HBufC* ValueL(TParameter aParam) const;
    IMPORT_C void GetValueL(TParameter aParam, TUint& aValue) const;
    IMPORT_C void GetValueL(TParameter aParam, TBool& aValue) const;
    IMPORT_C void GetValueL(TParameter aParam, RArray<TUint>& aArray) const;
   
private:

	CMTPFrameworkConfig();
	void ConstructL();

private: // Owned

	/**
	The configurability parameter data repository.
	*/
	CRepository* iRepository;
	
	/**
	 To save the value of EDownState 
	 */
	TInt         iAbnormalDownValue;
    };
    
#endif // CMTPFRAMEWORKCONFIG_H
