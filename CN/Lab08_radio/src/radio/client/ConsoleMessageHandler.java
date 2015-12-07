package radio.client;

import radio.Protocol;
import radio.UnknownProtocolException;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.Scanner;

public class ConsoleMessageHandler implements Runnable {
    private final Client client;

    public ConsoleMessageHandler(Client client) {
        this.client = client;
    }

    @Override
    public void run() {
        printHelp();
        BufferedReader reader = new BufferedReader(new InputStreamReader(System.in));
        String newLine;
        try {
            while (!Thread.currentThread().isInterrupted()) {
                newLine = reader.readLine();
                handleMessage(newLine.trim());
            }
        } catch (IOException e) {
            e.printStackTrace();
            Thread.currentThread().interrupt();
        }


    }

    private void printHelp() {
        System.out.println("For get list stations enter: list, \n" +
                "for chose station enter: play:<stationName>, \n" +
                "for get current item name enter: about");
    }

    private void handleMessage(String newLine) throws IOException {
        String[] strings = newLine.split(Protocol.SPLITTER, 2);
        switch (strings[0]) {
            case Protocol.LIST_COMMAND:
                handleListCommandMessage();
                break;
            case Protocol.PLAY_COMMAND:
                if (strings.length > 1) {
                    handlePlayCommandMessage(strings[1]);
                } else {
                    printHelp();
                }
                break;
            case Protocol.ABOUT_COMMAND:
                handleAboutCommandMessage();
                break;
            default:
                printHelp();
        }
    }

    private void handleAboutCommandMessage() throws IOException {
        byte[] message = {Protocol.ABOUT_COMMAND_TYPE}; //todo
        client.sendMessage(message);
    }

    private void handlePlayCommandMessage(String string) throws IOException {
        ByteBuffer byteBuffer = ByteBuffer.allocate(Protocol.BUFF_SIZE);
        byteBuffer.put(Protocol.PLAY_COMMAND_TYPE);
        byte[] messageData = string.getBytes();
        int size = messageData.length;
        byteBuffer.putInt(size);
        byteBuffer.put(messageData);
        client.sendMessage(Arrays.copyOf(byteBuffer.array(), Protocol.BYTE_SIZE + Protocol.INT_SIZE + size));
    }

    private void handleListCommandMessage() throws IOException {
        byte[] message = {Protocol.LIST_COMMAND_TYPE};

        client.sendMessage(message);
    }
}
