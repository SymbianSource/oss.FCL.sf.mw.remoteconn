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

#include <barsc.h> 
#include <barsread.h>
#include "ruidmapper.h"
#include <framework.rsg>
_LIT(KFrameworkFilename, "\\resource\\mtp\\framework.rsc");

void RUidMapping::ReadFromResourceFileL()
	{
	RFs 	Fs;
	User::LeaveIfError(Fs.Connect());
	CleanupClosePushL(Fs); 
	const TDriveName driveName(TDriveUnit(EDriveZ).Name());
	HBufC* filename = HBufC::NewLC(KMaxDriveName + 
									KFrameworkFilename().Length() );
	
	TPtr pFileName = filename->Des();	
	pFileName.Append(driveName);
	pFileName.Append(KFrameworkFilename);	
	
	// Reading from resource file  
	RResourceFile resourceFile;
	resourceFile.OpenL(Fs, *filename);
	
	CleanupClosePushL(resourceFile);  
	TResourceReader resourceReader;
	MappingStruct st;
	HBufC8* buffer = resourceFile.AllocReadLC(R_ELEMENTS_INFO); 
	resourceReader.SetBuffer(buffer);
	const TInt numberOfEntries=resourceReader.ReadInt16();
	for(TInt count =0;count<numberOfEntries ; count++)
		{
		st.dpUid=resourceReader.ReadInt32();
			
		TInt entries = resourceReader.ReadInt16();
		for (TInt n = 0 ; n < entries; ++n)
			{
			TUint transportId(resourceReader.ReadInt32());
			st.iTransportUidList.AppendL(transportId);
			}
		InsertToMappingStruct(st);	
		}
	
	CleanupStack::PopAndDestroy(4, &Fs);
	}
	

RUidMapping::RUidMapping() 
	{
	
	}

void RUidMapping::Close()
	{
	iMappingStruct.Close();
	}

void RUidMapping::Open()
	{
	iMappingStruct.Reset();
	}

void RUidMapping::InsertToMappingStruct(MappingStruct& aRef)
	{
	iMappingStruct.Append(aRef);
	}
	
TBool RUidMapping::GetSupportedTransport(const TUint& aDPUid,const TUint& aTransportUid)
	{
	TBool found = ETrue;
	TInt count = iMappingStruct.Count();
	for(TInt i=0 ; i<count ; i++)
		{
		if(iMappingStruct[i].dpUid == aDPUid)
			{
			if(iMappingStruct[i].iTransportUidList.Find(aTransportUid)<0)
				{
				found = EFalse;
				return found;
				}
			
			}
		}
	return found;
	}
	