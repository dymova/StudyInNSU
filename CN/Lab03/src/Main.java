import java.io.IOException;
import java.net.*;
import java.util.HashSet;

public class Main {
    private static final String USAGE = "Usage: java Main <port> <broadcastAddress>";
    private static final int BUFSIZE = 512;
    private static final String STARTING_MESSAGE = "IBORN";
    private static final String FINAL_MESSAGE = "IEXIT";
    private static final String REQUEST_ANSWER = "ILIVE";


    public static void main(String[] args) throws UnknownHostException {
        if (args.length != 2) {
            throw new IllegalArgumentException(USAGE);
        }
        int port = Integer.parseInt(args[0]);
        InetAddress broadcastAddress = InetAddress.getByName(args[1]);

        try(DatagramSocket socket = new DatagramSocket(port)) {
            socket.setBroadcast(true);

            Runtime.getRuntime().addShutdownHook(new Thread() {
                public void run() {
                    try {
                        byte[] finalMessageData = FINAL_MESSAGE.getBytes();
                        DatagramPacket finalPacket = new DatagramPacket(finalMessageData,
                                finalMessageData.length, broadcastAddress, port);
                        socket.send(finalPacket);
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            });

            byte[] startingMessageData = STARTING_MESSAGE.getBytes();
            DatagramPacket startingPacket = new DatagramPacket(startingMessageData,
                    startingMessageData.length, broadcastAddress, port);
            socket.send(startingPacket);

            byte[] receiveData = new byte[BUFSIZE];
            DatagramPacket receivePacket = new DatagramPacket(receiveData, receiveData.length);

            HashSet<InetAddress> copies = new HashSet<>();

            for (;;) {
                socket.receive(receivePacket);
                String receiveMessage = new String(receivePacket.getData(), 0, receivePacket.getLength());
                if (receiveMessage.equals(REQUEST_ANSWER)) {
                    copies.add(receivePacket.getAddress());
                } else if (receiveMessage.equals(STARTING_MESSAGE)){
                    copies.add(receivePacket.getAddress());
                    byte[] responseMessageData = REQUEST_ANSWER.getBytes();
                    DatagramPacket responsePacket = new DatagramPacket(responseMessageData,
                            responseMessageData.length, broadcastAddress, port);
                    socket.send(responsePacket);
                } else if (receiveMessage.equals(FINAL_MESSAGE)) {
                    copies.remove(receivePacket.getAddress());
                } else {
                    continue;
                }
                System.out.println(receiveMessage);
                System.out.println("Current copies:");
                for (InetAddress copy : copies) {
                    System.out.println(copy);
                }
                System.out.println("Total: " + copies.size());
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
