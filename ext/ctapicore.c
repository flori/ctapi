/* 
 * Document-module: CTAPICore
 *
 * The CTAPICore that interfaces with the shared library for your cardterminal.
 * The following constants are defined in this module:
 *  CT
 *  CTBCS_CLA
 *  CTBCS_DATA_STATUS_CARD
 *  CTBCS_DATA_STATUS_CARD_CONNECT
 *  CTBCS_DATA_STATUS_NOCARD
 *  CTBCS_INS_EJECT
 *  CTBCS_INS_REQUEST
 *  CTBCS_INS_RESET
 *  CTBCS_INS_STATUS
 *  CTBCS_MIN_COMMAND_SIZE
 *  CTBCS_MIN_RESPONSE_SIZE
 *  CTBCS_P1_CT_KERNEL
 *  CTBCS_P1_DISPLAY
 *  CTBCS_P1_INTERFACE1
 *  CTBCS_P1_INTERFACE10
 *  CTBCS_P1_INTERFACE11
 *  CTBCS_P1_INTERFACE12
 *  CTBCS_P1_INTERFACE13
 *  CTBCS_P1_INTERFACE14
 *  CTBCS_P1_INTERFACE2
 *  CTBCS_P1_INTERFACE3
 *  CTBCS_P1_INTERFACE4
 *  CTBCS_P1_INTERFACE5
 *  CTBCS_P1_INTERFACE6
 *  CTBCS_P1_INTERFACE7
 *  CTBCS_P1_INTERFACE8
 *  CTBCS_P1_INTERFACE9
 *  CTBCS_P1_KEYPAD
 *  CTBCS_P2_REQUEST_GET_ATR
 *  CTBCS_P2_REQUEST_GET_HIST
 *  CTBCS_P2_REQUEST_NO_RESP
 *  CTBCS_P2_RESET_GET_ATR
 *  CTBCS_P2_RESET_GET_HIST
 *  CTBCS_P2_RESET_NO_RESP
 *  CTBCS_P2_STATUS_ICC
 *  CTBCS_P2_STATUS_MANUFACTURER
 *  CTBCS_SW1_COMMAND_NOT_ALLOWED
 *  CTBCS_SW1_EJECT_NOT_REMOVED
 *  CTBCS_SW1_EJECT_OK
 *  CTBCS_SW1_EJECT_REMOVED
 *  CTBCS_SW1_ICC_ERROR
 *  CTBCS_SW1_OK
 *  CTBCS_SW1_REQUEST_ASYNC_OK
 *  CTBCS_SW1_REQUEST_CARD_PRESENT
 *  CTBCS_SW1_REQUEST_ERROR
 *  CTBCS_SW1_REQUEST_NO_CARD
 *  CTBCS_SW1_REQUEST_SYNC_OK
 *  CTBCS_SW1_REQUEST_TIMER_ERROR
 *  CTBCS_SW1_RESET_ASYNC_OK
 *  CTBCS_SW1_RESET_CT_OK
 *  CTBCS_SW1_RESET_ERROR
 *  CTBCS_SW1_RESET_SYNC_OK
 *  CTBCS_SW1_WRONG_CLA
 *  CTBCS_SW1_WRONG_INS
 *  CTBCS_SW1_WRONG_LENGTH
 *  CTBCS_SW1_WRONG_PARAM
 *  CTBCS_SW2_COMMAND_NOT_ALLOWED
 *  CTBCS_SW2_EJECT_NOT_REMOVED
 *  CTBCS_SW2_EJECT_OK
 *  CTBCS_SW2_EJECT_REMOVED
 *  CTBCS_SW2_ICC_ERROR
 *  CTBCS_SW2_OK
 *  CTBCS_SW2_REQUEST_ASYNC_OK
 *  CTBCS_SW2_REQUEST_CARD_PRESENT
 *  CTBCS_SW2_REQUEST_ERROR
 *  CTBCS_SW2_REQUEST_NO_CARD
*  CTBCS_SW2_REQUEST_SYNC_OK
*  CTBCS_SW2_REQUEST_TIMER_ERROR
*  CTBCS_SW2_RESET_ASYNC_OK
*  CTBCS_SW2_RESET_CT_OK
*  CTBCS_SW2_RESET_ERROR
*  CTBCS_SW2_RESET_SYNC_OK
*  CTBCS_SW2_WRONG_CLA
*  CTBCS_SW2_WRONG_INS
*  CTBCS_SW2_WRONG_LENGTH
*  CTBCS_SW2_WRONG_PARAM
*  ERR_CT
*  ERR_HTSI
*  ERR_INVALID
*  ERR_MEMORY
*  ERR_TRANS
*  HOST
*  HTSIError
*  ICC1
*  ICC10
*  ICC11
*  ICC12
*  ICC13
*  ICC14
*  ICC2
*  ICC3
*  ICC4
*  ICC5
*  ICC6
*  ICC7
*  ICC8
*  ICC9
*  MAX_APDULEN
*  OK
*  PORT_COM1
*  PORT_COM2
*  PORT_COM3
*  PORT_COM4
*  PORT_LPT1
*  PORT_LPT2
*  PORT_Modem
*  PORT_Printer
*/

