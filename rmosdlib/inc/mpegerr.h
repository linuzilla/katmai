/*---
Copyright (c) 1995  Microsoft Corporation

Module Name:

    mpegport.mc

Abstract:

    Log message file for Mpeg Port Driver

Author:

    Robert Nelson (robertn) 04-Sep-95

Revision History:

--*/


//
//  Status values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-------------------------+-------------------------------+
//  |Sev|C|       Facility          |               Code            |
//  +---+-+-------------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//

//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +-+-+-+-+-+---------------------+-------------------------------+
//  |S|R|C|N|r|    Facility         |               Code            |
//  +-+-+-+-+-+---------------------+-------------------------------+
//
//  where
//
//      S - Severity - indicates success/fail
//
//          0 - Success
//          1 - Fail (COERROR)
//
//      R - reserved portion of the facility code, corresponds to NT's
//              second severity bit.
//
//      C - reserved portion of the facility code, corresponds to NT's
//              C field.
//
//      N - reserved portion of the facility code. Used to indicate a
//              mapped NT status value.
//
//      r - reserved portion of the facility code. Reserved for internal
//              use. Used to indicate HRESULT values that are not status
//              values, but are instead message ids for display strings.
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//
#define FACILITY_MPEG_STATUS             0x106


//
// Define the severity codes
//
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3


//
// MessageId: MpegStatusSuccess
//
// MessageText:
//
//  MPEG no error.
//
#define MpegStatusSuccess                ((long)0x01060000L)

//
// MessageId: MpegStatusPending
//
// MessageText:
//
//  MPEG request pending.
//
#define MpegStatusPending                ((long)0x01060001L)

//
// MessageId: MpegStatusCancelled
//
// MessageText:
//
//  MPEG request cancelled.
//
#define MpegStatusCancelled              ((long)0xC1060002L)

//
// MessageId: MpegStatusNoMore
//
// MessageText:
//
//  MPEG no more entries.
//
#define MpegStatusNoMore                 ((long)0xC1060003L)

//
// MessageId: MpegStatusBusy
//
// MessageText:
//
//  MPEG device busy.
//
#define MpegStatusBusy                   ((long)0xC1060004L)

//
// MessageId: MpegStatusInvalidParameter
//
// MessageText:
//
//  MPEG invalid parameter.
//
#define MpegStatusInvalidParameter       ((long)0xC1060005L)

//
// MessageId: MpegStatusUnsupported
//
// MessageText:
//
//  MPEG request unsupported.
//
#define MpegStatusUnsupported            ((long)0xC1060006L)

//
// MessageId: MpegStatusResourceFailure
//
// MessageText:
//
//  MPEG resource failure.
//
#define MpegStatusResourceFailure        ((long)0xC1060007L)

//
// MessageId: MpegStatusHardwareFailure
//
// MessageText:
//
//  MPEG device failure.
//
#define MpegStatusHardwareFailure        ((long)0xC1060008L)

//
// MessageId: MpegStatusVersionMismatch
//
// MessageText:
//
//  MPEG mismatched component version.
//
#define MpegStatusVersionMismatch        ((long)0xC1060009L)

//
// MessageId: MpegStatusDeviceNotFound
//
// MessageText:
//
//  MPEG device not found.
//
#define MpegStatusDeviceNotFound         ((long)0xC106000AL)

//
// MessageId: MpegStatusInternalError
//
// MessageText:
//
//  MPEG internal error.
//
#define MpegStatusInternalError          ((long)0xC106000BL)

//
// MessageId: MpegStatusDeviceIOError
//
// MessageText:
//
//  MPEG internal error.
//
#define MpegStatusDeviceIOError          ((long)0xC106000CL)

