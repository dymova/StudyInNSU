package radio.server;

import radio.Protocol;
import radio.UnknownProtocolException;

import java.io.DataInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.List;

public class TcpMessageHandler implements Runnable{
    private final InputStream inputStream;
    private final OutputStream outputStream;
    private final Server server;
    private UdpMessageSender sender;

    public TcpMessageHandler(InputStream inputStream, OutputStream outputStream, Server server) {

        this.inputStream = inputStream;
        this.outputStream = outputStream;
        this.server = server;
    }

    @Override
    public void run() {
        try {
            while (!Thread.currentThread().isInterrupted()) {
                int type = inputStream.read();
                switch (type) {
                    case Protocol.LIST_COMMAND_TYPE:
                        handleListCommandMessage();
                        break;
                    case Protocol.PLAY_COMMAND_TYPE:
                        handlePlayCommandMessage();
                        break;
                    case Protocol.ABOUT_COMMAND_TYPE:
                        handleAboutCommandMessage();
                        break;
                    default:
                        throw new UnknownProtocolException("tcp handler type " + type);

                }
            }
        } catch (IOException | UnknownProtocolException e) {
            Thread.currentThread().interrupt();
        }
    }

    private void handleAboutCommandMessage() throws IOException {
        if(sender != null) {
            outputStream.write(createAboutMessage(sender.getCurrentItemName()));
        } else {
            byte[] message = {Protocol.STATION_NOT_CHOSE};
            outputStream.write(message);
//            outputStream.write(Protocol.STATION_NOT_CHOSE);
        }
    }

    private byte[] createAboutMessage(String currentItemName) {
        ByteBuffer byteBuffer = ByteBuffer.allocate(Protocol.BUFF_SIZE);
        byteBuffer.put(Protocol.ABOUT_ANSWER_TYPE);
        byte[] aboutData = currentItemName.getBytes();
        int size = aboutData.length;
        byteBuffer.putInt(size);
        byteBuffer.put(aboutData);
        return Arrays.copyOf(byteBuffer.array(), Protocol.BYTE_SIZE + Protocol.INT_SIZE + size);
    }

    private void handlePlayCommandMessage() throws IOException {
        DataInputStream dataInputStream = new DataInputStream(inputStream);
        int size = dataInputStream.readInt();
        byte[] requestData = new byte[size];
        inputStream.read(requestData);
        String request = new String(requestData);
        UdpMessageSender station = server.getStation(request);
        if(null != station){
            outputStream.write(createPlayAnswerMessage(station));
            server.playStations(station);
            sender = station;
        } else {
            outputStream.write(Protocol.STATION_NOT_EXIST_TYPE);
        }
    }

    private byte[] createPlayAnswerMessage(UdpMessageSender station) {
        ByteBuffer byteBuffer = ByteBuffer.allocate(Protocol.BUFF_SIZE);
        byteBuffer.put(Protocol.PLAY_ANSWER_TYPE);
        String message = station.getAddress() + Protocol.SPLITTER + station.getPort();
        byte[] messageData = message.getBytes();
        int size = messageData.length;
        byteBuffer.putInt(size);
        byteBuffer.put(messageData);
        return Arrays.copyOf(byteBuffer.array(), Protocol.BYTE_SIZE + Protocol.INT_SIZE + size);
    }


    private void handleListCommandMessage() throws IOException {
        ByteBuffer byteBuffer = ByteBuffer.allocate(Protocol.BUFF_SIZE);
        byteBuffer.put(Protocol.LIST_ANSWER_TYPE);
        List<String> stations = server.getStationsName();
        String message = "";
        for (String station : stations) {
            message += Protocol.SPLITTER + station;
        }
        byte[] messageData = message.getBytes();
        int size = messageData.length;
        byteBuffer.putInt(size);
        byteBuffer.put(messageData);
        outputStream.write(Arrays.copyOf(byteBuffer.array(), Protocol.BYTE_SIZE + Protocol.INT_SIZE + size));
    }
}
