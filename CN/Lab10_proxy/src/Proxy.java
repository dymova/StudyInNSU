import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

public class Proxy {
    private final int port;

    public Proxy(int port) {

        this.port = port;
    }

    public void start() {
        while (true) {
            try (ServerSocket serverSocket = new ServerSocket(port)){
                Socket socket = serverSocket.accept();
                new Thread(new Connection(socket), "Connection").start();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
}
