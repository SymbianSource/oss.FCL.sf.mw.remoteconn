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

#ifndef CMTPTYPEDELETEOBJECTPROPLIST_H_
#define CMTPTYPEDELETEOBJECTPROPLIST_H_

#include <mtp/cmtptypelist.h>

class TMTPTypeDataPair;

class CMTPTypeDeleteObjectPropList : public CMTPTypeList
    {
    
public:
    IMPORT_C static CMTPTypeDeleteObjectPropList* NewL();
    IMPORT_C static CMTPTypeDeleteObjectPropList* NewLC();
    IMPORT_C ~CMTPTypeDeleteObjectPropList();
    
  
    
public: //CMTPTypeList
    IMPORT_C  void AppendL(const TMTPTypeDataPair* aElement);
    IMPORT_C  TMTPTypeDataPair& ElementL(const TInt aIndex) const;

private:
    CMTPTypeDeleteObjectPropList();
    void ConstructL();
    };



#endif /* CMTPTYPEDELETEOBJECTPROPLIST_H_ */
