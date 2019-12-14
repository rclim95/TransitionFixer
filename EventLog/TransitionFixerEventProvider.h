 // Header
 // Messages
//
//  Values are 32 bit values laid out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
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
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//


//
// Define the severity codes
//
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_ERROR            0x3


//
// MessageId: MSG_INFO
//
// MessageText:
//
// %1
//
#define MSG_INFO                         0x40000000L

//
// MessageId: MSG_WARNING
//
// MessageText:
//
// %1
//
#define MSG_WARNING                      0x80000001L

//
// MessageId: MSG_ERROR
//
// MessageText:
//
// %1
//
#define MSG_ERROR                        0xC0000002L

//
// MessageId: MSG_SUCCESS
//
// MessageText:
//
// %1
//
#define MSG_SUCCESS                      0x00000003L

