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
#include <mtp/cmtptypedeleteserviceproplist.h>

EXPORT_C CMTPTypeDeleteServicePropList* CMTPTypeDeleteServicePropList::NewL()
    {
    CMTPTypeDeleteServicePropList* self = CMTPTypeDeleteServicePropList::NewLC(); 
    CleanupStack::Pop(self);
    return self;  
    }

EXPORT_C CMTPTypeDeleteServicePropList* CMTPTypeDeleteServicePropList::NewLC()
    {
    CMTPTypeDeleteServicePropList* self = new(ELeave) CMTPTypeDeleteServicePropList();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

EXPORT_C CMTPTypeDeleteServicePropList::~CMTPTypeDeleteServicePropList()
    {
    
    }

EXPORT_C void CMTPTypeDeleteServicePropList::AppendL(const TMTPTypeDataPair* aElement)
    {
    CMTPTypeList::AppendL( aElement );
    }


EXPORT_C TMTPTypeDataPair& CMTPTypeDeleteServicePropList::ElementL(const TInt aIndex) const
    {
    return static_cast<TMTPTypeDataPair&>( CMTPTypeList::ElementL( aIndex ) );
    }

CMTPTypeDeleteServicePropList::CMTPTypeDeleteServicePropList():
    CMTPTypeList( EMTPTypeDeleteServicePropListDataset , EMTPTypeDataPair )
    {
    
    }

void CMTPTypeDeleteServicePropList::ConstructL()
    {
    InitListL();
    }
