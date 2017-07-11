/*
 * ast_header.h
 *
 *  Created on: Feb 28, 2017
 *      Author: pchero
 */

#ifndef BACKEND_SRC_AST_HEADER_H_
#define BACKEND_SRC_AST_HEADER_H_


/*! \page AstCauses Hangup Causes for Asterisk

The Asterisk hangup causes are delivered to the dialplan in the
${HANGUPCAUSE} channel variable after a call (after execution
of "dial").

In SIP, we have a conversion table to convert between SIP
return codes and Q.931 both ways. This is to improve SIP/ISDN
compatibility.

These are the current codes, based on the Q.850/Q.931
specification:

  - AST_CAUSE_UNALLOCATED                      1
  - AST_CAUSE_NO_ROUTE_TRANSIT_NET             2
  - AST_CAUSE_NO_ROUTE_DESTINATION             3
  - AST_CAUSE_MISDIALLED_TRUNK_PREFIX          5
  - AST_CAUSE_CHANNEL_UNACCEPTABLE             6
  - AST_CAUSE_CALL_AWARDED_DELIVERED           7
  - AST_CAUSE_PRE_EMPTED                       8
  - AST_CAUSE_NUMBER_PORTED_NOT_HERE          14
  - AST_CAUSE_NORMAL_CLEARING                 16
  - AST_CAUSE_USER_BUSY                       17
  - AST_CAUSE_NO_USER_RESPONSE                18
  - AST_CAUSE_NO_ANSWER                       19
  - AST_CAUSE_CALL_REJECTED                   21
  - AST_CAUSE_NUMBER_CHANGED                  22
  - AST_CAUSE_REDIRECTED_TO_NEW_DESTINATION   23
  - AST_CAUSE_ANSWERED_ELSEWHERE              26
  - AST_CAUSE_DESTINATION_OUT_OF_ORDER        27
  - AST_CAUSE_INVALID_NUMBER_FORMAT           28
  - AST_CAUSE_FACILITY_REJECTED               29
  - AST_CAUSE_RESPONSE_TO_STATUS_ENQUIRY      30
  - AST_CAUSE_NORMAL_UNSPECIFIED              31
  - AST_CAUSE_NORMAL_CIRCUIT_CONGESTION       34
  - AST_CAUSE_NETWORK_OUT_OF_ORDER            38
  - AST_CAUSE_NORMAL_TEMPORARY_FAILURE        41
  - AST_CAUSE_SWITCH_CONGESTION               42
  - AST_CAUSE_ACCESS_INFO_DISCARDED           43
  - AST_CAUSE_REQUESTED_CHAN_UNAVAIL          44
  - AST_CAUSE_FACILITY_NOT_SUBSCRIBED         50
  - AST_CAUSE_OUTGOING_CALL_BARRED            52
  - AST_CAUSE_INCOMING_CALL_BARRED            54
  - AST_CAUSE_BEARERCAPABILITY_NOTAUTH        57
  - AST_CAUSE_BEARERCAPABILITY_NOTAVAIL       58
  - AST_CAUSE_BEARERCAPABILITY_NOTIMPL        65
  - AST_CAUSE_CHAN_NOT_IMPLEMENTED            66
  - AST_CAUSE_FACILITY_NOT_IMPLEMENTED        69
  - AST_CAUSE_INVALID_CALL_REFERENCE          81
  - AST_CAUSE_INCOMPATIBLE_DESTINATION        88
  - AST_CAUSE_INVALID_MSG_UNSPECIFIED         95
  - AST_CAUSE_MANDATORY_IE_MISSING            96
  - AST_CAUSE_MESSAGE_TYPE_NONEXIST           97
  - AST_CAUSE_WRONG_MESSAGE                   98
  - AST_CAUSE_IE_NONEXIST                     99
  - AST_CAUSE_INVALID_IE_CONTENTS            100
  - AST_CAUSE_WRONG_CALL_STATE               101
  - AST_CAUSE_RECOVERY_ON_TIMER_EXPIRE       102
  - AST_CAUSE_MANDATORY_IE_LENGTH_ERROR      103
  - AST_CAUSE_PROTOCOL_ERROR                 111
  - AST_CAUSE_INTERWORKING                   127

For more information:
- \ref app_dial.c
*/

/*! \name Causes for disconnection (from Q.850/Q.931)
 *  These are the internal cause codes used in Asterisk.
 *  \ref AstCauses
 */
