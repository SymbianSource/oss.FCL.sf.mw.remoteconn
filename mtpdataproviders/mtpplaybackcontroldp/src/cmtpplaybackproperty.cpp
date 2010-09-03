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

#include "cmtpplaybackmap.h"
#include "cmtpplaybackproperty.h"
#include "mtpplaybackcontrolpanic.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmtpplaybackpropertyTraces.h"
#endif


const TInt32 KMTPDefaultPlaybackRate = 0;

const TUint32 KMTPMaxPlaybackVolume = 100;
const TUint32 KMTPMinPlaybackVolume = 0;
const TUint32 KMTPDefaultPlaybackVolume = 40;
const TUint32 KMTPCurrentPlaybackVolume = 40;
const TUint32 KMTPVolumeStep = 1;

const TUint32 KMTPDefaultPlaybackObject = 0;
const TUint32 KMTPDefaultPlaybackIndex = 0;
const TUint32 KMTPDefaultPlaybackPosition = 0;

/**
Two-phase constructor.
@param aPlugin The data provider plugin
@return a pointer to the created playback checker object
*/  
CMTPPlaybackProperty* CMTPPlaybackProperty::NewL()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKPROPERTY_NEWL_ENTRY );
    CMTPPlaybackProperty* self = new (ELeave) CMTPPlaybackProperty();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    OstTraceFunctionExit0( CMTPPLAYBACKPROPERTY_NEWL_EXIT );
    return self;
    }

/**
Destructor.
*/    
CMTPPlaybackProperty::~CMTPPlaybackProperty()
    {    
    OstTraceFunctionEntry0( CMTPPLAYBACKPROPERTY_CMTPPLAYBACKPROPERTY_ENTRY );
    delete iPlaybackVolumeData;
    OstTraceFunctionExit0( CMTPPLAYBACKPROPERTY_CMTPPLAYBACKPROPERTY_EXIT );
    }

/**
Constructor.
*/    
CMTPPlaybackProperty::CMTPPlaybackProperty()
    {    
    OstTraceFunctionEntry0( DUP1_CMTPPLAYBACKPROPERTY_CMTPPLAYBACKPROPERTY_ENTRY );
    OstTraceFunctionExit0( DUP1_CMTPPLAYBACKPROPERTY_CMTPPLAYBACKPROPERTY_EXIT );
    }
    
/**
Second-phase constructor.
*/        
void CMTPPlaybackProperty::ConstructL()
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKPROPERTY_CONSTRUCTL_ENTRY );
    OstTraceFunctionExit0( CMTPPLAYBACKPROPERTY_CONSTRUCTL_EXIT );
    }

void CMTPPlaybackProperty::GetDefaultPropertyValueL(TMTPDevicePropertyCode aProp, TInt32& aValue)
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKPROPERTY_GETDEFAULTPROPERTYVALUEL_ENTRY );
    
    __ASSERT_ALWAYS_OST((aProp == EMTPDevicePropCodePlaybackRate), OstTrace0( TRACE_ERROR, CMTPPLAYBACKPROPERTY_GETDEFAULTPROPERTYVALUEL, "Error argument" ), User::Leave(KErrArgument));

    aValue = KMTPDefaultPlaybackRate;

    OstTraceFunctionExit0( CMTPPLAYBACKPROPERTY_GETDEFAULTPROPERTYVALUEL_EXIT );
    }

void CMTPPlaybackProperty::GetDefaultPropertyValueL(TMTPDevicePropertyCode aProp, TUint32& aValue)
    {
    OstTraceFunctionEntry0( DUP1_CMTPPLAYBACKPROPERTY_GETDEFAULTPROPERTYVALUEL_ENTRY );
    switch(aProp)
        {
    case EMTPDevicePropCodeVolume:
        {
        if(iPlaybackVolumeData != NULL)
            {
            aValue = iPlaybackVolumeData->DefaultVolume();            
            }
        else
            {
            aValue = KMTPDefaultPlaybackVolume;
            }
        }
        break;
        
    case EMTPDevicePropCodePlaybackObject:
        {
        aValue = KMTPDefaultPlaybackObject;
        }
        break;
        
    case EMTPDevicePropCodePlaybackContainerIndex:
        {
        aValue = KMTPDefaultPlaybackIndex;
        }
        break;
        
    case EMTPDevicePropCodePlaybackPosition:
        {
        aValue = KMTPDefaultPlaybackPosition;
        }
        break;
        
    default:
        LEAVEIFERROR(KErrArgument, 
                                OstTrace0( TRACE_ERROR, DUP1_CMTPPLAYBACKPROPERTY_GETDEFAULTPROPERTYVALUEL, "Error argument" ));
        }
    OstTraceFunctionExit0( DUP1_CMTPPLAYBACKPROPERTY_GETDEFAULTPROPERTYVALUEL_EXIT );
    }

