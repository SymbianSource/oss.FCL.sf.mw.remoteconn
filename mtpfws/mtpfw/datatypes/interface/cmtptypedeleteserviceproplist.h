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
@internalComponent
*/

#ifndef CMTPTYPEDELETESERVICEPROPLIST_H_
#define CMTPTYPEDELETESERVICEPROPLIST_H_


#include <mtp/cmtptypelist.h>

class TMTPTypeDataPair;

class CMTPTypeDeleteServicePropList : public CMTPTypeList
    {
    
public:
    IMPORT_C static CMTPTypeDeleteServicePropList* NewL();
    IMPORT_C static CMTPTypeDeleteServicePropList* NewLC();
    IMPORT_C ~CMTPTypeDeleteServicePropList();
    
  
    
public: //CMTPTypeList
    IMPORT_C  void AppendL(const TMTPTypeDataPair* aElement);
    IMPORT_C  TMTPTypeDataPair& ElementL(const TInt aIndex) const;

private:
    CMTPTypeDeleteServicePropList();
    void ConstructL();
    };


#endif /* CMTPTYPEDELETESERVICEPROPLIST_H_ */