#include "ruby.h"
#include <ctapi.h>

#define NUM2USHRT(n) NUM2UINT(n)

static VALUE mCTAPICore, eCTAPIError, eInvalidError, eCardterminalError,
             eTransmissionError, eMemoryError, eHTSIError, eUnknownError;

/*
 * Wrap the returned errors into a nice Ruby Exception and throw it.
 */
void try(char rv) {
    switch (rv) {
        case OK:
        return;
        case ERR_INVALID:
        rb_raise(eInvalidError, "Invalid Data: %d", rv);
        case ERR_CT:
        rb_raise(eCardterminalError, "Cardterminal Error: %d", rv);
        case ERR_TRANS:
        rb_raise(eTransmissionError, "Transmission Error: %d", rv);
        case ERR_MEMORY:
        rb_raise(eMemoryError, "Memory Allocate Error: %d", rv);
        case ERR_HTSI:
        rb_raise(eHTSIError, "Host Transport Service Interface Error:: %d",
                rv); default:    
            rb_raise(eUnknownError, "Unknown Error: %d", rv);
    }
}

/*
 * We wrap CT_init to get the exceptions right.
 */
void ct_init(unsigned short Ctn, unsigned short pn) {
    try(CT_init(Ctn, pn));
}

/*
 * Print the buffer (of length length) as byte sequence to stderr. This
 * is used for debugging.
 */
void 
debug_command (unsigned char * buffer, unsigned length)
{
    unsigned i;

    if (length > 16)
            fprintf (stderr, "\n");

    for (i=0; i<length; i++)
    {
        fprintf (stderr, "%02X ", buffer[i]);
        if (i%16 == 15)
                fprintf (stderr, "\n");
    }

    if (i%16 != 0)
            fprintf (stderr, "\n");
}

/*
 * We wrap CT_data to get the exceptions and debugging right.
 */
char *ct_data( 
        unsigned short Ctn,    /* Terminal Number */
        unsigned char  dad,    /* Destination */
        unsigned char  sad,    /* Source */
        unsigned char *cmd,    /* Command/Data Buffer */
        unsigned short lc,    /* Length of Command */
        unsigned short *lr    /* Length of Response */
        )
{
    *lr = MAX_APDULEN;
    unsigned char *rsp = ALLOC_N(unsigned char, *lr);  /* Response */
#if DEBUG
    debug_command(cmd, lc);
#endif
    try(CT_data(Ctn, &dad, &sad, lc, cmd, lr, rsp));
#if DEBUG
    debug_command(rsp, *lr);
#endif
    return (char *) rsp;
}

/*
 * We wrap CT_close to get the exceptions right.
 */
void ct_close(unsigned short Ctn) {
    try(CT_close(Ctn));
}

/*
 * A wrapper function for the CT API CT_init(ctn, pn) function. The first
 * argument is a cardterminal number, the second argument is an identifier for
 * the interface as a portnumber. See ctapi(3) for more information.
 */
