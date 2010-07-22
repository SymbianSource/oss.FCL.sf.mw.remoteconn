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

#include "cmtpplaybackcommand.h"
#include "mtpplaybackcontrolpanic.h"

// Class constants.
__FLOG_STMT(_LIT8(KComponent,"CMtpPbCmd");)

/*********************************************
    class TMTPPbDataVolume
**********************************************/
TMTPPbDataVolume::TMTPPbDataVolume(TUint32 aMax, TUint32 aMin, TUint32 aDefault, TUint32 aCurrent, TUint32 aStep):
    iMaxVolume(aMax),iMinVolume(aMin), iDefaultVolume(aDefault), iCurrentVolume(aCurrent), iStep(aStep)
    {
    __ASSERT_DEBUG((aMin < aMax), Panic(EMTPPBArgumentErr));
    __ASSERT_DEBUG((aMin <= aDefault && aDefault <= aMax), Panic(EMTPPBArgumentErr));
    __ASSERT_DEBUG((aMin <= aCurrent && aCurrent <= aMax), Panic(EMTPPBArgumentErr));
    __ASSERT_DEBUG((aStep <= (aMax-aMin)), Panic(EMTPPBArgumentErr));
    __ASSERT_DEBUG((aStep != 0), Panic(EMTPPBArgumentErr));
    }

TMTPPbDataVolume::TMTPPbDataVolume(const TMTPPbDataVolume& aVol):
    iMaxVolume(aVol.MaxVolume()),
    iMinVolume(aVol.MinVolume()), 
    iDefaultVolume(aVol.DefaultVolume()),
    iCurrentVolume(aVol.CurrentVolume()),
    iStep(aVol.Step())
    {
    
    }

void TMTPPbDataVolume::SetVolume(TUint32 aMax, TUint32 aMin, TUint32 aDefault, TUint32 aCurrent, TUint32 aStep)
    {
    __ASSERT_DEBUG((aMin < aMax), Panic(EMTPPBArgumentErr));
    __ASSERT_DEBUG((aMin <= aDefault && aDefault <= aMax), Panic(EMTPPBArgumentErr));
    __ASSERT_DEBUG((aMin <= aCurrent && aCurrent <= aMax), Panic(EMTPPBArgumentErr));
    __ASSERT_DEBUG((aStep <= (aMax-aMin)), Panic(EMTPPBArgumentErr));
    __ASSERT_DEBUG((aStep != 0), Panic(EMTPPBArgumentErr));
    iMaxVolume  = aMax;
    iMinVolume = aMin;
    iDefaultVolume  = aDefault;
    iCurrentVolume = aCurrent,
    iStep = aStep;
    }

void TMTPPbDataVolume::operator =(const TMTPPbDataVolume& aVol)
    {
    iMaxVolume = aVol.MaxVolume();
    iMinVolume = aVol.MinVolume(); 
    iDefaultVolume = aVol.DefaultVolume(); 
    iCurrentVolume = aVol.CurrentVolume();
    iStep = aVol.Step();
    }

TUint32 TMTPPbDataVolume::MaxVolume() const
    {
    return iMaxVolume;
    }

TUint32 TMTPPbDataVolume::MinVolume() const
    {
    return iMinVolume;
    }

TUint32 TMTPPbDataVolume::DefaultVolume() const
    {
    return iDefaultVolume;
    }

TUint32 TMTPPbDataVolume::CurrentVolume() const
    {
    return iCurrentVolume;
    }

TUint32 TMTPPbDataVolume::Step() const
    {
    return iStep;
    }

/*********************************************
    class CMTPPbCmdParam
**********************************************/

CMTPPbCmdParam* CMTPPbCmdParam::NewL(TMTPPbCategory aCategory, const TDesC& aSuid)
    {
    CMTPPbCmdParam* self = new (ELeave) CMTPPbCmdParam(aCategory, aSuid);
    CleanupStack::PushL(self);
    self->ConstructL(aCategory, aSuid);
    CleanupStack::Pop(self);
    return self;
    }

