/*
* Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  Backup-Restore Queue implementation
*
*/


// INCLUDE FILES
#include "sconbrqueue.h"
#include "sconbackuprestore.h"
#include "sconpcdconsts.h"
#include "sconinstqueue.h"
#include "debug.h"

// -----------------------------------------------------------------------------
// CSConBackupRestoreQueue::NewL( const TInt aMaxObjectSize )
// Two-phase constructor
// -----------------------------------------------------------------------------
//
CSConBackupRestoreQueue* CSConBackupRestoreQueue::NewL( const TInt aMaxObjectSize, RFs& aFs )
	{
	CSConBackupRestoreQueue* self = new (ELeave) CSConBackupRestoreQueue();
	CleanupStack::PushL( self );
	self->ConstructL( aMaxObjectSize, aFs );
	CleanupStack::Pop( self );
    return self;
	}
	
// -----------------------------------------------------------------------------
// CSConBackupRestoreQueue::CSConBackupRestoreQueue()
// Destructor
// -----------------------------------------------------------------------------
//
CSConBackupRestoreQueue::CSConBackupRestoreQueue() : 
					CActive( EPriorityStandard )
	{
	}
	
// -----------------------------------------------------------------------------
// CSConBackupRestoreQueue::ConstructL( const TInt aMaxObjectSize )
// Initializes member data
// -----------------------------------------------------------------------------
//
void CSConBackupRestoreQueue::ConstructL( const TInt aMaxObjectSize, RFs& aFs )
	{
	iBackupRestore = CSConBackupRestore::NewL( this, aMaxObjectSize, aFs );
	CActiveScheduler::Add( iBackupRestore );
	User::LeaveIfError( iTimer.CreateLocal() );
	}
	
// -----------------------------------------------------------------------------
// CSConBackupRestoreQueue::~CSConBackupRestoreQueue()
// Destructor
// -----------------------------------------------------------------------------
//
CSConBackupRestoreQueue::~CSConBackupRestoreQueue()
	{
	TRACE_FUNC_ENTRY;
	Cancel();
	if( iBackupRestore )
		{
		delete iBackupRestore;
		iBackupRestore = NULL;
		}
	
	TRACE_FUNC_EXIT;
	}
	
// -----------------------------------------------------------------------------
// CSConBackupRestoreQueue::StartQueue()
// Starts queue polling
// -----------------------------------------------------------------------------
//
void CSConBackupRestoreQueue::StartQueue()	
	{
	if( IsActive() )
		{
		Cancel();
		}
		
	iTimer.After( iStatus, KSConTimerValue );
	SetActive();
	}
	
// -----------------------------------------------------------------------------
// CSConBackupRestoreQueue::StopQueue()
// Stops queue polling
// -----------------------------------------------------------------------------
//
void CSConBackupRestoreQueue::StopQueue()	
	{
	iTimer.Cancel();
	}

// -----------------------------------------------------------------------------
// CSConBackupRestoreQueue::AddNewTask( CSConTask*& aNewTask, TInt aTaskId )
// Adds a new task to queue
// -----------------------------------------------------------------------------
//
TInt CSConBackupRestoreQueue::AddNewTask( CSConTask*& aNewTask, TInt aTaskId )
	{
	LOGGER_WRITE_1( "CSConBackupRestoreQueue::AddNewTask aTaskId: %d", aTaskId );
	TInt ret( KErrNone );
	
	aNewTask->iTaskId = aTaskId;
	
	//Set progress value "task accepted for execution"
	aNewTask->SetProgressValue( KSConCodeTaskCreated );
	aNewTask->SetCompleteValue( EFalse );
	
	//For RequestData and SupplyData
	if( iQueue.Find( aNewTask, CSConTaskQueue::Match ) != KErrNotFound )
		{
		RemoveTask( aTaskId );
		}
	
	if( iQueue.Count() == 0 )
		{
		StartQueue();
		}

	ret = iQueue.InsertInOrder( aNewTask, CSConTaskQueue::Compare );
	return ret;
	}

