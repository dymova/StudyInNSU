import java.io.*;
import java.net.Socket;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;

public class Connection implements Runnable {


    public static final int PORT = 80;
    public static final int BUF_SIZE = 1024;
    private final InputStream clientInput;
    private final OutputStream clientOutput;

    public Connection(Socket socket) throws IOException {
        clientInput = socket.getInputStream();
        clientOutput = socket.getOutputStream();
    }

    @Override
    public void run() {
        try {
            Socket serverSocket = handleRequest();
            if(serverSocket != null) {
                handelAnswer(serverSocket);
            }
        } catch (IOException | URISyntaxException e) {
            e.printStackTrace();
        }
    }

    private Socket handleRequest() throws IOException, URISyntaxException {
        Socket serverSocket;
        BufferedReader clientReader = new BufferedReader(new InputStreamReader(clientInput));
            String readLine = clientReader.readLine();
            if(readLine == null) {
                return null;
            }
            System.out.println("readLine:" + readLine);
            if(!readLine.contains("GET")) {
                return null;
            }

            String[] tokens = readLine.split(" ");
            URL url = new URL(tokens[1]);
            System.out.println("url:" + url);

            serverSocket = new Socket(url.getHost(), PORT);

            OutputStream serverOutput = serverSocket.getOutputStream();

            System.out.println(">>" + readLine );
            serverOutput.write((readLine + "\n\n").getBytes());

        return serverSocket;
    }

    private void handelAnswer(Socket serverSocket) throws IOException {
        byte[] buf = new byte[BUF_SIZE];
        InputStream serverInput = serverSocket.getInputStream();

        int bytesReadNumber;
        while (-1 != (bytesReadNumber = serverInput.read(buf))) {
            System.out.println("<<" + new String(buf));
            clientOutput.write(buf, 0, bytesReadNumber);
        }

        System.out.println("full page");
    }
}
