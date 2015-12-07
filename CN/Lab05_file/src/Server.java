import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;

public class Server {
    private static final String USAGE = "Usage: java Server <port>";
    public static final String OUTPUT_FILE_NAME = "/home/nastya/StudyInNSU/CN/Lab05_file/src/out.txt";

    public static void main(String[] args) {
        if (args.length != 1) {
            throw new IllegalArgumentException(USAGE);
        }
        int port = Integer.parseInt(args[0]);

        try (ServerSocket serverSocket = new ServerSocket(port)) {
            System.out.println("Server starting in " + port + " port.");
            try (Socket socket = serverSocket.accept()) {
                System.out.println("Connection establish");
                try (DataInputStream reader = new DataInputStream(socket.getInputStream())) {
                    byte[] buffer = new byte[1024];
                    String filename = reader.readUTF();
                    try (FileOutputStream writer = new FileOutputStream("/home/nastya/" + filename)) {
                        int read;
                        while (-1 != (read = reader.read(buffer))) {
                            writer.write(buffer, 0, read);
                        }
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
