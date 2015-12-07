import java.net.*;

public class Main {
    private static final String USAGE = "Usage: java Main <port> <parent_address> <parent_port>";

    public static void main(String[] args) throws UnknownHostException {
        if(args.length != 3 && args.length != 1) {
            throw new IllegalArgumentException(USAGE);
        }

        Node node = new Node(args);
        node.run();
    }
}
