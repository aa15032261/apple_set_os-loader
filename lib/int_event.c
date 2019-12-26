#include "../include/int_event.h"

EFI_STATUS _INT_WaitForSingleEvent(EFI_BOOT_SERVICES* BS, EFI_EVENT Event, UINT64 Timeout)
{
    EFI_STATUS          Status;
    UINTN               Index;
    EFI_EVENT           TimerEvent;
    EFI_EVENT           WaitList[2];

    if (Timeout) {
        //
        // Create a timer event
        //

        Status = BS->CreateEvent(EVT_TIMER, 0, NULL, NULL, &TimerEvent);
        if (!EFI_ERROR(Status)) {

            //
            // Set the timer event
            //

            BS->SetTimer(TimerEvent, TimerRelative, Timeout);

            //
            // Wait for the original event or the timer
            //

            WaitList[0] = Event;
            WaitList[1] = TimerEvent;
            Status = BS->WaitForEvent(2, WaitList, &Index);
            BS->CloseEvent(TimerEvent);

            //
            // If the timer expired, change the return to timed out
            //

            if (!EFI_ERROR(Status)  &&  Index == 1) {
                Status = EFI_TIMEOUT;
            }
        }

    } else {

        //
        // No timeout... just wait on the event
        //

        Status = BS->WaitForEvent(1, &Event, &Index);
    }

    return Status;
}
