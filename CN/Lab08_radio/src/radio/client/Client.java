package radio.client;

import radio.Protocol;
import radio.UnknownProtocolException;

import java.io.DataInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.Socket;

public class Client {
    private final InetAddress serverAddress;
    private final int serverPort;
    private Socket socket;
    private Thread consoleHandler;
    private OutputStream outputStream;
    private InputStream inputStream;
    private DataInputStream dataInputStream;
    private Thread translator;

    public Client(InetAddress inetAddress, int serverPort) {
        serverAddress = inetAddress;
        this.serverPort = serverPort;
        consoleHandler = new Thread(new ConsoleMessageHandler(this), "ConsoleHandler");
    }

    void start() {
        try(Socket socket = new Socket(serverAddress,serverPort)) {
            this.socket = socket;
            outputStream = socket.getOutputStream();
            inputStream = socket.getInputStream();
            dataInputStream = new DataInputStream(inputStream);
            consoleHandler.start();

            while (true) {
                byte type = (byte) inputStream.read();
                switch (type) {
                    case Protocol.LIST_ANSWER_TYPE:
                        handleListAnswerMessage();
                        break;
                    case Protocol.PLAY_ANSWER_TYPE:
                        handlePlayAnswerMessage();
                        break;
                    case Protocol.ABOUT_ANSWER_TYPE:
                        handleAboutAnswerMessage();
                        break;
                    case Protocol.STATION_NOT_EXIST_TYPE:
                        System.out.println("This station not exist.");
                        break;
                    case Protocol.STATION_NOT_CHOSE:
                        System.out.println("Chose station for get info about it.");
                        break;
                    case Protocol.EOF:
                        throw new IOException();
                    default:
                        throw new UnknownProtocolException("answer type <" + type + ">");
                }
            }
        } catch (IOException | UnknownProtocolException e) {
            consoleHandler.interrupt();
            if (translator != null) {
                translator.interrupt();
            }
        }
    }

    private void handlePlayAnswerMessage() throws IOException {
        int size = dataInputStream.readInt();
        byte[] messageData = new byte[size];
        inputStream.read(messageData);
        String message = new String(messageData);
        String[] multicast = message.split(Protocol.SPLITTER);
        InetAddress sessionAddress = InetAddress.getByName(multicast[0]);
        int sessionPort = Integer.parseInt(multicast[1]);

        if(translator != null) {
            translator.interrupt();
        }

        System.out.println("address:" + sessionAddress+ " port: " + sessionPort);
        translator = new Thread(new SessionTranslator(sessionPort, sessionAddress), "Transator" + sessionAddress);
        translator.start();
    }

    private void handleAboutAnswerMessage() throws IOException {
        int size = dataInputStream.readInt();
        byte[] messageData = new byte[size];
        inputStream.read(messageData);
        String message = new String(messageData);
        String[] info = message.split(Protocol.SPLITTER);
        System.out.println("Info about current items:");
        for (String s : info) {
            System.out.println("\t" + s);
        }

    }

    private void handleListAnswerMessage() throws IOException {
        int size = dataInputStream.readInt();
        byte[] messageData = new byte[size];
        inputStream.read(messageData);
        String message = new String(messageData);
        String[] stations = message.split(Protocol.SPLITTER);
        System.out.print("Stations:");
        for (String station : stations) {
            System.out.println("\t" + station);
        }
    }

    public void sendMessage(byte[] message) throws IOException {
        outputStream.write(message);
    }
}
