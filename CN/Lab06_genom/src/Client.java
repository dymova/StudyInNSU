import java.net.InetAddress;
import java.net.UnknownHostException;

public class Client {

    private static final String USAGE = "Usage: java Client <serverAddress> <serverPort>";

    public static void main(String[] args) throws UnknownHostException {
        if(args.length != 2) {
            throw new IllegalArgumentException(USAGE);
        }

        InetAddress serverAddress = InetAddress.getByName(args[0]);
        int serverPort = Integer.parseInt(args[1]);

        Thread clientThread = new Thread(new ClientThread(serverAddress, serverPort), "Client");
        clientThread.start();
    }
}
