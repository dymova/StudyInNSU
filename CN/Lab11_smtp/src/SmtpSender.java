import java.io.*;
import java.net.InetAddress;
import java.net.Socket;
import java.util.Scanner;


public class SmtpSender {
    public static final String HELO = "HELO ";
    public static final String MAIL_FROM = "MAIL FROM: ";
    public static final String RCPT_TO = "RCPT TO: ";
    private static final String SUBJECT = "SUBJECT: ";
    private static final String MESSAGE_ENTRY = "MESSAGE: ";
    private static final String DATA_MESSAGE = "DATA\n";
    private static final String QUIT_MESSAGE = "QUIT\n";
    public static final String END_LINE = "\n";
    private static final String DELIMITER = " ";
    private static final String END_LINE_SYMBOL = ".";
    private static final int SERVICE_READY = 220;
    private static final int OK = 250;
    private static final int START_MAIL_INPUT = 354;

    private final InetAddress serverAddress;
    private final int serverPort;
    private BufferedReader reader;
    private OutputStream writer;
    private Scanner scanner;

    public SmtpSender(InetAddress address, int port) {
        serverAddress = address;
        serverPort = port;
    }


    public void start() throws IOException {
        try (Socket socket = new Socket(serverAddress, serverPort)) {
            reader = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            writer = socket.getOutputStream();
            scanner = new Scanner(System.in);

            if (handleHeloCommand()) {
                if (handleMailFromCommand()) {
                    if (handleRcptTo()) {
                        if (handleDataCommand()) {
                            if (handleSubjectCommand())
                                if (handleMessageEntry()) {
                                    handleQuitCommand();
                                }
                        }
                    }
                }
            }


        }
    }

    private int getAnswerCode(String connectAnswer) {
        return Integer.parseInt(connectAnswer.split(DELIMITER)[0]);
    }

    private void handleQuitCommand() throws IOException {
        String serverAnswer = reader.readLine();
        System.out.println(serverAnswer);

        if (getAnswerCode(serverAnswer) == OK) {
            writer.write(QUIT_MESSAGE.getBytes());
            System.out.println(QUIT_MESSAGE);
        }
    }

    private boolean handleMessageEntry() throws IOException {
        System.out.print(MESSAGE_ENTRY);
        String line;
        do {
            line = scanner.nextLine();
            writer.write((line + END_LINE).getBytes());

        } while (!line.equals(END_LINE_SYMBOL));
        return true;
    }


    private boolean handleSubjectCommand() throws IOException {
        String serverAnswer = reader.readLine();
        System.out.println(serverAnswer);

        if (getAnswerCode(serverAnswer) == START_MAIL_INPUT) {
            System.out.print(SUBJECT);
            String subject = scanner.nextLine();
            writer.write((SUBJECT + subject + END_LINE + END_LINE).getBytes());
            return true;
        }
        return false;
    }


    private boolean handleDataCommand() throws IOException {
        String serverAnswer = reader.readLine();
        System.out.println(serverAnswer);

        if (getAnswerCode(serverAnswer) == OK) {
            System.out.println(DATA_MESSAGE);
            writer.write(DATA_MESSAGE.getBytes());
            return true;
        }
        return false;
    }


    private boolean handleRcptTo() throws IOException {
        String serverAnswer = reader.readLine();
        System.out.println(serverAnswer);

        if (getAnswerCode(serverAnswer) == OK) {
            System.out.print(RCPT_TO);
            String toAddress = scanner.nextLine();
            writer.write((RCPT_TO + toAddress + END_LINE).getBytes());
            return true;
        }
        return false;
    }

    public boolean handleMailFromCommand() throws IOException {
        String serverAnswer = reader.readLine();
        System.out.println(serverAnswer);

        if (getAnswerCode(serverAnswer) == OK) {
            System.out.print(MAIL_FROM);
            String fromAddress = scanner.nextLine();
            writer.write((MAIL_FROM + fromAddress + END_LINE).getBytes());
            return true;
        }
        return false;
    }


    public boolean handleHeloCommand() throws IOException {
        String serverAnswer = reader.readLine();
        System.out.println(serverAnswer);

        if (getAnswerCode(serverAnswer) == SERVICE_READY) {
            System.out.print(HELO);
            String ip = scanner.nextLine();
            writer.write((HELO + ip + END_LINE).getBytes());
            return true;
        }
        return false;
    }
}