// -----------------------------------------------------------------------------
// CSConBackupRestoreQueue::CancelTask( TInt aTask, TBool aAllTasks )
// Cancels a task
// -----------------------------------------------------------------------------
//
void CSConBackupRestoreQueue::CancelTask( TInt aTask, TBool aAllTasks )
	{
	TRACE_FUNC_ENTRY;
	//Stop backup/restore
	if( aTask && !aAllTasks )
		{
		LOGGER_WRITE_1("CSConBackupRestoreQueue::CancelTask - Cancel task: %d", aTask);
		iBackupRestore->StopBackupRestore( aTask );
		}
	
	if( aAllTasks )
		{
		LOGGER_WRITE("CSConBackupRestoreQueue::CancelTask - Cancel All");
		iBackupRestore->Cancel();
		iBackupRestore->Reset();
		}
		
	CSConTaskQueue::CancelTask( aTask, aAllTasks );
	TRACE_FUNC_EXIT;
	}

// -----------------------------------------------------------------------------
// CSConBackupRestoreQueue::Reset()
// Resets the queue
// -----------------------------------------------------------------------------
//
void CSConBackupRestoreQueue::Reset()
	{
	TRACE_FUNC_ENTRY;
	CSConTaskQueue::Reset();
	iBackupRestore->Reset();
	TRACE_FUNC_EXIT;
	}
	
// -----------------------------------------------------------------------------
// CSConBackupRestoreQueue::QueueAddress( CSConInstallerQueue*& aTaskQueue )
// An address pointer to another queue
// -----------------------------------------------------------------------------
//
void CSConBackupRestoreQueue::QueueAddress( CSConInstallerQueue*& aTaskQueue )
	{
	iInstQueueAddress = aTaskQueue;
	}

// -----------------------------------------------------------------------------
// CSConBackupRestoreQueue::GetTaskMethod( TInt& aTaskId )
// Returns the task type
// -----------------------------------------------------------------------------
//		
TSConMethodName CSConBackupRestoreQueue::GetTaskMethodL( TInt aTaskId )
	{
	TRACE_FUNC_ENTRY;
	CSConTask* task = NULL;
	CSConTaskQueue::GetTask( aTaskId, task );
	LOGGER_WRITE_1( "CSConBackupRestoreQueue::GetTaskMethodL( TInt aTaskId ) : returned %d",
        task->GetServiceId() );
	return task->GetServiceId();
	}
	
// -----------------------------------------------------------------------------
// CSConBackupRestoreQueue::PollQueue()
// Polls queue
// -----------------------------------------------------------------------------
//
void CSConBackupRestoreQueue::PollQueue()
	{
	// find and start next task if BR and installer is inactive
	if( !iBackupRestore->BackupRestoreActive()
		&& !iInstQueueAddress->QueueProcessActive() )
		{
		//find next task
		for( TInt i = 0; i < iQueue.Count(); i++ )
			{
			TBool complete = iQueue[i]->GetCompleteValue();
			
			if( complete == EFalse )
				{
				iBackupRestore->StartBackupRestore( 
				iQueue[i]->iTaskId );
				i = iQueue.Count() + 1; // jump out from loop
				}
			}
		}
	}

// -----------------------------------------------------------------------------
// Implementation of CActive::DoCancel()
// Entry to CSConPCD
// -----------------------------------------------------------------------------
//
void CSConBackupRestoreQueue::DoCancel()
	{
	TRACE_FUNC_ENTRY;
	iTimer.Cancel();
	TRACE_FUNC_EXIT;
	}
	
// -----------------------------------------------------------------------------
// Implementation of CActive::RunL()
// Entry to CSConPCD
// -----------------------------------------------------------------------------
//
void CSConBackupRestoreQueue::RunL()
	{
	if( iQueue.Count() > 0 )
		{
		PollQueue();
		StartQueue();
		}
	}

// End of file