/*@{ */
#define AST_CAUSE_UNALLOCATED                    1
#define AST_CAUSE_NO_ROUTE_TRANSIT_NET           2
#define AST_CAUSE_NO_ROUTE_DESTINATION           3
#define AST_CAUSE_MISDIALLED_TRUNK_PREFIX        5
#define AST_CAUSE_CHANNEL_UNACCEPTABLE           6
#define AST_CAUSE_CALL_AWARDED_DELIVERED         7
#define AST_CAUSE_PRE_EMPTED                     8
#define AST_CAUSE_NUMBER_PORTED_NOT_HERE        14
#define AST_CAUSE_NORMAL_CLEARING               16
#define AST_CAUSE_USER_BUSY                     17
#define AST_CAUSE_NO_USER_RESPONSE              18
#define AST_CAUSE_NO_ANSWER                     19
#define AST_CAUSE_SUBSCRIBER_ABSENT             20
#define AST_CAUSE_CALL_REJECTED                 21
#define AST_CAUSE_NUMBER_CHANGED                22
#define AST_CAUSE_REDIRECTED_TO_NEW_DESTINATION 23
#define AST_CAUSE_ANSWERED_ELSEWHERE            26
#define AST_CAUSE_DESTINATION_OUT_OF_ORDER      27
#define AST_CAUSE_INVALID_NUMBER_FORMAT         28
#define AST_CAUSE_FACILITY_REJECTED             29
#define AST_CAUSE_RESPONSE_TO_STATUS_ENQUIRY    30
#define AST_CAUSE_NORMAL_UNSPECIFIED            31
#define AST_CAUSE_NORMAL_CIRCUIT_CONGESTION     34
#define AST_CAUSE_NETWORK_OUT_OF_ORDER          38
#define AST_CAUSE_NORMAL_TEMPORARY_FAILURE      41
#define AST_CAUSE_SWITCH_CONGESTION             42
#define AST_CAUSE_ACCESS_INFO_DISCARDED         43
#define AST_CAUSE_REQUESTED_CHAN_UNAVAIL        44
#define AST_CAUSE_FACILITY_NOT_SUBSCRIBED       50
#define AST_CAUSE_OUTGOING_CALL_BARRED          52
#define AST_CAUSE_INCOMING_CALL_BARRED          54
#define AST_CAUSE_BEARERCAPABILITY_NOTAUTH      57
#define AST_CAUSE_BEARERCAPABILITY_NOTAVAIL     58
#define AST_CAUSE_BEARERCAPABILITY_NOTIMPL      65
#define AST_CAUSE_CHAN_NOT_IMPLEMENTED          66
#define AST_CAUSE_FACILITY_NOT_IMPLEMENTED      69
#define AST_CAUSE_INVALID_CALL_REFERENCE        81
#define AST_CAUSE_INCOMPATIBLE_DESTINATION      88
#define AST_CAUSE_INVALID_MSG_UNSPECIFIED       95
#define AST_CAUSE_MANDATORY_IE_MISSING          96
#define AST_CAUSE_MESSAGE_TYPE_NONEXIST         97
#define AST_CAUSE_WRONG_MESSAGE                 98
#define AST_CAUSE_IE_NONEXIST                   99
#define AST_CAUSE_INVALID_IE_CONTENTS          100
#define AST_CAUSE_WRONG_CALL_STATE             101
#define AST_CAUSE_RECOVERY_ON_TIMER_EXPIRE     102
#define AST_CAUSE_MANDATORY_IE_LENGTH_ERROR    103
#define AST_CAUSE_PROTOCOL_ERROR               111
#define AST_CAUSE_INTERWORKING                 127

/* Special Asterisk aliases */
#define AST_CAUSE_BUSY          AST_CAUSE_USER_BUSY
#define AST_CAUSE_FAILURE       AST_CAUSE_NETWORK_OUT_OF_ORDER
#define AST_CAUSE_NORMAL        AST_CAUSE_NORMAL_CLEARING
#define AST_CAUSE_NOANSWER      AST_CAUSE_NO_ANSWER
#define AST_CAUSE_CONGESTION    AST_CAUSE_NORMAL_CIRCUIT_CONGESTION
#define AST_CAUSE_UNREGISTERED  AST_CAUSE_SUBSCRIBER_ABSENT
#define AST_CAUSE_NOTDEFINED    0
#define AST_CAUSE_NOSUCHDRIVER  AST_CAUSE_CHAN_NOT_IMPLEMENTED
/*@} */


// device status
#define AST_DEVICE_UNKNOWN      0
#define AST_DEVICE_NOT_INUSE    1
#define AST_DEVICE_INUSE        2
#define AST_DEVICE_BUSY         3
#define AST_DEVICE_INVALID      4
#define AST_DEVICE_UNAVAILABLE  5
#define AST_DEVICE_RINGING      6
#define AST_DEVICE_RINGINUSE    7
#define AST_DEVICE_ONHOLD       8

