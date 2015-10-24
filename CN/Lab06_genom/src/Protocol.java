public class Protocol {
    public static final int BUF_SIZE = 1024;
    public static final int PREFIX_SIZE = 5;
    public static final int INT_SIZE = 4;
    public static final byte FIST_REQUEST_TYPE = 0;
    public static final byte HASH_ANSWER_TYPE = -1;
    public static final byte DIAPASON_REQUEST_TYPE = 1;
    public static final byte LAST_DIAPASON = 15; //00001111
    public static final byte DIAPASONS_ENDED_TYPE = -3;
    public static final byte NEW_DIAPASON_TYPE = -2;
    public static final byte RESULT_TYPE = 3;

    public static final int SYMBOL_IN_BYTE = 4;

    public static final byte A = 0;
    public static final byte C = 1;
    public static final byte G = 2;
    public static final byte T = 3;

    public static char byteToChar(byte b) throws UnknownProtocolException {
        switch (b) {
            case A:
                return 'a';
            case C:
                return 'c';
            case G:
                return 'g';
            case T:
                return 't';
        }
        throw new UnknownProtocolException("char to byte:" + b);
    }


}
