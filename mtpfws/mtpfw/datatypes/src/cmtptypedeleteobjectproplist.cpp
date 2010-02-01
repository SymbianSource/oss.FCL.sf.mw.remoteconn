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
@publishedPartner
*/
#include <mtp/mtpdatatypeconstants.h>
#include <mtp/tmtptypedatapair.h>
#include <mtp/cmtptypedeleteobjectproplist.h>

EXPORT_C CMTPTypeDeleteObjectPropList* CMTPTypeDeleteObjectPropList::NewL()
    {
    CMTPTypeDeleteObjectPropList* self = CMTPTypeDeleteObjectPropList::NewLC(); 
    CleanupStack::Pop(self);
    return self;  
    }

EXPORT_C CMTPTypeDeleteObjectPropList* CMTPTypeDeleteObjectPropList::NewLC()
    {
    CMTPTypeDeleteObjectPropList* self = new(ELeave) CMTPTypeDeleteObjectPropList();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

EXPORT_C CMTPTypeDeleteObjectPropList::~CMTPTypeDeleteObjectPropList()
    {
    
    }

EXPORT_C void CMTPTypeDeleteObjectPropList::AppendL(const TMTPTypeDataPair* aElement)
    {
    CMTPTypeList::AppendL( aElement );
    }


EXPORT_C TMTPTypeDataPair& CMTPTypeDeleteObjectPropList::ElementL(const TInt aIndex) const
    {
    return static_cast<TMTPTypeDataPair&>( CMTPTypeList::ElementL( aIndex ) );
    }

CMTPTypeDeleteObjectPropList::CMTPTypeDeleteObjectPropList():
    CMTPTypeList( EMTPTypeDeleteObjectPropListDataset , EMTPTypeDataPair )
    {
    
    }

void CMTPTypeDeleteObjectPropList::ConstructL()
    {
    InitListL();
    }