CMTPPbCmdParam* CMTPPbCmdParam::NewL(TInt32 aValue)
    {
    CMTPPbCmdParam* self = new (ELeave) CMTPPbCmdParam(aValue);
    CleanupStack::PushL(self);
    self->ConstructL(aValue);
    CleanupStack::Pop(self);
    return self;
    }

CMTPPbCmdParam* CMTPPbCmdParam::NewL(TUint32 aValue)
    {
    CMTPPbCmdParam* self = new (ELeave) CMTPPbCmdParam(aValue);
    CleanupStack::PushL(self);
    self->ConstructL(aValue);
    CleanupStack::Pop(self);
    return self;
    }

CMTPPbCmdParam* CMTPPbCmdParam::NewL(const TMTPPbDataVolume& aVolume)
    {
    CMTPPbCmdParam* self = new (ELeave) CMTPPbCmdParam(aVolume);
    CleanupStack::PushL(self);
    self->ConstructL(aVolume);
    CleanupStack::Pop(self);
    return self;
    }

CMTPPbCmdParam* CMTPPbCmdParam::NewL(const CMTPPbCmdParam& aParam)
    {
    CMTPPbCmdParam* self = new (ELeave) CMTPPbCmdParam();
    CleanupStack::PushL(self);
    self->ConstructL(aParam);
    CleanupStack::Pop(self);
    return self;
    }

CMTPPbCmdParam::~CMTPPbCmdParam()
    {
    
    }

CMTPPbCmdParam::CMTPPbCmdParam():
    CMTPPbParamBase()
    {

    }

CMTPPbCmdParam::CMTPPbCmdParam(TMTPPbCategory aCategory, const TDesC& aSuid):
    CMTPPbParamBase(aCategory, aSuid)
    {

    }

CMTPPbCmdParam::CMTPPbCmdParam(TInt32 aValue):
    CMTPPbParamBase(aValue)
    {
    
    }

CMTPPbCmdParam::CMTPPbCmdParam(TUint32 aValue):
    CMTPPbParamBase(aValue)
    {
    
    }

CMTPPbCmdParam::CMTPPbCmdParam(const TMTPPbDataVolume& /*aVolume*/):
    CMTPPbParamBase()
    {
    CMTPPbParamBase::SetType(EMTPPbVolumeSet);
    }

void CMTPPbCmdParam::ConstructL(TMTPPbCategory aCategory, const TDesC& aSuid)
    {
    CMTPPbParamBase::ConstructL(aCategory, aSuid);
    }

void CMTPPbCmdParam::ConstructL(TInt32 aValue)
    {
    CMTPPbParamBase::ConstructL(aValue);
    }

void CMTPPbCmdParam::ConstructL(TUint32 aValue)
    {
    CMTPPbParamBase::ConstructL(aValue);
    }

void CMTPPbCmdParam::ConstructL(const TMTPPbDataVolume& aVolume)
    {
    TMTPPbDataVolume* val = new (ELeave) TMTPPbDataVolume(aVolume);
    CMTPPbParamBase::SetData(static_cast<TAny*>(val));
    }

void CMTPPbCmdParam::ConstructL(const CMTPPbCmdParam& aParam)
    {
    TMTPPbDataType type(aParam.Type());

    __ASSERT_DEBUG((type > EMTPPbTypeNone && type < EMTPPbTypeEnd), Panic(EMTPPBArgumentErr));
    __ASSERT_ALWAYS((type > EMTPPbTypeNone && type < EMTPPbTypeEnd), User::Leave(KErrArgument));

    if(type == EMTPPbVolumeSet)
        {
        TMTPPbDataVolume* val = new (ELeave) TMTPPbDataVolume(aParam.VolumeSetL());
        CMTPPbParamBase::SetData(static_cast<TAny*>(val));
        CMTPPbParamBase::SetType(type);
        }
    else
        {
        CMTPPbParamBase::ConstructL(aParam);
        }
    }

