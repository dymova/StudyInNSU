import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

public class HttpServer implements Runnable{
    private final int port;
    private final String defaultDir;

    public HttpServer(int port, String defaultDir) {

        this.port = port;
        this.defaultDir = defaultDir;
    }

    @Override
    public void run() {
        try(ServerSocket serverSocket = new ServerSocket(port)){
            while (true) {
                Socket socket = serverSocket.accept();
                System.out.println("Socket accept");
                new Thread(new Connection(socket.getInputStream(), socket.getOutputStream(), defaultDir)).start();
            }

        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
