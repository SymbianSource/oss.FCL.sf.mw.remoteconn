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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpplaybackparamTraces.h"
#endif



/*********************************************
    class TMTPPbDataSuid
**********************************************/
TMTPPbCategory TMTPPbDataSuid::Category() const
    {
    OstTraceFunctionEntry0( TMTPPBDATASUID_CATEGORY_ENTRY );
    OstTraceFunctionExit0( TMTPPBDATASUID_CATEGORY_EXIT );
    return iPlayCategory;
    }

const TDesC& TMTPPbDataSuid::Suid() const
    {
    OstTraceFunctionEntry0( TMTPPBDATASUID_SUID_ENTRY );
    OstTraceFunctionExit0( TMTPPBDATASUID_SUID_EXIT );
    return iSuid;
    }

TMTPPbDataSuid::TMTPPbDataSuid(TMTPPbCategory aCategory, const TDesC& aSuid):
    iPlayCategory(aCategory),
    iSuid(aSuid)
    {
    OstTraceFunctionEntry0( TMTPPBDATASUID_TMTPPBDATASUID_ENTRY );
    
    OstTraceFunctionExit0( TMTPPBDATASUID_TMTPPBDATASUID_EXIT );
    }

/*********************************************
    class CMTPPbParamBase
**********************************************/

CMTPPbParamBase::~CMTPPbParamBase()
    {
    OstTraceFunctionEntry0( CMTPPBPARAMBASE_CMTPPBPARAMBASE_ENTRY );
    delete iData;
    OstTraceFunctionExit0( CMTPPBPARAMBASE_CMTPPBPARAMBASE_EXIT );
    }

CMTPPbParamBase::CMTPPbParamBase():
    iParamType(EMTPPbTypeNone)
    {
OstTraceFunctionEntry0( DUP1_CMTPPBPARAMBASE_CMTPPBPARAMBASE_ENTRY );

    OstTraceFunctionExit0( DUP1_CMTPPBPARAMBASE_CMTPPBPARAMBASE_EXIT );
    }

CMTPPbParamBase::CMTPPbParamBase(TMTPPbCategory /*aCategory*/, const TDesC& /*aSuid*/):
    iParamType(EMTPPbSuidSet)
    {
OstTraceFunctionEntry0( DUP2_CMTPPBPARAMBASE_CMTPPBPARAMBASE_ENTRY );

    OstTraceFunctionExit0( DUP2_CMTPPBPARAMBASE_CMTPPBPARAMBASE_EXIT );
    }

CMTPPbParamBase::CMTPPbParamBase(TInt32 /*aValue*/):
    iParamType(EMTPPbInt32)
    {
    OstTraceFunctionEntry0( DUP3_CMTPPBPARAMBASE_CMTPPBPARAMBASE_ENTRY );
    
    OstTraceFunctionExit0( DUP3_CMTPPBPARAMBASE_CMTPPBPARAMBASE_EXIT );
    }

CMTPPbParamBase::CMTPPbParamBase(TUint32 /*aValue*/):
    iParamType(EMTPPbUint32)
    {
    OstTraceFunctionEntry0( DUP4_CMTPPBPARAMBASE_CMTPPBPARAMBASE_ENTRY );
    
    OstTraceFunctionExit0( DUP4_CMTPPBPARAMBASE_CMTPPBPARAMBASE_EXIT );
    }

void CMTPPbParamBase::ConstructL(TMTPPbCategory aCategory, const TDesC& aSuid)
    {
    OstTraceFunctionEntry0( CMTPPBPARAMBASE_CONSTRUCTL_ENTRY );
    TMTPPbDataSuid* val = new (ELeave) TMTPPbDataSuid(aCategory, aSuid);
    iData = static_cast<TAny*>(val);
    OstTraceFunctionExit0( CMTPPBPARAMBASE_CONSTRUCTL_EXIT );
    }

void CMTPPbParamBase::ConstructL(TInt32 aValue)
    {
    OstTraceFunctionEntry0( DUP1_CMTPPBPARAMBASE_CONSTRUCTL_ENTRY );
    TInt32* val = new (ELeave) TInt32();
    *val = aValue;
    iData = static_cast<TAny*>(val);
    OstTraceFunctionExit0( DUP1_CMTPPBPARAMBASE_CONSTRUCTL_EXIT );
    }

void CMTPPbParamBase::ConstructL(TUint32 aValue)
    {
    OstTraceFunctionEntry0( DUP2_CMTPPBPARAMBASE_CONSTRUCTL_ENTRY );
    TUint32* val = new (ELeave) TUint32();
    *val = aValue;
    iData = static_cast<TAny*>(val);
    OstTraceFunctionExit0( DUP2_CMTPPBPARAMBASE_CONSTRUCTL_EXIT );
    }

