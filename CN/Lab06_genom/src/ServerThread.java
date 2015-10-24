import java.io.*;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;

public class ServerThread implements Runnable {
    private final String hash;
    private final int stringLength;
    private final int port;
    private byte nextDiapason = 0;

    public ServerThread(int port, String hash, int stringLength) {
        this.hash = hash;
        this.stringLength = stringLength;
        this.port = port;
    }

    @Override
    public void run() {
        try (ServerSocket serverSocket = new ServerSocket(port)) {
            System.out.println("Server starting in " + port + " port.");
            HashMap<InetAddress, ArrayList<DiapasonInfo>> clients = new HashMap<>();
            while (true) {
                Socket socket = serverSocket.accept();
                try (InputStream inputStream = socket.getInputStream();
                     DataInputStream dataInputStream = new DataInputStream(inputStream);
                     OutputStream outputStream = socket.getOutputStream()) {
                    while (true) {
                        int size = dataInputStream.readInt();
//                        System.out.println("size:" + size);
                        int type = inputStream.read();
                        System.out.println("type:" + type);
                        switch (type) {
                            case Protocol.FIST_REQUEST_TYPE:
                                outputStream.write(createMessageWithHash());
                                break;
                            case Protocol.DIAPASON_REQUEST_TYPE:
                                ArrayList<DiapasonInfo> array = clients.get(socket.getInetAddress());
                                if (array == null) {
                                    array = new ArrayList<>();
                                    array.add(new DiapasonInfo(nextDiapason));
                                    clients.put(socket.getInetAddress(), array);
                                } else {
                                    array.add(new DiapasonInfo(nextDiapason));
                                }
                                outputStream.write(createMessageWithDiapason());
                                break;
                            case Protocol.RESULT_TYPE:
                                ArrayList<DiapasonInfo> diapasons = clients.get(socket.getInetAddress());
                                DiapasonInfo info = diapasons.get(diapasons.size() - 1);
                                int count = dataInputStream.readInt();
                                byte[] result = new byte[size - Protocol.INT_SIZE];
                                inputStream.read(result);
                                if (count != 0) {
                                    info.setResults(getStringArrayListFromResult(count, result));
                                }
                        }
                        printClientList(clients);

                    }
                } catch (EOFException | UnknownProtocolException ignored) {
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }

    }

    private void printClientList(HashMap<InetAddress, ArrayList<DiapasonInfo>> clients) {
        for (InetAddress inetAddress : clients.keySet()) {
            System.out.println(inetAddress + ":");
            for (DiapasonInfo diapasonInfo : clients.get(inetAddress)) {
                System.out.print("\t" + diapasonInfo.getDiapason() + ": ");
                if (diapasonInfo.getResults() != null) {
                    for (String s : diapasonInfo.getResults()) {
                        System.out.print(s + " ");
                    }
                    System.out.println();
                } else {
                    System.out.println("null");
                }
            }

        }
    }

    private ArrayList<String> getStringArrayListFromResult(int count, byte[] result) throws UnknownProtocolException {
        ArrayList<String> strings = new ArrayList<>(count);
        int byteInString = (stringLength / 4) + 1;
        String str = "";
        for (int i = 0; i < result.length; i++) {
            for (int j = 0; j < 4; j++) {
                byte b = (byte) ((result[i] >> j * 2) & 3);
                str += Protocol.byteToChar(b);
            }
            if ((i + 1) % byteInString == 0) {
                strings.add(new StringBuilder(str).reverse().toString());
                str = "";
            }
        }
        for (String string : strings) {
            System.out.println("strings: " + string);
        }

        return strings;
    }


    private byte[] createAnswer(int size, int type) {
        switch (type) {
            case Protocol.FIST_REQUEST_TYPE:
                return createMessageWithHash();
            case Protocol.DIAPASON_REQUEST_TYPE:
                return createMessageWithDiapason();
        }
        throw new IllegalArgumentException("from server");
    }

    private byte[] createMessageWithDiapason() {
        if (nextDiapason != Protocol.LAST_DIAPASON) {
            ByteBuffer byteMessageBuffer = ByteBuffer.allocate(Protocol.BUF_SIZE);
            byteMessageBuffer.putInt(1);
            byteMessageBuffer.put(Protocol.NEW_DIAPASON_TYPE);
            byteMessageBuffer.put(nextDiapason);
            nextDiapason++;
            return Arrays.copyOf(byteMessageBuffer.array(), Protocol.PREFIX_SIZE + 1);
        } else {
            ByteBuffer byteMessageBuffer = ByteBuffer.allocate(Protocol.BUF_SIZE);
            byteMessageBuffer.putInt(0);
            byteMessageBuffer.put(Protocol.DIAPASONS_ENDED_TYPE);
            return Arrays.copyOf(byteMessageBuffer.array(), Protocol.PREFIX_SIZE);
        }
    }

    private byte[] createMessageWithHash() {
        ByteBuffer byteMessageBuffer = ByteBuffer.allocate(Protocol.BUF_SIZE);
        int size = Protocol.INT_SIZE + hash.getBytes().length;
        byteMessageBuffer.putInt(size);
        byteMessageBuffer.put(Protocol.HASH_ANSWER_TYPE);
        byteMessageBuffer.putInt(stringLength);
        byteMessageBuffer.put(hash.getBytes());

        return Arrays.copyOf(byteMessageBuffer.array(), Protocol.PREFIX_SIZE + size);
    }
}