static VALUE
ctapi_ct_init(int argc, VALUE *argv, VALUE self) {
    unsigned short arg1 ;
    unsigned short arg2 ;

    if ((argc < 2) || (argc > 2))
            rb_raise(rb_eArgError, "wrong # of arguments(%d for 2)", argc);
    arg1 = NUM2USHRT(argv[0]);
    arg2 = NUM2USHRT(argv[1]);
    ct_init(arg1, arg2);

    return Qnil;
}

/*
 * A wrapper function for the CT API CT_data function. This one is
 * much nicer than the original CT_data because is has fewer arguments:
 * ct_data(ctn, dad, sad, cmd) where ctn is the cardterminal number,
 * dad is the destination address number, sad is the source address number and
 * cmd is the command as a String. See ctapi(3) for more information.
 */
static VALUE
ctapi_ct_data(int argc, VALUE *argv, VALUE self) {
    unsigned short arg1 ;
    unsigned int arg2 ;
    unsigned int arg3 ;
    char *arg4 ;
    unsigned short length;
    char *result;
    unsigned short lr;
    VALUE vresult = Qnil;

    if ((argc < 4) || (argc > 4))
            rb_raise(rb_eArgError, "wrong # of arguments(%d for 4)", argc);
    arg1 = NUM2USHRT(argv[0]);
    arg2 = NUM2UINT(argv[1]);
    arg3 = NUM2UINT(argv[2]);
    arg4 = RSTRING(argv[3])->ptr;
    length = RSTRING(argv[3])->len;
    result = (char *) ct_data(arg1, arg2, arg3, arg4, length, &lr);
    vresult = rb_str_new(result, lr);
    ruby_xfree(result);
    return vresult;
}

/*
 * A wrapper function for the CT API CT_close(ctn) function. It takes a
 * cardterminal number as the only argument. See ctapi(3) for more information.
 */
static VALUE
ctapi_ct_close(int argc, VALUE *argv, VALUE self) {
    unsigned short arg1 ;

    if ((argc < 1) || (argc > 1))
            rb_raise(rb_eArgError, "wrong # of arguments(%d for 1)",argc);
    arg1 = NUM2USHRT(argv[0]);
    ct_close(arg1);

    return Qnil;
}

