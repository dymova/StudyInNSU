package radio.server;

import java.io.IOException;
import java.net.UnknownHostException;

public class ServerMain {
    private static final String USAGE = "Usage: java radio.server.ServerMain <serverPort> <pathToSrc>";

    public static void main(String[] args) throws IOException {
        if(args.length != 2) {
            throw new IllegalArgumentException(USAGE);
        }

        int port = Integer.parseInt(args[0]);
        new Server(port, args[1]).start();
    }

}
