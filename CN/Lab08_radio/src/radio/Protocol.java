package radio;

public class Protocol {
    public static final byte LIST_COMMAND_TYPE = 1;
    public static final byte LIST_ANSWER_TYPE = 2;
    public static final byte PLAY_COMMAND_TYPE = 3;
    public static final byte PLAY_ANSWER_TYPE = 4;
    public static final byte ABOUT_COMMAND_TYPE = 5;
    public static final byte ABOUT_ANSWER_TYPE = 6;
    public static final byte STATION_NOT_EXIST_TYPE = 7;
    public static final byte STATION_NOT_CHOSE = 8;
//    public static final byte LOGOUT


    public static final byte EOF = -1;
    public static final int INT_SIZE = 4;
    public static final int BYTE_SIZE = 1;
    public static final String SPLITTER = ":";
    public static final String LIST_COMMAND = "list";
    public static final String PLAY_COMMAND = "play";
    public static final String ABOUT_COMMAND = "about";
    public static final int BUFF_SIZE = 256;
//    public static final String MULTICAST_ADDRESS = "230.0.0.0";
    public static final String CONFIG = "config.txt";




}
