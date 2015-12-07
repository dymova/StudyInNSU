package radio.server;

import radio.Protocol;

import java.io.*;
import java.net.*;

import static java.lang.Thread.sleep;

public class UdpMessageSender implements Runnable {
    private final File pathToSrc;
    private final String name;
    private String currentItemName;
    private InetAddress address;
    private final int port;

    public UdpMessageSender(File pathToSrc, int port) throws IOException {
        this.pathToSrc = pathToSrc;
        this.port = port;
        File[] files = pathToSrc.listFiles();
        for (File file : files) {
            if(file.getName().equals(Protocol.CONFIG)) {
                BufferedReader reader = new BufferedReader(new FileReader(file));
                address = InetAddress.getByName(reader.readLine().split(Protocol.SPLITTER)[1]);
                break;
            }
        }
        name = pathToSrc.getName();
    }

    @Override
    public void run() {
        try(DatagramSocket datagramSocket = new DatagramSocket()) {
            DatagramPacket packet;
            while(!Thread.currentThread().isInterrupted()) {
                File[] files = pathToSrc.listFiles();
                for (File file : files) {
                    BufferedReader reader = new BufferedReader(new FileReader(file));
                    currentItemName = file.getName();
                    String line;
                    while ((line = reader.readLine()) != null) {
                        byte[] message = line.getBytes();
                        packet = new DatagramPacket(message, message.length, address, port);
                        datagramSocket.send(packet);
                        sleep(5000);
                        System.out.println("<<" + line);
                    }
                }
            }
        } catch (IOException | InterruptedException e) {
            e.printStackTrace();
        }
    }

    public String getCurrentItemName() {
        return currentItemName;
    }

    public String getAddress() {
        return address.getHostAddress();
    }

    public int getPort() {
        return port;
    }

    public String getName() {
        return name;
    }
}
