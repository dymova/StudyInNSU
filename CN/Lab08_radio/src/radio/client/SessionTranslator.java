package radio.client;

import radio.Protocol;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.InetAddress;
import java.net.MulticastSocket;

public class SessionTranslator implements Runnable {
    private final int port;
    private final InetAddress address;

    public SessionTranslator(int port, InetAddress address) {
        this.port = port;
        this.address = address;
    }

    @Override
    public void run() {
        try (MulticastSocket socket = new MulticastSocket(port)) {
            socket.joinGroup(address);

            DatagramPacket packet;
            while (!Thread.currentThread().isInterrupted()) {
                try {
                    byte[] buf = new byte[Protocol.BUFF_SIZE];
                    packet = new DatagramPacket(buf, buf.length);
                    socket.receive(packet);

                    String message = new String(packet.getData());
                    System.out.println(message);
                } catch (IOException e) {
                    e.printStackTrace();
                    Thread.currentThread().interrupt();
                }

            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
