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
* Description:  Queue implementation
*
*/


// INCLUDE FILES

#include "sconqueue.h"
#include "sconpcdconsts.h"
#include "debug.h"
#include <SWInstDefs.h> // installer errors

// ============================= MEMBER FUNCTIONS ===============================


// -----------------------------------------------------------------------------
// CSConTaskQueue::~CSConTaskQueue()
// Destructor
// -----------------------------------------------------------------------------
//
CSConTaskQueue::~CSConTaskQueue()
    {
    TRACE_FUNC;
    iQueue.ResetAndDestroy();
    iQueue.Close(); 
    iTimer.Close();
    }
    
// -----------------------------------------------------------------------------
// CSConTaskQueue::GetQueueStatus( TInt aTask, TBool aAllTasks, 
//                                  CSConStatusReply*& aStatus )
// Returns the status of a specified task / all tasks
// -----------------------------------------------------------------------------
//
void CSConTaskQueue::GetQueueStatusL( TInt aTask, TBool aAllTasks, 
                                    CSConStatusReply*& aStatus )
    {
    RArray<TInt> completedTasks;
    CleanupClosePushL( completedTasks );
    if ( aAllTasks )
        {
        //if there are tasks
        if ( iQueue.Count() > 0 )
            {
            //set iNoTasks as EFalse
            aStatus->iNoTasks = EFalse;
            for ( TInt i = 0; i < iQueue.Count(); i++ )
                {
                //Fill reply object
                CSConTaskReply* taskReply = new (ELeave) CSConTaskReply();
                CleanupStack::PushL( taskReply );
                taskReply->InitializeL( *iQueue[i] );
                User::LeaveIfError( aStatus->iTasks.Append( taskReply ) );
                CleanupStack::Pop( taskReply );
                TBool complete = iQueue[i]->GetComplete();

                //Collect completed task numbers to array for deleting
                if ( complete )
                    {
                    completedTasks.Append( iQueue[i]->iTaskId );
                    }
                //Otherwise clean all unneccessary data from the reply packet
                else
                    {
                    taskReply->CleanTaskData(); 
                    }
                }
            }
        else
            {
            //no task in the queue
            aStatus->iNoTasks = ETrue;
            }

        //Remove completed tasks from queue
        for ( TInt j = 0; j < completedTasks.Count(); j++ )
            {
            RemoveTask( completedTasks[j] );
            }
        }
    else if ( aTask > 0 )
        {
        CSConTask* temp = new (ELeave) CSConTask();
        temp->iTaskId = aTask;
        TInt index = iQueue.Find( temp, CSConTaskQueue::Match );
        delete temp;
        
        TBool complete = EFalse;
        CSConTaskReply* taskReply(NULL);

        if ( index != KErrNotFound )
            {
            aStatus->iNoTasks = EFalse;
            //Fill reply object
            taskReply = new (ELeave) CSConTaskReply();
            CleanupStack::PushL( taskReply );
            taskReply->InitializeL( *iQueue[index] );
            User::LeaveIfError( aStatus->iTasks.Append( taskReply ) );
            CleanupStack::Pop( taskReply );
            complete = iQueue[index]->GetComplete();
            }
        else
            {
            //no task in the queue
            aStatus->iNoTasks = ETrue;
            }        
        
        //Delete completed tasks from queue
        if ( complete )
            {
            RemoveTask( aTask );
            }
        //Otherwise clean all unneccessary data from the reply packet
        else if ( taskReply )
            {
            taskReply->CleanTaskData(); 
            }
        }
    else
        {
        //no task in the queue
        aStatus->iNoTasks = ETrue;
        }
    CleanupStack::PopAndDestroy( &completedTasks ); // close
    }   
    
// -----------------------------------------------------------------------------
// CSConTaskQueue::AddNewTask( CSConTask*& aNewTask, TInt aTaskId )
// Adds a new task to queue
// -----------------------------------------------------------------------------
//
TInt CSConTaskQueue::AddNewTask( CSConTask*& aNewTask, TInt aTaskId )
    {
    TInt ret( KErrNone );
    
    aNewTask->iTaskId = aTaskId;
    
    //Set progress value "task accepted for execution"
    aNewTask->SetProgressValue( KSConCodeTaskCreated );
    aNewTask->SetCompleteValue( EFalse );
    
    if ( iQueue.Count() == 0 )
        {
        StartQueue();
        }
    ret = iQueue.InsertInOrder( aNewTask, CSConTaskQueue::Compare );
    return ret;
    }
    
