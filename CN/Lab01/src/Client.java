import java.io.IOException;
import java.net.*;
import java.util.Scanner;

public class Client {
    private static final String  ENDING_MESSAGE = "End of file with quotes.\n";
    private static final String USAGE = "Usage: java Client <ipAddress> <port>";
    private static final int BUFSIZE = 1024;
    private static final int TIME_OUT = 10000;


    public static void main(String[] args) {
        if(args.length != 2) {
            throw new IllegalArgumentException(USAGE);
        }

        String userName = "";
        while (userName.length() == 0) {
            System.out.println("Please, enter your name:");
            Scanner scanner = new Scanner(System.in);
            userName = scanner.nextLine();
        }

        String ipAddress = args[0];
        int port = Integer.parseInt(args[1]);



        byte[] receiveData = new byte[BUFSIZE];
        try(DatagramSocket socket = new DatagramSocket()){
            socket.setSoTimeout(TIME_OUT);
            InetAddress inetAddress = InetAddress.getByName(ipAddress);

            DatagramPacket packet = new DatagramPacket(userName.getBytes(), userName.getBytes().length,inetAddress, port);
            socket.send(packet);

            DatagramPacket receivedPacket = new DatagramPacket(receiveData, receiveData.length);
            socket.receive(receivedPacket);
            System.out.println(new String(receiveData));
        } catch (IOException e) {
            e.printStackTrace();
        }

    }
}
