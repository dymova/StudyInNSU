import java.net.InetAddress;
import java.net.UnknownHostException;

public class Main {
    public static final String USAGE = "java Main <serverAddress> <serverPort>";

    public static void main(String[] args) throws UnknownHostException {
        if(args.length != 2) {
            throw new IllegalArgumentException(USAGE);
        }
        InetAddress address = InetAddress.getByName(args[0]);
        int port = Integer.parseInt(args[1]);

        Pop3Receiver receiver = new Pop3Receiver(address, port);
        receiver.start();
    }
}