const TMTPPbDataVolume& CMTPPbCmdParam::VolumeSetL() const
    {
    __ASSERT_DEBUG((CMTPPbParamBase::Type() == EMTPPbVolumeSet), Panic(EMTPPBDataTypeErr));
    __ASSERT_ALWAYS((CMTPPbParamBase::Type() == EMTPPbVolumeSet), User::Leave(KErrArgument));
    return *static_cast<TMTPPbDataVolume*>(CMTPPbParamBase::GetData());
    }

/*********************************************
    class CMTPPlaybackCommand
**********************************************/
CMTPPlaybackCommand* CMTPPlaybackCommand::NewL(TMTPPlaybackCommand aCmd, CMTPPbCmdParam* aParam)
    {
    __ASSERT_DEBUG((aCmd > EPlaybackCmdNone && aCmd < EPlaybackCmdEnd), Panic(EMTPPBArgumentErr));
    __ASSERT_ALWAYS((aCmd > EPlaybackCmdNone && aCmd < EPlaybackCmdEnd), User::Leave(KErrArgument));
    
    CMTPPlaybackCommand* self = new (ELeave) CMTPPlaybackCommand(aCmd, aParam);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

CMTPPlaybackCommand* CMTPPlaybackCommand::NewL(const CMTPPlaybackCommand& aCmd)
    {
    CMTPPlaybackCommand* self = new (ELeave) CMTPPlaybackCommand(aCmd.PlaybackCommand(), NULL);
    CleanupStack::PushL(self);
    self->ConstructL(aCmd);
    CleanupStack::Pop(self);
    return self;
    }

/**
Destructor.
*/    
CMTPPlaybackCommand::~CMTPPlaybackCommand()
    {    
    __FLOG(_L8("~CMTPPlaybackCommand - Entry"));
    delete iParam;
    __FLOG(_L8("~CMTPPlaybackCommand - Exit"));
    __FLOG_CLOSE;
    }

/**
Constructor.
*/    
CMTPPlaybackCommand::CMTPPlaybackCommand(TMTPPlaybackCommand aCmd,
                                         CMTPPbCmdParam* aParam):
    iPbCmd(aCmd),iParam(aParam)
    {    
    }
    
/**
Second-phase constructor.
*/        
void CMTPPlaybackCommand::ConstructL()
    {
    __FLOG_OPEN(KMTPSubsystem, KComponent);
    __FLOG(_L8("CMTPPlaybackCommand: ConstructL - Entry")); 
    __FLOG(_L8("CMTPPlaybackCommand: ConstructL - Exit")); 
    }

/**
Second-phase constructor.
*/        
void CMTPPlaybackCommand::ConstructL(const CMTPPlaybackCommand& aCmd)
    {
    __FLOG(_L8("CMTPPlaybackCommand: ConstructL - Entry"));
    if(aCmd.HasParam())
        {
        iParam = CMTPPbCmdParam::NewL(aCmd.ParamL());
        }
    __FLOG(_L8("CMTPPlaybackCommand: ConstructL - Exit")); 
    }

TMTPPlaybackCommand CMTPPlaybackCommand::PlaybackCommand() const
    {
    __ASSERT_DEBUG((iPbCmd > EPlaybackCmdNone && iPbCmd < EPlaybackCmdEnd), Panic(EMTPPBArgumentErr));
    return iPbCmd;
    }

TBool CMTPPlaybackCommand::HasParam() const
    {
    TBool result(iParam != NULL);
    return result;
    }

const CMTPPbCmdParam& CMTPPlaybackCommand::ParamL() const
    {
    __ASSERT_DEBUG((iParam != NULL), Panic(EMTPPBDataNullErr));
    __ASSERT_ALWAYS((iParam != NULL), User::Leave(KErrArgument));
    return *iParam;
    }

void CMTPPlaybackCommand::SetParam(CMTPPbCmdParam* aParam)
    {
    delete iParam;
    iParam = aParam;
    }
