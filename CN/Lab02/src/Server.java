import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;

public class Server {
    private static final String USAGE = "Usage: java Server <port>";
    private static final int BUFSIZE = 512;
    private static final String PONG = "pong";


    public static void main(String[] args) {
        if (args.length != 1) {
            throw new IllegalArgumentException(USAGE);
        }

        int port = Integer.parseInt(args[0]);


        try (DatagramSocket socket = new DatagramSocket(port)) {
            System.out.println("Server starting on " + port + " port...\n");

            byte[] receiveData = new byte[BUFSIZE];
            DatagramPacket packet = new DatagramPacket(receiveData, receiveData.length);

            for (; ; ) {
                socket.receive(packet);
                byte[] sendData = PONG.getBytes();
                DatagramPacket sendPacket = new DatagramPacket(sendData,
                        sendData.length, packet.getAddress(), packet.getPort());
                socket.send(sendPacket);
                System.out.println("send answer");
            }
        } catch (IOException e) {
            e.printStackTrace();
        }

    }
}
