// Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

#include "cmtpplaybackparam.h"
#include "mtpplaybackcontrolpanic.h"


/*********************************************
    class TMTPPbDataSuid
**********************************************/
TMTPPbCategory TMTPPbDataSuid::Category() const
    {
    return iPlayCategory;
    }

const TDesC& TMTPPbDataSuid::Suid() const
    {
    return iSuid;
    }

TMTPPbDataSuid::TMTPPbDataSuid(TMTPPbCategory aCategory, const TDesC& aSuid):
    iPlayCategory(aCategory),
    iSuid(aSuid)
    {
    
    }

/*********************************************
    class CMTPPbParamBase
**********************************************/

CMTPPbParamBase::~CMTPPbParamBase()
    {
    delete iData;
    }

CMTPPbParamBase::CMTPPbParamBase():
    iParamType(EMTPPbTypeNone)
    {

    }

CMTPPbParamBase::CMTPPbParamBase(TMTPPbCategory /*aCategory*/, const TDesC& /*aSuid*/):
    iParamType(EMTPPbSuidSet)
    {

    }

CMTPPbParamBase::CMTPPbParamBase(TInt32 /*aValue*/):
    iParamType(EMTPPbInt32)
    {
    
    }

CMTPPbParamBase::CMTPPbParamBase(TUint32 /*aValue*/):
    iParamType(EMTPPbUint32)
    {
    
    }

void CMTPPbParamBase::ConstructL(TMTPPbCategory aCategory, const TDesC& aSuid)
    {
    TMTPPbDataSuid* val = new (ELeave) TMTPPbDataSuid(aCategory, aSuid);
    iData = static_cast<TAny*>(val);
    }

void CMTPPbParamBase::ConstructL(TInt32 aValue)
    {
    TInt32* val = new (ELeave) TInt32();
    *val = aValue;
    iData = static_cast<TAny*>(val);
    }

void CMTPPbParamBase::ConstructL(TUint32 aValue)
    {
    TUint32* val = new (ELeave) TUint32();
    *val = aValue;
    iData = static_cast<TAny*>(val);
    }

void CMTPPbParamBase::ConstructL(const CMTPPbParamBase& aParam)
    {
    TMTPPbDataType type(aParam.Type());
    __ASSERT_DEBUG((type > EMTPPbTypeNone && type < EMTPPbTypeEnd), Panic(EMTPPBArgumentErr));
    __ASSERT_ALWAYS((type > EMTPPbTypeNone && type < EMTPPbTypeEnd), User::Leave(KErrArgument));
    
    switch(type)
        {
        case EMTPPbSuidSet:
            {
            ConstructL(aParam.SuidSetL().Category(), aParam.SuidSetL().Suid());
            }
            break;
        case EMTPPbInt32:
            {
            ConstructL(aParam.Int32L());
            }
            break;
        case EMTPPbUint32:
            {
            ConstructL(aParam.Uint32L());
            }
            break;
        default:
            User::Leave(KErrArgument);
            break;
        }

    iParamType = type;
    }

TMTPPbDataType CMTPPbParamBase::Type() const
    {
    __ASSERT_DEBUG((iParamType > EMTPPbTypeNone && iParamType < EMTPPbTypeEnd), 
                    Panic(EMTPPBDataTypeErr));
    return iParamType;
    }

void CMTPPbParamBase::SetType(TMTPPbDataType aType)
    {
    __ASSERT_DEBUG((iParamType == EMTPPbTypeNone), Panic(EMTPPBDataTypeErr));
    __ASSERT_DEBUG((aType > EMTPPbTypeNone && aType < EMTPPbTypeEnd), Panic(EMTPPBDataTypeErr));
    iParamType = aType;
    }

TAny* CMTPPbParamBase::GetData() const
    {
    __ASSERT_DEBUG((iData != NULL), Panic(EMTPPBDataNullErr));
    return iData;
    }

void CMTPPbParamBase::SetData(TAny* aData)
    {
    __ASSERT_DEBUG((aData != NULL), Panic(EMTPPBDataNullErr));
    iData = aData;
    }

const TMTPPbDataSuid& CMTPPbParamBase::SuidSetL() const
    {
    __ASSERT_DEBUG((iParamType == EMTPPbSuidSet), Panic(EMTPPBDataTypeErr));
    __ASSERT_ALWAYS((iParamType == EMTPPbSuidSet), User::Leave(KErrArgument));

    return *static_cast<TMTPPbDataSuid*>(iData);
    }

TInt32 CMTPPbParamBase::Int32L() const
    {
    __ASSERT_DEBUG((iParamType == EMTPPbInt32), Panic(EMTPPBDataTypeErr));
    __ASSERT_ALWAYS((iParamType == EMTPPbInt32), User::Leave(KErrArgument));

    return *static_cast<TInt32*>(iData);
    }

TUint32 CMTPPbParamBase::Uint32L() const
    {
    __ASSERT_DEBUG((iParamType == EMTPPbUint32), Panic(EMTPPBDataTypeErr));
    __ASSERT_ALWAYS((iParamType == EMTPPbUint32), User::Leave(KErrArgument));

    return *static_cast<TUint32*>(iData);
    }