// -----------------------------------------------------------------------------
// CSConTaskQueue::CompleteTask( TInt aTask, TInt aError )
// Set the task to completed -mode
// -----------------------------------------------------------------------------
//
void CSConTaskQueue::CompleteTask( TInt aTask, TInt aError )
    {
    LOGGER_WRITE_1( "CSConTaskQueue::CompleteTask aError: %d", aError );
    TInt index( KErrNotFound );
    
    CSConTask* temp = new CSConTask();
    temp->iTaskId = aTask;
    index = iQueue.Find( temp, CSConTaskQueue::Match );
    delete temp;
    
    if ( index != KErrNotFound )
        {
        TBool complete( ETrue );
        TBool notComplete( EFalse );
        TInt progress( KSConCodeTaskCompleted );        
        
        switch( aError )
            {
            case KErrNone :
                iQueue[index]->SetCompleteValue( complete );
                progress =  KSConCodeTaskCompleted;
                break;
            case KErrNotFound :
                iQueue[index]->SetCompleteValue( complete );
                progress =  KSConCodeNotFound;
                break;
            case KErrCompletion :
                iQueue[index]->SetCompleteValue( notComplete );
                progress = KSConCodeTaskPartiallyCompleted;
                break;

            // installer specific errors
            case SwiUI::KSWInstErrUserCancel:
            	LOGGER_WRITE("User cancelled the operation");
            	iQueue[index]->SetCompleteValue( complete );
            	progress = KSConCodeInstErrUserCancel;
            	break;
            case SwiUI::KSWInstErrFileCorrupted:
            	LOGGER_WRITE("File is corrupted");
            	iQueue[index]->SetCompleteValue( complete );
            	progress = KSConCodeInstErrFileCorrupted;
            	break;
            case SwiUI::KSWInstErrInsufficientMemory:
            	LOGGER_WRITE("Insufficient free memory in the drive to perform the operation");
	            iQueue[index]->SetCompleteValue( complete );
	            progress = KSConCodeInstErrInsufficientMemory;	
	            break;
            case SwiUI::KSWInstErrPackageNotSupported:
            	LOGGER_WRITE("Installation of the package is not supported");
            	iQueue[index]->SetCompleteValue( complete );
            	progress = KSConCodeInstErrPackageNotSupported;
            	break;
            case SwiUI::KSWInstErrSecurityFailure:
            	LOGGER_WRITE("Package cannot be installed due to security error");
            	iQueue[index]->SetCompleteValue( complete );
            	progress = KSConCodeInstErrSecurityFailure;
            	break;
            case SwiUI::KSWInstErrMissingDependency:
            	LOGGER_WRITE("Package cannot be installed due to missing dependency");
            	iQueue[index]->SetCompleteValue( complete );
            	progress = KSConCodeInstErrMissingDependency;
            	break;
            case SwiUI::KSWInstErrFileInUse:
            	LOGGER_WRITE("Mandatory file is in use and prevents the operation");
            	iQueue[index]->SetCompleteValue( complete );
            	progress = KSConCodeInstErrFileInUse;
            	break;
            case SwiUI::KSWInstErrGeneralError:
            	LOGGER_WRITE("Unknown error");
            	iQueue[index]->SetCompleteValue( complete );
            	progress = KSConCodeInstErrGeneralError;
            	break;
            case SwiUI::KSWInstErrNoRights:
            	LOGGER_WRITE("The package has no rights to perform the operation");
            	iQueue[index]->SetCompleteValue( complete );
            	progress = KSConCodeInstErrNoRights;
            	break;
            case SwiUI::KSWInstErrNetworkFailure:
            	LOGGER_WRITE("Indicates that network failure aborted the operation");
            	iQueue[index]->SetCompleteValue( complete );
            	progress = KSConCodeInstErrNetworkFailure;
            	break;
            case SwiUI::KSWInstErrBusy:
            	LOGGER_WRITE("Installer is busy doing some other operation");
            	iQueue[index]->SetCompleteValue( complete );
        		progress = KSConCodeInstErrBusy;
            	break;
            case SwiUI::KSWInstErrAccessDenied:
            	LOGGER_WRITE("Target location of package is not accessible");
            	iQueue[index]->SetCompleteValue( complete );
            	progress = KSConCodeInstErrAccessDenied;
            	break;
            case SwiUI::KSWInstUpgradeError:
            	LOGGER_WRITE("The package is an invalid upgrade");
            	iQueue[index]->SetCompleteValue( complete );
            	progress = KSConCodeInstUpgradeError;
            	break;
            
            default :
                iQueue[index]->SetCompleteValue( complete );
                progress = KSConCodeConflict;
                break;
            }
            
        iQueue[index]->SetProgressValue( progress );
        }
    StartQueue();
    }
    