void CMTPPlaybackProperty::GetDefaultVolSet(TMTPPbDataVolume& aValue)
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKPROPERTY_GETDEFAULTVOLSET_ENTRY );
    if(iPlaybackVolumeData == NULL)
        {
        aValue.SetVolume(KMTPMaxPlaybackVolume,
                         KMTPMinPlaybackVolume,
                         KMTPDefaultPlaybackVolume,
                         KMTPCurrentPlaybackVolume,
                         KMTPVolumeStep);
        }
    else
        {
        aValue = (*iPlaybackVolumeData);
        }
    OstTraceFunctionExit0( CMTPPLAYBACKPROPERTY_GETDEFAULTVOLSET_EXIT );
    }

void CMTPPlaybackProperty::SetDefaultVolSetL(const TMTPPbDataVolume& aValue)
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKPROPERTY_SETDEFAULTVOLSETL_ENTRY );
    if(iPlaybackVolumeData == NULL)
        {
        iPlaybackVolumeData = new (ELeave) TMTPPbDataVolume(aValue);
        }
    else
        {
        (*iPlaybackVolumeData) = aValue;
        }
    OstTraceFunctionExit0( CMTPPLAYBACKPROPERTY_SETDEFAULTVOLSETL_EXIT );
    }

void CMTPPlaybackProperty::GetDefaultPropertyValueL(TMTPPbCtrlData& aValue)
    {
    OstTraceFunctionEntry0( DUP2_CMTPPLAYBACKPROPERTY_GETDEFAULTPROPERTYVALUEL_ENTRY );
    __ASSERT_DEBUG((aValue.iOptCode == EMTPOpCodeResetDevicePropValue), Panic(EMTPPBArgumentErr));
    
    switch(aValue.iDevPropCode)
        {
    case EMTPDevicePropCodePlaybackRate:
        {
        TInt32 val;
        GetDefaultPropertyValueL(aValue.iDevPropCode, val);
        aValue.iPropValInt32.Set(val);
        }
        break;

    case EMTPDevicePropCodeVolume:
    case EMTPDevicePropCodePlaybackObject:
    case EMTPDevicePropCodePlaybackContainerIndex:
    case EMTPDevicePropCodePlaybackPosition:
        {
        TUint32 val;
        GetDefaultPropertyValueL(aValue.iDevPropCode, val);
        aValue.iPropValUint32.Set(val);
        }
        break;
        
    default:
        LEAVEIFERROR(KErrArgument, 
                                OstTrace0( TRACE_ERROR, DUP2_CMTPPLAYBACKPROPERTY_GETDEFAULTPROPERTYVALUEL, "Error argument" ));
        }
    OstTraceFunctionExit0( DUP2_CMTPPLAYBACKPROPERTY_GETDEFAULTPROPERTYVALUEL_EXIT );
    }

TBool CMTPPlaybackProperty::IsDefaultPropertyValueL(const TMTPPbCtrlData& aValue) const
    {
    OstTraceFunctionEntry0( CMTPPLAYBACKPROPERTY_ISDEFAULTPROPERTYVALUEL_ENTRY );
    
    TInt result(EFalse);

    switch(aValue.iDevPropCode)
        {
    case EMTPDevicePropCodePlaybackRate:
        {
        if(aValue.iPropValInt32.Value() == KMTPDefaultPlaybackRate)
            {
            result = ETrue;
            }
        }
        break;
            
    case EMTPDevicePropCodeVolume:
        {
        if(iPlaybackVolumeData == NULL)
            {
            if(aValue.iPropValUint32.Value() == KMTPDefaultPlaybackVolume)
                {
                result = ETrue;
                }
            }
        else
            {
            if(aValue.iPropValUint32.Value() == iPlaybackVolumeData->DefaultVolume())
                {
                result = ETrue;
                }
            }
        }
        break;

    case EMTPDevicePropCodePlaybackObject:
        {
        if(aValue.iPropValUint32.Value() == KMTPDefaultPlaybackObject)
            {
            result = ETrue;
            }
        }
        break;
        
    case EMTPDevicePropCodePlaybackContainerIndex:
        {
        if(aValue.iPropValUint32.Value() == KMTPDefaultPlaybackIndex)
            {
            result = ETrue;
            }
        }
        break;
        
    case EMTPDevicePropCodePlaybackPosition:
        {
        if(aValue.iPropValUint32.Value() == KMTPDefaultPlaybackPosition)
            {
            result = ETrue;
            }
        }
        break;
        
    default:
        LEAVEIFERROR(KErrArgument, 
                OstTrace0( TRACE_ERROR, CMTPPLAYBACKPROPERTY_ISDEFAULTPROPERTYVALUEL, "error argument" ));
        }

    
    OstTraceFunctionExit0( CMTPPLAYBACKPROPERTY_ISDEFAULTPROPERTYVALUEL_EXIT );
    return result;
    }

