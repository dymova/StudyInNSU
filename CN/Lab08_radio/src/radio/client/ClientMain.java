package radio.client;

import radio.UnknownProtocolException;
import radio.client.Client;

import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;

public class ClientMain {
    private static final String USAGE = "Usage: java radio.client.ClientMain <serverAddress> <serverPort>";

    public static void main(String[] args) {
        if(args.length != 2) {
            throw new IllegalArgumentException(USAGE);
        }
        try {
            InetAddress inetAddress = InetAddress.getByName(args[0]);
            int serverPort = Integer.parseInt(args[1]);
            Client client = new Client(inetAddress, serverPort);
            client.start();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
