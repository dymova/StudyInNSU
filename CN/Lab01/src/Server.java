import java.io.*;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;

public class Server {
    private static final String ENDING_MESSAGE = "End of file with quotes.\n";
    private static final String USAGE = "Usage: java Server <port>";
    private static final int BUFSIZE = 1024;


    public static void main(String[] args) {
        if (args.length != 1) {
            throw new IllegalArgumentException(USAGE);
        }

        int port = Integer.parseInt(args[0]);

        System.out.println("Please, enter path to file with quotes:");
        Scanner scanner = new Scanner(System.in);
        String pathToFile = scanner.nextLine();

        System.out.println("Server starting on " + port + " port...\n");

        try (BufferedReader reader = new BufferedReader(new FileReader(pathToFile));
             DatagramSocket socket = new DatagramSocket(port)) {


            List<String> quotes = new ArrayList<>();
            String message = reader.readLine();

            while (message != null) {
                quotes.add(message);
                message = reader.readLine();
            }

            for (String str : quotes) {
                byte[] receiveData = new byte[BUFSIZE];
                DatagramPacket receivedPacket = new DatagramPacket(receiveData, receiveData.length);
                socket.receive(receivedPacket);

                byte[] sendData = str.getBytes();
                DatagramPacket sendPacket = new DatagramPacket(sendData,
                        sendData.length, receivedPacket.getAddress(), receivedPacket.getPort());
                socket.send(sendPacket);
                System.out.println(receivedPacket.getAddress() + ": " + str);
            }


        } catch (IOException e) {
            e.printStackTrace();
        }


    }
}