enum ast_control_frame_type {
  AST_CONTROL_HANGUP = 1,     /*!< Other end has hungup */
  AST_CONTROL_RING = 2,     /*!< Local ring */
  AST_CONTROL_RINGING = 3,    /*!< Remote end is ringing */
  AST_CONTROL_ANSWER = 4,     /*!< Remote end has answered */
  AST_CONTROL_BUSY = 5,     /*!< Remote end is busy */
  AST_CONTROL_TAKEOFFHOOK = 6,  /*!< Make it go off hook */
  AST_CONTROL_OFFHOOK = 7,    /*!< Line is off hook */
  AST_CONTROL_CONGESTION = 8,   /*!< Congestion (circuits busy) */
  AST_CONTROL_FLASH = 9,      /*!< Flash hook */
  AST_CONTROL_WINK = 10,      /*!< Wink */
  AST_CONTROL_OPTION = 11,    /*!< Set a low-level option */
  AST_CONTROL_RADIO_KEY = 12,   /*!< Key Radio */
  AST_CONTROL_RADIO_UNKEY = 13, /*!< Un-Key Radio */
  AST_CONTROL_PROGRESS = 14,    /*!< Indicate PROGRESS */
  AST_CONTROL_PROCEEDING = 15,  /*!< Indicate CALL PROCEEDING */
  AST_CONTROL_HOLD = 16,      /*!< Indicate call is placed on hold */
  AST_CONTROL_UNHOLD = 17,    /*!< Indicate call is left from hold */
  AST_CONTROL_VIDUPDATE = 18,   /*!< Indicate video frame update */
  _XXX_AST_CONTROL_T38 = 19,    /*!< T38 state change request/notification \deprecated This is no longer supported. Use AST_CONTROL_T38_PARAMETERS instead. */
  AST_CONTROL_SRCUPDATE = 20,   /*!< Indicate source of media has changed */
  AST_CONTROL_TRANSFER = 21,    /*!< Indicate status of a transfer request */
  AST_CONTROL_CONNECTED_LINE = 22,/*!< Indicate connected line has changed */
  AST_CONTROL_REDIRECTING = 23, /*!< Indicate redirecting id has changed */
  AST_CONTROL_T38_PARAMETERS = 24,/*!< T38 state change request/notification with parameters */
  AST_CONTROL_CC = 25,      /*!< Indication that Call completion service is possible */
  AST_CONTROL_SRCCHANGE = 26,   /*!< Media source has changed and requires a new RTP SSRC */
  AST_CONTROL_READ_ACTION = 27, /*!< Tell ast_read to take a specific action */
  AST_CONTROL_AOC = 28,     /*!< Advice of Charge with encoded generic AOC payload */
  AST_CONTROL_END_OF_Q = 29,    /*!< Indicate that this position was the end of the channel queue for a softhangup. */
  AST_CONTROL_INCOMPLETE = 30,  /*!< Indication that the extension dialed is incomplete */
  AST_CONTROL_MCID = 31,      /*!< Indicate that the caller is being malicious. */
  AST_CONTROL_UPDATE_RTP_PEER = 32, /*!< Interrupt the bridge and have it update the peer */
  AST_CONTROL_PVT_CAUSE_CODE = 33, /*!< Contains an update to the protocol-specific cause-code stored for branching dials */
  AST_CONTROL_MASQUERADE_NOTIFY = 34, /*!< A masquerade is about to begin/end. (Never sent as a frame but directly with ast_indicate_data().) */

  /*
   * WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
   *
   * IAX2 sends these values out over the wire.  To prevent future
   * incompatibilities, pick the next value in the enum from whatever
   * is on the current trunk.  If you lose the merge race you need to
   * fix the previous branches to match what is on trunk.  In addition
   * you need to change chan_iax2 to explicitly allow the control
   * frame over the wire if it makes sense for the frame to be passed
   * to another Asterisk instance.
   *
   * WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
   */

  /* Control frames used to manipulate a stream on a channel. The values for these
   * must be greater than the allowed value for a 8-bit char, so that they avoid
   * conflicts with DTMF values. */
  AST_CONTROL_STREAM_STOP = 1000,   /*!< Indicate to a channel in playback to stop the stream */
  AST_CONTROL_STREAM_SUSPEND = 1001,  /*!< Indicate to a channel in playback to suspend the stream */
  AST_CONTROL_STREAM_RESTART = 1002,  /*!< Indicate to a channel in playback to restart the stream */
  AST_CONTROL_STREAM_REVERSE = 1003,  /*!< Indicate to a channel in playback to rewind */
  AST_CONTROL_STREAM_FORWARD = 1004,  /*!< Indicate to a channel in playback to fast forward */
  /* Control frames to manipulate recording on a channel. */
  AST_CONTROL_RECORD_CANCEL = 1100, /*!< Indicated to a channel in record to stop recording and discard the file */
  AST_CONTROL_RECORD_STOP = 1101, /*!< Indicated to a channel in record to stop recording */
  AST_CONTROL_RECORD_SUSPEND = 1102,  /*!< Indicated to a channel in record to suspend/unsuspend recording */
  AST_CONTROL_RECORD_MUTE = 1103, /*!< Indicated to a channel in record to mute/unmute (i.e. write silence) recording */
};




#endif /* BACKEND_SRC_AST_HEADER_H_ */
