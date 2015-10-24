import java.io.*;
import java.math.BigInteger;
import java.net.InetAddress;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.Arrays;

public class ClientThread implements Runnable {
    private final InetAddress serverAddress;
    private final int serverPort;
    private int stringLength;
    private byte currentDiapason;
    private String hash;
    private boolean needNewDiapason = true;

    public ClientThread(InetAddress serverAddress, int serverPort) {
        this.serverAddress = serverAddress;
        this.serverPort = serverPort;
    }

    @Override
    public void run() {
        try (Socket socket = new Socket(serverAddress, serverPort)) {
            try (OutputStream outputStream = socket.getOutputStream();
                 InputStream inputStream = socket.getInputStream();
                 DataInputStream dataInputStream = new DataInputStream(inputStream)) {

                //send starting message
                byte[] startingMessageData = createStartingMessage();
                outputStream.write(startingMessageData);

                //receive hash and stringLength
                int messageSize = dataInputStream.readInt();
//                System.out.println("size: " + messageSize);
                byte messageType = (byte) inputStream.read();
                System.out.println("type:" + messageType);
                if (messageType == Protocol.HASH_ANSWER_TYPE) {
                    stringLength = dataInputStream.readInt();
                    System.out.println("string length: " + stringLength);
                    byte[] hashData = new byte[messageSize - Protocol.INT_SIZE];
                    inputStream.read(hashData);
                    hash = new String(hashData);
                    System.out.println("hash: " + hash);
                } else {
                    throw new UnknownProtocolException("messageType==" + messageType);
                }

                //send diapason request and receive answer
                if (!getNewDiapason(outputStream, inputStream, dataInputStream)) {
                    return;
                }
            }
            //compute
            ArrayList<byte[]> suitableString = computeDiapason();

            //send result
//            byte[] resultMessage = createResultMessage(suitableString);
            while(sendResultAndGetNewDiapason(createResultMessage(suitableString))){
                suitableString = computeDiapason();
            }

        } catch (IOException | UnknownProtocolException | NoSuchAlgorithmException e) {
            e.printStackTrace();
        }


    }

    private boolean getNewDiapason(OutputStream outputStream, InputStream inputStream, DataInputStream dataInputStream) throws IOException, UnknownProtocolException {
        int messageSize;
        byte messageType;
        outputStream.write(createDiapasonRequest());

        //receive diapason
        messageSize = dataInputStream.readInt();
        System.out.println("size: " + messageSize);
        messageType = (byte) inputStream.read();
        System.out.println("type:" + messageType);
        if (messageType == Protocol.DIAPASONS_ENDED_TYPE) {
            needNewDiapason = false;
            return false;
        } else if (messageType == Protocol.NEW_DIAPASON_TYPE) {
            currentDiapason = (byte) inputStream.read();
            System.out.println("currentDiapason: " + currentDiapason);
        } else {
            throw new UnknownProtocolException("messageType==" + messageType);
        }
        return true;
    }

    private boolean sendResultAndGetNewDiapason(byte[] resultMessage) throws IOException, UnknownProtocolException {
        try (Socket socket = new Socket(serverAddress, serverPort)) {
            try (OutputStream outputStream = socket.getOutputStream();
                 InputStream inputStream = socket.getInputStream();
                 DataInputStream dataInputStream = new DataInputStream(inputStream)) {

                outputStream.write(resultMessage);

                return getNewDiapason(outputStream, inputStream, dataInputStream);
            }
        }
    }

    private ArrayList<byte[]> computeDiapason() throws UnsupportedEncodingException, NoSuchAlgorithmException {
        ArrayList<byte[]> diapasonAsArray = getDiapasonAsArray();
        ArrayList<byte[]> suitableString = new ArrayList<>();

        for (byte[] bytes : diapasonAsArray) {
            if (getMd5Hash(getString(bytes)).equals(hash)) {
                suitableString.add(bytes);
            }
        }
        return suitableString;
    }

    private String getString(byte[] bytes) {
        String str = "";
        for (byte aByte : bytes) {
            switch (aByte) {
                case Protocol.A:
                    str += 'a';
                    break;
                case Protocol.C:
                    str += 'c';
                    break;
                case Protocol.G:
                    str += 'g';
                    break;
                case Protocol.T:
                    str += 't';
                    break;
            }
        }
        return str;
    }