void CMTPPbParamBase::ConstructL(const CMTPPbParamBase& aParam)
    {
    OstTraceFunctionEntry0( DUP3_CMTPPBPARAMBASE_CONSTRUCTL_ENTRY );
    TMTPPbDataType type(aParam.Type());
    __ASSERT_DEBUG((type > EMTPPbTypeNone && type < EMTPPbTypeEnd), Panic(EMTPPBArgumentErr));
    __ASSERT_ALWAYS_OST((type > EMTPPbTypeNone && type < EMTPPbTypeEnd), OstTrace0( TRACE_ERROR, CMTPPBPARAMBASE_CONSTRUCTL, "Error argument" ), User::Leave(KErrArgument));
    
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
            LEAVEIFERROR(KErrArgument, 
                    OstTrace0( TRACE_ERROR, DUP1_CMTPPBPARAMBASE_CONSTRUCTL, "Error argument" ));
            
            break;
        }

    iParamType = type;
    OstTraceFunctionExit0( DUP3_CMTPPBPARAMBASE_CONSTRUCTL_EXIT );
    }

TMTPPbDataType CMTPPbParamBase::Type() const
    {
    OstTraceFunctionEntry0( CMTPPBPARAMBASE_TYPE_ENTRY );
    __ASSERT_DEBUG((iParamType > EMTPPbTypeNone && iParamType < EMTPPbTypeEnd), 
                    Panic(EMTPPBDataTypeErr));
    OstTraceFunctionExit0( CMTPPBPARAMBASE_TYPE_EXIT );
    return iParamType;
    }

void CMTPPbParamBase::SetType(TMTPPbDataType aType)
    {
    OstTraceFunctionEntry0( CMTPPBPARAMBASE_SETTYPE_ENTRY );
    __ASSERT_DEBUG((iParamType == EMTPPbTypeNone), Panic(EMTPPBDataTypeErr));
    __ASSERT_DEBUG((aType > EMTPPbTypeNone && aType < EMTPPbTypeEnd), Panic(EMTPPBDataTypeErr));
    iParamType = aType;
    OstTraceFunctionExit0( CMTPPBPARAMBASE_SETTYPE_EXIT );
    }

TAny* CMTPPbParamBase::GetData() const
    {
    OstTraceFunctionEntry0( CMTPPBPARAMBASE_GETDATA_ENTRY );
    __ASSERT_DEBUG((iData != NULL), Panic(EMTPPBDataNullErr));
    OstTraceFunctionExit0( CMTPPBPARAMBASE_GETDATA_EXIT );
    return iData;
    }

void CMTPPbParamBase::SetData(TAny* aData)
    {
    OstTraceFunctionEntry0( CMTPPBPARAMBASE_SETDATA_ENTRY );
    __ASSERT_DEBUG((aData != NULL), Panic(EMTPPBDataNullErr));
    iData = aData;
    OstTraceFunctionExit0( CMTPPBPARAMBASE_SETDATA_EXIT );
    }

const TMTPPbDataSuid& CMTPPbParamBase::SuidSetL() const
    {
    OstTraceFunctionEntry0( CMTPPBPARAMBASE_SUIDSETL_ENTRY );
    __ASSERT_DEBUG((iParamType == EMTPPbSuidSet), Panic(EMTPPBDataTypeErr));
    __ASSERT_ALWAYS_OST((iParamType == EMTPPbSuidSet), OstTrace0( TRACE_ERROR, CMTPPBPARAMBASE_SUIDSETL, "Error argument" ), User::Leave(KErrArgument));

    return *static_cast<TMTPPbDataSuid*>(iData);
    }

TInt32 CMTPPbParamBase::Int32L() const
    {
    OstTraceFunctionEntry0( CMTPPBPARAMBASE_INT32L_ENTRY );
    __ASSERT_DEBUG((iParamType == EMTPPbInt32), Panic(EMTPPBDataTypeErr));
    __ASSERT_ALWAYS_OST((iParamType == EMTPPbInt32), OstTrace0( TRACE_ERROR, CMTPPBPARAMBASE_INT32L, "Error argument" ), User::Leave(KErrArgument));

    return *static_cast<TInt32*>(iData);
    }

TUint32 CMTPPbParamBase::Uint32L() const
    {
    OstTraceFunctionEntry0( CMTPPBPARAMBASE_UINT32L_ENTRY );
    __ASSERT_DEBUG((iParamType == EMTPPbUint32), Panic(EMTPPBDataTypeErr));
    __ASSERT_ALWAYS_OST((iParamType == EMTPPbUint32), OstTrace0( TRACE_ERROR, CMTPPBPARAMBASE_UINT32L, "Error argument" ), User::Leave(KErrArgument));

    return *static_cast<TUint32*>(iData);
    }
