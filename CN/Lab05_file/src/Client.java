import java.io.*;
import java.net.Socket;
import java.nio.file.Path;
import java.nio.file.Paths;

public class Client {
    private static final String USAGE = "Usage: " +
            "java Client <serverAddress> <serverPort> + <pathToFile>";


    public static void main(String[] args) {
        if (args.length != 3) {
            throw new IllegalArgumentException(USAGE);
        }
        String address = args[0];
        int port = Integer.parseInt(args[1]);
        String pathToFile = args[2];

        Path path = Paths.get(pathToFile);

        try (Socket socket = new Socket(address, port)) {
            try (DataOutputStream writer = new DataOutputStream(socket.getOutputStream());
                 FileInputStream reader = new FileInputStream(pathToFile)) {
                byte[] buffer = new byte[1024];
                int read;
                writer.writeUTF(path.getFileName().toString());
                System.out.println(path.getFileName().toString());
                while (-1 != (read = reader.read(buffer))) {
                    writer.write(buffer, 0, read);
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }

    }
}