    private ArrayList<byte[]> getDiapasonAsArray() {
        ArrayList<byte[]> array = new ArrayList<>();
        long stringCounts = (long) Math.pow(4, stringLength - 2);
        byte first = (byte) (currentDiapason >> 2);
        byte second = (byte) (currentDiapason & 3); //3 = 00000011
        System.out.println("first: " + first);
        System.out.println("second: " + second);

        for (long stringEndNo = 0; stringEndNo < stringCounts; stringEndNo++) {
            ByteBuffer byteBuffer = ByteBuffer.allocate(stringLength);
            byteBuffer.put(first);
            byteBuffer.put(second);
            for (int i = 0; i < stringLength - 2; i++) {
                int symbolNumber = (int) (stringEndNo / 4);
                byteBuffer.put((byte) ((stringEndNo >> symbolNumber * 2) & 3));
            }
            array.add(byteBuffer.array());
        }
//        for (byte[] bytes : array) {
//            for (byte aByte : bytes) {
//                System.out.print(aByte);
//            }
//            System.out.println();
//        }
        return array;
    }

    private static byte[] createStartingMessage() {
        ByteBuffer byteMessageBuffer = ByteBuffer.allocate(Protocol.BUF_SIZE);
        byteMessageBuffer.putInt(0);
        byteMessageBuffer.put(Protocol.FIST_REQUEST_TYPE);
        return Arrays.copyOf(byteMessageBuffer.array(), Protocol.PREFIX_SIZE);
    }


    private byte[] createDiapasonRequest() {
        ByteBuffer byteMessageBuffer = ByteBuffer.allocate(Protocol.BUF_SIZE);
        byteMessageBuffer.putInt(0);
        byteMessageBuffer.put(Protocol.DIAPASON_REQUEST_TYPE);
        return Arrays.copyOf(byteMessageBuffer.array(), Protocol.PREFIX_SIZE);
    }

    private byte[] createResultMessage(ArrayList<byte[]> suitableString) {
        ByteBuffer byteMessageBuffer = ByteBuffer.allocate(Protocol.BUF_SIZE);
        if (!suitableString.isEmpty()) {
            byte[] strings = arrayListToByteArray(suitableString);
            int size = Protocol.INT_SIZE + strings.length;
            byteMessageBuffer.putInt(size);
            byteMessageBuffer.put(Protocol.RESULT_TYPE);
            byteMessageBuffer.putInt(suitableString.size());
            byteMessageBuffer.put(strings);
            return Arrays.copyOf(byteMessageBuffer.array(), Protocol.PREFIX_SIZE + size);
        } else {
            byteMessageBuffer.putInt(Protocol.INT_SIZE);
            byteMessageBuffer.put(Protocol.RESULT_TYPE);
            byteMessageBuffer.putInt(0);
            return Arrays.copyOf(byteMessageBuffer.array(), Protocol.PREFIX_SIZE + Protocol.INT_SIZE);
        }

    }

    private byte[] arrayListToByteArray(ArrayList<byte[]> suitableString) {
        ByteBuffer byteBuffer = ByteBuffer.allocate((stringLength / 4) + 1);
        for (byte[] bytes : suitableString) {
            byte b = 0;
            int bigPartCount = bytes.length / 4;
            for (int j = 0; j < bigPartCount; j++) {
                for (int i = 0; i < 4; i++) {
                    b = (byte) ((b << 2) + bytes[i]);
                }
                byteBuffer.put(b);
                b = 0;
            }
            for (int i = 0; i < bytes.length % 4; i++) {
                b = (byte) ((b << 2) + bytes[i + bigPartCount*Protocol.SYMBOL_IN_BYTE]);
            }
            byteBuffer.put(b);
        }
        return byteBuffer.array();
    }

    private String getMd5Hash(String str) throws UnsupportedEncodingException, NoSuchAlgorithmException {
        byte[] strData = str.getBytes("UTF-8");
        MessageDigest md = MessageDigest.getInstance("MD5");
        byte[] res = md.digest(strData);
        BigInteger bigInt = new BigInteger(1, res);
        String md5Hex = bigInt.toString(16);
        while (md5Hex.length() < 32) {
            md5Hex = "0" + md5Hex;
        }
        return md5Hex;
    }
}