// -----------------------------------------------------------------------------
// CSConTaskQueue::SetTaskProgress( TInt aTask, TInt aProgressValue )
// Set the task progress value
// -----------------------------------------------------------------------------
//
void CSConTaskQueue::SetTaskProgress( TInt aTask, TInt aProgressValue )
    {
    TInt index( KErrNotFound );

    CSConTask* temp = new CSConTask();
    temp->iTaskId = aTask;
    index = iQueue.Find( temp, CSConTaskQueue::Match );
    delete temp;
    
    if ( index != KErrNotFound )
        {
        iQueue[index]->SetProgressValue( aProgressValue );
        }
    }
    
// -----------------------------------------------------------------------------
// CSConTaskQueue::GetTask( TInt aTaskId, CSConTask*& aTask )
// Receives a specified task
// -----------------------------------------------------------------------------
//
TInt CSConTaskQueue::GetTask( TInt aTaskId, CSConTask*& aTask )
    {
    TInt ret( KErrNone );
    TInt index;
    
    CSConTask* temp = new CSConTask();
    temp->iTaskId = aTaskId;
    index = iQueue.Find( temp, CSConTaskQueue::Match );
    delete temp;

    if ( index != KErrNotFound )
        {
        aTask = iQueue[index];
        }
    else
        {
        ret = KErrNotFound;
        }
    return ret;
    }
    
// -----------------------------------------------------------------------------
// CSConTaskQueue::RemoveTask( TInt aTask )
// Removes a task from the queue
// -----------------------------------------------------------------------------
//
void CSConTaskQueue::RemoveTask( TInt aTask )
    {
    TInt index( KErrNotFound );
    
    CSConTask* temp = new CSConTask();
    temp->iTaskId = aTask;
    index = iQueue.Find( temp, CSConTaskQueue::Match );
    delete temp;
    
    if ( index != KErrNotFound ) 
        {
        delete iQueue[index];
        iQueue.Remove( index );
        iQueue.Compress();
        }
    
    if ( iQueue.Count() == 0 )
        {
        StopQueue();
        iQueue.Reset();
        }
    }
    
// -----------------------------------------------------------------------------
// CSConTaskQueue::CancelTask( TInt aTask, TBool aAllTasks )
// Cancels a task
// -----------------------------------------------------------------------------
//
void CSConTaskQueue::CancelTask( TInt aTask, TBool aAllTasks )
    {
    TRACE_FUNC_ENTRY;
    
    //Remove the task from the queue
    if ( aTask > 0 && !aAllTasks )
        {
        RemoveTask( aTask );
        }
        
    //Remove all tasks from the queue
    if ( aAllTasks )
        {
        iQueue.ResetAndDestroy();
        }
    
    TRACE_FUNC_EXIT;
    }
    
// -----------------------------------------------------------------------------
// CSConTaskQueue::QueueProcessActive()
// The status of the process
// -----------------------------------------------------------------------------
//  
TBool CSConTaskQueue::QueueProcessActive() const
    {
    return iQueueProcessActive;
    }

// -----------------------------------------------------------------------------
// CSConTaskQueue::ChangeQueueProcessStatus()
// Changes the status of the queue process
// -----------------------------------------------------------------------------
//  
void CSConTaskQueue::ChangeQueueProcessStatus()
    {
    iQueueProcessActive = !iQueueProcessActive;
    }
    
// -----------------------------------------------------------------------------
// CSConTaskQueue::Reset()
// Resets the queue
// -----------------------------------------------------------------------------
//
void CSConTaskQueue::Reset()
    {
    TRACE_FUNC_ENTRY;
    iTimer.Cancel();
    iQueue.ResetAndDestroy();
    TRACE_FUNC_EXIT;
    }

// ---------------------------------------------------------
// CSConTaskQueue::Compare( const CSConTask& aFirst, 
//                          const CSConTask& aSecond )
// Compares task numbers
// ---------------------------------------------------------    
TInt CSConTaskQueue::Compare( const CSConTask& aFirst, 
                            const CSConTask& aSecond )
    {
    if ( aFirst.iTaskId < aSecond.iTaskId )
        {
        return -1;
        }
    else if ( aFirst.iTaskId > aSecond.iTaskId )
        {
        return 1;
        }
    
    return 0;
    }
    
// -----------------------------------------------------------------------------
// CSConTaskQueue::Match( const CSConTask& aFirst, const CSConTask& aSecond )
// Matches the task numbers
// -----------------------------------------------------------------------------
//
TInt CSConTaskQueue::Match( const CSConTask& aFirst, const CSConTask& aSecond )
    {
    if ( aFirst.iTaskId == aSecond.iTaskId )
        {
        return ETrue;
        }
        
    return EFalse;
    }
// End of file
