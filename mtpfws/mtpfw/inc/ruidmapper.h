// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
 @internalComponent
*/
#ifndef RUIDMAPPER_H
#define RUIDMAPPER_H

#include <e32base.h>



/** 
Uitlity class for Getting the transport id from the Resource file corresponding to the Data provider UID.

@internalComponent
 
*/
struct MappingStruct
	{
	TUint dpUid;
	RArray<TUint> iTransportUidList;
	};

class RUidMapping
	{
public:	
	RUidMapping();
	void Open();
	void Close();
	TBool GetSupportedTransport(const TUint& aDPUid,const TUint& aTransportUid);
	void ReadFromResourceFileL();
private:
	void InsertToMappingStruct(MappingStruct& aRef);
private:
	
	RArray<MappingStruct> iMappingStruct;
	 	
	};
	
#endif
