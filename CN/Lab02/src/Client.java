import java.io.IOException;
import java.net.*;

public class Client {
    private static final String USAGE = "Usage: java Client <ipAddress> <port> <messageCount>";
    private static final int BUFSIZE = 512;
    private static final String PING = "ping";
    private static final String PONG = "pong";
    private static final int TIME_OUT = 1000;

    public static void main(String[] args) throws UnknownHostException {
        if (args.length != 3) {
            throw new IllegalArgumentException(USAGE);
        }

        InetAddress inetAddress = InetAddress.getByName(args[0]);
        int port = Integer.parseInt(args[1]);
        int messageCount = Integer.parseInt(args[2]);

        try (DatagramSocket socket = new DatagramSocket()) {
            socket.setSoTimeout(TIME_OUT);

            byte[] sendData = PING.getBytes();
            byte[] receiveData = new byte[BUFSIZE];
            DatagramPacket receivePacket = new DatagramPacket(receiveData, receiveData.length);
            int lostPacketCount = 0;

            for (int i = 0; i < messageCount; i++) {
                DatagramPacket sendPacket = new DatagramPacket(sendData, sendData.length, inetAddress, port);
                socket.send(sendPacket);

                long timeSend = System.currentTimeMillis();

                try {
                    socket.receive(receivePacket);
                } catch (SocketTimeoutException e) {
                    lostPacketCount++;
                    System.out.println("Packet was lost.");
                }
                long timeReceive = System.currentTimeMillis();
                long delay = timeReceive - timeSend;
                System.out.println("delay=" + delay + " ms");
            }
            System.out.println(lostPacketCount + " packet loss.");

        } catch (IOException e) {
            e.printStackTrace();
        }

    }
}
