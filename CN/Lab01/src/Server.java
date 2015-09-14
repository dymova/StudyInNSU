import java.io.*;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.util.Scanner;

public class Server {
    private static final String  ENDING_MESSAGE = "End of file with quotes.\n";
    private static final String USAGE = "Usage: java Server <port>";
    private static final int BUFSIZE = 1024;


    public static void main(String[] args) {
        if(args.length != 1) {
            throw new IllegalArgumentException(USAGE);
        }

        int port = Integer.parseInt(args[0]);

        System.out.println("Please, enter path to file with quotes:");
        Scanner scanner = new Scanner(System.in);
        String pathToFile = scanner.nextLine();

        System.out.println("Server starting on " + port + " port...\n");

        try (BufferedReader reader = new BufferedReader(new FileReader(pathToFile));
             DatagramSocket socket = new DatagramSocket(port)) {

            byte[] receiveData = new byte[BUFSIZE];
            DatagramPacket receivedPacket = new DatagramPacket(receiveData, receiveData.length);

            for(;;) {
                socket.receive(receivedPacket);

                String message = reader.readLine();
                if (message == null) {
                    message = ENDING_MESSAGE;
                    byte[] sendData = message.getBytes();
                    DatagramPacket sendPacket = new DatagramPacket(sendData,
                            sendData.length, receivedPacket.getAddress(), receivedPacket.getPort());
                    socket.send(sendPacket);
                    break;
                }
                byte[] sendData = message.getBytes();
                DatagramPacket sendPacket = new DatagramPacket(sendData,
                        sendData.length, receivedPacket.getAddress(), receivedPacket.getPort());
                socket.send(sendPacket);

                System.out.println(receivedPacket.getAddress() + ":v" + message);
            }


        } catch (IOException e) {
            e.printStackTrace();
        }


    }
}