void Init_ctapicore(void) {
    mCTAPICore = rb_define_module("CTAPICore");
    rb_define_const(mCTAPICore, "CTBCS_MIN_COMMAND_SIZE", INT2NUM(2));
    rb_define_const(mCTAPICore, "CTBCS_MIN_RESPONSE_SIZE", INT2NUM(2));
    rb_define_const(mCTAPICore, "CTBCS_CLA", INT2NUM(0x20));
    rb_define_const(mCTAPICore, "CTBCS_INS_RESET", INT2NUM(0x11));
    rb_define_const(mCTAPICore, "CTBCS_INS_REQUEST", INT2NUM(0x12));
    rb_define_const(mCTAPICore, "CTBCS_INS_STATUS", INT2NUM(0x13));
    rb_define_const(mCTAPICore, "CTBCS_INS_EJECT", INT2NUM(0x15));
    rb_define_const(mCTAPICore, "CTBCS_P1_CT_KERNEL", INT2NUM(0x00));
    rb_define_const(mCTAPICore, "CTBCS_P1_INTERFACE1", INT2NUM(0x01));
    rb_define_const(mCTAPICore, "CTBCS_P1_INTERFACE2", INT2NUM(0x02));
    rb_define_const(mCTAPICore, "CTBCS_P1_INTERFACE3", INT2NUM(0x03));
    rb_define_const(mCTAPICore, "CTBCS_P1_INTERFACE4", INT2NUM(0x04));
    rb_define_const(mCTAPICore, "CTBCS_P1_INTERFACE5", INT2NUM(0x05));
    rb_define_const(mCTAPICore, "CTBCS_P1_INTERFACE6", INT2NUM(0x06));
    rb_define_const(mCTAPICore, "CTBCS_P1_INTERFACE7", INT2NUM(0x07));
    rb_define_const(mCTAPICore, "CTBCS_P1_INTERFACE8", INT2NUM(0x08));
    rb_define_const(mCTAPICore, "CTBCS_P1_INTERFACE9", INT2NUM(0x09));
    rb_define_const(mCTAPICore, "CTBCS_P1_INTERFACE10", INT2NUM(0x0A));
    rb_define_const(mCTAPICore, "CTBCS_P1_INTERFACE11", INT2NUM(0x0B));
    rb_define_const(mCTAPICore, "CTBCS_P1_INTERFACE12", INT2NUM(0x0C));
    rb_define_const(mCTAPICore, "CTBCS_P1_INTERFACE13", INT2NUM(0x0D));
    rb_define_const(mCTAPICore, "CTBCS_P1_INTERFACE14", INT2NUM(0x0E));
    rb_define_const(mCTAPICore, "CTBCS_P1_DISPLAY", INT2NUM(0x40));
    rb_define_const(mCTAPICore, "CTBCS_P1_KEYPAD", INT2NUM(0x50));
    rb_define_const(mCTAPICore, "CTBCS_P2_RESET_NO_RESP", INT2NUM(0x00));
    rb_define_const(mCTAPICore, "CTBCS_P2_RESET_GET_ATR", INT2NUM(0x01));
    rb_define_const(mCTAPICore, "CTBCS_P2_RESET_GET_HIST", INT2NUM(0x02));
    rb_define_const(mCTAPICore, "CTBCS_P2_REQUEST_NO_RESP", INT2NUM(0x00));
    rb_define_const(mCTAPICore, "CTBCS_P2_REQUEST_GET_ATR", INT2NUM(0x01));
    rb_define_const(mCTAPICore, "CTBCS_P2_REQUEST_GET_HIST", INT2NUM(0x02));
    rb_define_const(mCTAPICore, "CTBCS_P2_STATUS_MANUFACTURER", INT2NUM(0x46));
    rb_define_const(mCTAPICore, "CTBCS_P2_STATUS_ICC", INT2NUM(0x80));
    rb_define_const(mCTAPICore, "CTBCS_SW1_OK", INT2NUM(0x90));
    rb_define_const(mCTAPICore, "CTBCS_SW2_OK", INT2NUM(0x00));
    rb_define_const(mCTAPICore, "CTBCS_SW1_WRONG_LENGTH", INT2NUM(0x67));
    rb_define_const(mCTAPICore, "CTBCS_SW2_WRONG_LENGTH", INT2NUM(0x00));
    rb_define_const(mCTAPICore, "CTBCS_SW1_COMMAND_NOT_ALLOWED", INT2NUM(0x69));
    rb_define_const(mCTAPICore, "CTBCS_SW2_COMMAND_NOT_ALLOWED", INT2NUM(0x00));
    rb_define_const(mCTAPICore, "CTBCS_SW1_WRONG_PARAM", INT2NUM(0x6A));
    rb_define_const(mCTAPICore, "CTBCS_SW2_WRONG_PARAM", INT2NUM(0x00));
    rb_define_const(mCTAPICore, "CTBCS_SW1_WRONG_INS", INT2NUM(0x6D));
    rb_define_const(mCTAPICore, "CTBCS_SW2_WRONG_INS", INT2NUM(0x00));
    rb_define_const(mCTAPICore, "CTBCS_SW1_WRONG_CLA", INT2NUM(0x6E));
    rb_define_const(mCTAPICore, "CTBCS_SW2_WRONG_CLA", INT2NUM(0x00));
    rb_define_const(mCTAPICore, "CTBCS_SW1_ICC_ERROR", INT2NUM(0x6F));
    rb_define_const(mCTAPICore, "CTBCS_SW2_ICC_ERROR", INT2NUM(0x00));
    rb_define_const(mCTAPICore, "CTBCS_SW1_RESET_CT_OK", INT2NUM(0x90));
    rb_define_const(mCTAPICore, "CTBCS_SW2_RESET_CT_OK", INT2NUM(0x00));
    rb_define_const(mCTAPICore, "CTBCS_SW1_RESET_SYNC_OK", INT2NUM(0x90));
    rb_define_const(mCTAPICore, "CTBCS_SW2_RESET_SYNC_OK", INT2NUM(0x00));
    rb_define_const(mCTAPICore, "CTBCS_SW1_RESET_ASYNC_OK", INT2NUM(0x90));
    rb_define_const(mCTAPICore, "CTBCS_SW2_RESET_ASYNC_OK", INT2NUM(0x01));
    rb_define_const(mCTAPICore, "CTBCS_SW1_RESET_ERROR", INT2NUM(0x64));
    rb_define_const(mCTAPICore, "CTBCS_SW2_RESET_ERROR", INT2NUM(0x00));
    rb_define_const(mCTAPICore, "CTBCS_SW1_REQUEST_SYNC_OK", INT2NUM(0x90));
    rb_define_const(mCTAPICore, "CTBCS_SW2_REQUEST_SYNC_OK", INT2NUM(0x00));
    rb_define_const(mCTAPICore, "CTBCS_SW1_REQUEST_ASYNC_OK", INT2NUM(0x90));
    rb_define_const(mCTAPICore, "CTBCS_SW2_REQUEST_ASYNC_OK", INT2NUM(0x01));
    rb_define_const(mCTAPICore, "CTBCS_SW1_REQUEST_NO_CARD", INT2NUM(0x62));
    rb_define_const(mCTAPICore, "CTBCS_SW2_REQUEST_NO_CARD", INT2NUM(0x00));
    rb_define_const(mCTAPICore, "CTBCS_SW1_REQUEST_CARD_PRESENT", INT2NUM(0x62));
    rb_define_const(mCTAPICore, "CTBCS_SW2_REQUEST_CARD_PRESENT", INT2NUM(0x01));
    rb_define_const(mCTAPICore, "CTBCS_SW1_REQUEST_ERROR", INT2NUM(0x64));
    rb_define_const(mCTAPICore, "CTBCS_SW2_REQUEST_ERROR", INT2NUM(0x00));
    rb_define_const(mCTAPICore, "CTBCS_SW1_REQUEST_TIMER_ERROR", INT2NUM(0x69));
    rb_define_const(mCTAPICore, "CTBCS_SW2_REQUEST_TIMER_ERROR", INT2NUM(0x00));
    rb_define_const(mCTAPICore, "CTBCS_SW1_EJECT_OK", INT2NUM(0x90));
    rb_define_const(mCTAPICore, "CTBCS_SW2_EJECT_OK", INT2NUM(0x00));
    rb_define_const(mCTAPICore, "CTBCS_SW1_EJECT_REMOVED", INT2NUM(0x90));
    rb_define_const(mCTAPICore, "CTBCS_SW2_EJECT_REMOVED", INT2NUM(0x01));
    rb_define_const(mCTAPICore, "CTBCS_SW1_EJECT_NOT_REMOVED", INT2NUM(0x62));
    rb_define_const(mCTAPICore, "CTBCS_SW2_EJECT_NOT_REMOVED", INT2NUM(0x00));
    rb_define_const(mCTAPICore, "CTBCS_DATA_STATUS_NOCARD", INT2NUM(0x00));
    rb_define_const(mCTAPICore, "CTBCS_DATA_STATUS_CARD", INT2NUM(0x01));
    rb_define_const(mCTAPICore, "CTBCS_DATA_STATUS_CARD_CONNECT", INT2NUM(0x05));
    rb_define_const(mCTAPICore, "MAX_APDULEN", INT2NUM(MAX_APDULEN));
    rb_define_const(mCTAPICore, "OK", INT2NUM(0));
    rb_define_const(mCTAPICore, "ERR_INVALID", INT2NUM(-1));
    rb_define_const(mCTAPICore, "ERR_CT", INT2NUM(-8));
    rb_define_const(mCTAPICore, "ERR_TRANS", INT2NUM(-10));
    rb_define_const(mCTAPICore, "ERR_MEMORY", INT2NUM(-11));
    rb_define_const(mCTAPICore, "ERR_HTSI", INT2NUM(-128));
    /*
     * Document-class: CTAPICore::CTAPIError
     *
     * All error exceptions in CTAPICore are of class CTAPIError which is a
     * child of StandardError.
     */
    eCTAPIError =
        rb_define_class_under(mCTAPICore, "CTAPIError", rb_eStandardError);
    /*
     * Document-class: CTAPICore::InvalidError
     *
     * This exception is raised if an invalid parameter was used for
     * CTAPICore function.
     */
    eInvalidError =
        rb_define_class_under(mCTAPICore, "InvalidError", eCTAPIError);
    /*
     * Document-class: CTAPICore::CardterminalError
     *
     * This exception is raised if the cardterminal is  temporarily  not
     * accessible (busy with   other  or  internal processes).  The problem can
     * be solved by the application. Just retry.
     */
    eCardterminalError =
        rb_define_class_under(mCTAPICore, "CardterminalError", eCTAPIError);
    /*
     * Document-class: CTAPICore::TransmissionError
     *
     * This exception is raised if a mechanical, electrical or protocol
     * failures. Reset of the cardterminal is necessary.
     */
    eTransmissionError =
        rb_define_class_under(mCTAPICore, "TransmissionError", eCTAPIError);
    /*
     * Document-class: CTAPICore::MemoryError
     *
     * This exception is raised if a memory error occurred (f.i. the
     * allocated buffer is too small for the returned data). This should
     * NOT happen. If it does, it's a bug. Please tell me.
     */
    eMemoryError =
        rb_define_class_under(mCTAPICore, "MemoryError", eCTAPIError);
    /*
     * Document-class: CTAPICore::HTSIError
     *
     * This exception (HTSI = Host Transport Service Interface) is raised if
     * the  error  is  produced by the software layer and not in the
     * communication with the hardware.
     */
    eHTSIError =
        rb_define_class_under(mCTAPICore, "HTSIError", eCTAPIError);
    /*
     * Document-class: CTAPICore::UnknownError
     *
     * This exception is raised if an unkown error occurrs. This also should
     * NOT happen. If it does, it's a bug. Please tell me.
     */
    eUnknownError =
        rb_define_class_under(mCTAPICore, "UnknownError", eCTAPIError);

    rb_define_const(mCTAPICore, "PORT_COM1", INT2NUM(0));
    rb_define_const(mCTAPICore, "PORT_COM2", INT2NUM(1));
    rb_define_const(mCTAPICore, "PORT_COM3", INT2NUM(2));
    rb_define_const(mCTAPICore, "PORT_COM4", INT2NUM(3));
    rb_define_const(mCTAPICore, "PORT_Printer", INT2NUM(4));
    rb_define_const(mCTAPICore, "PORT_Modem", INT2NUM(5));
    rb_define_const(mCTAPICore, "PORT_LPT1", INT2NUM(6));
    rb_define_const(mCTAPICore, "PORT_LPT2", INT2NUM(7));
    rb_define_const(mCTAPICore, "CT", INT2NUM(1));
    rb_define_const(mCTAPICore, "HOST", INT2NUM(2));
    rb_define_const(mCTAPICore, "ICC1", INT2NUM(0));
    rb_define_const(mCTAPICore, "ICC2", INT2NUM(2));
    rb_define_const(mCTAPICore, "ICC3", INT2NUM(3));
    rb_define_const(mCTAPICore, "ICC4", INT2NUM(4));
    rb_define_const(mCTAPICore, "ICC5", INT2NUM(5));
    rb_define_const(mCTAPICore, "ICC6", INT2NUM(6));
    rb_define_const(mCTAPICore, "ICC7", INT2NUM(7));
    rb_define_const(mCTAPICore, "ICC8", INT2NUM(8));
    rb_define_const(mCTAPICore, "ICC9", INT2NUM(9));
    rb_define_const(mCTAPICore, "ICC10", INT2NUM(10));
    rb_define_const(mCTAPICore, "ICC11", INT2NUM(11));
    rb_define_const(mCTAPICore, "ICC12", INT2NUM(12));
    rb_define_const(mCTAPICore, "ICC13", INT2NUM(13));
    rb_define_const(mCTAPICore, "ICC14", INT2NUM(14));
    rb_define_module_function(mCTAPICore, "ct_init", ctapi_ct_init, -1);
    rb_define_module_function(mCTAPICore, "ct_data", ctapi_ct_data, -1);
    rb_define_module_function(mCTAPICore, "ct_close", ctapi_ct_close, -1);
}
/* vim: set et cin sw=4 ts=4: */
