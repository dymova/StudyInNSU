import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.Socket;
import java.util.Scanner;

public class Pop3Receiver {
    private static final String DELIMITER = " ";
    public static final String ERROR_ANSWER = "-ERR";
    public static final String USER = "USER ";
    public static final String PASS = "PASS ";
    public static final String RETR = "RETR ";
    public static final String LIST = "LIST\n";
    public static final String QUIT = "QUIT\n";
    private static final String END_LINE_SYMBOL = ".";
    public static final String END_LINE = "\n";
    public static final String USER_COMMANDS = "Commands:\n" +
            "\t list - to get list of new massages;\n" +
            "\t show <messageNumber> - to show message with this number;\n" +
            "\t quit - to exit.";



    private final InetAddress serverAddress;
    private final int serverPort;
    private BufferedReader reader;
    private final Scanner scanner;
    private OutputStream writer;
    private boolean isStopped = false;

    public Pop3Receiver(InetAddress address, int port) {
        serverAddress = address;
        serverPort = port;
        scanner = new Scanner(System.in);
    }

    public void start() {
        try(Socket socket = new Socket(serverAddress, serverPort)) {
            reader = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            writer = socket.getOutputStream();

            handleServerAnswer();

            System.out.println("Please, enter your login:");
            String line = scanner.nextLine();
            writer.write((USER + line + END_LINE).getBytes());

            handleServerAnswer();

            System.out.println("Please, enter your password:");
            line = scanner.nextLine();
            writer.write((PASS + line + END_LINE).getBytes());

            System.out.println(USER_COMMANDS);
            while (!isStopped) {
                line = scanner.nextLine();
                String[] tokens = line.split(DELIMITER);
                if(1 < tokens.length && tokens.length < 2) {
                    continue;
                }
                switch (tokens[0]) {
                    case "list":
                        handleListCommand();
                        break;
                    case "show":
                        if(tokens.length != 2) {
                            System.out.println(USER_COMMANDS);
                        } else {
                            handleShowCommand(tokens[1]);
                        }
                        break;
                    case "quit":
                        handleQuitCommand();
                        break;
                    default:
                        System.out.println(USER_COMMANDS);
                }
            }


        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void handleQuitCommand() throws IOException {
        writer.write(QUIT.getBytes());
        handleServerAnswer();
        isStopped = true;
    }

    private void handleShowCommand(String number) throws IOException {
        writer.write((RETR + number + END_LINE).getBytes());
        handleServerAnswer();

        String line;
        while (!(line = reader.readLine()).equals(END_LINE_SYMBOL)) {
            System.out.println(line);
        }

    }

    private void handleListCommand() throws IOException {
        writer.write((LIST).getBytes());
        handleServerAnswer();

        String line;
        while (!(line = reader.readLine()).equals(END_LINE_SYMBOL)) {
            System.out.println(line);
        }
    }

    private void handleServerAnswer() throws IOException {
        String answer = reader.readLine();
        String[] tokens = answer.split(DELIMITER);
        if(tokens[0].equals(ERROR_ANSWER)) {
            throw new IOException("error server answer " + answer);
        }
    }
}
