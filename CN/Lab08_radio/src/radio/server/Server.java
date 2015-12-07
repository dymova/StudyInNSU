package radio.server;

import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.*;
import java.util.stream.Collectors;

public class Server {
    private final int port;
    private final Map<UdpMessageSender, Thread> stationsThreads;

    public Server(int port, String pathToData) throws IOException {
        this.port = port;
        stationsThreads = Collections.synchronizedMap(new HashMap<>());
        File path = new File(pathToData);
        File[] files = path.listFiles();
        File config;
        for (File file : files) {
            if (file.isDirectory()) {
                UdpMessageSender udpMessageSender = new UdpMessageSender(file, port);
                stationsThreads.put(udpMessageSender, new Thread(udpMessageSender));
            }
        }
        stationsThreads.keySet().forEach(p -> System.out.println(p.getName()));
    }

    public void start() {
        try (ServerSocket serverSocket = new ServerSocket(port)) {
            System.out.println("Server starting on " + port + " port");
            while (true) {
                Socket socket = serverSocket.accept();
                socket.setSoTimeout(600000);
                System.out.println("connection establish");
//                new Connection(socket, this);
                Thread tcpThread  = new Thread(new TcpMessageHandler(socket.getInputStream(),socket.getOutputStream(), this), "TCPHandler");
                tcpThread.start();

            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public List<String> getStationsName() {
        List<String> stationsName = stationsThreads.keySet().stream()
                .map(UdpMessageSender::getName)
                .collect(Collectors.toList());
        return stationsName;
    }

    public UdpMessageSender getStation(String request) {
        for (UdpMessageSender udpMessageSender : stationsThreads.keySet()) {
            if (udpMessageSender.getName().equals(request)) {
                return udpMessageSender;
            }
        }
        return null;
    }

    public void playStations(UdpMessageSender station) {
        Thread thread = stationsThreads.get(station);
        if(!thread.isAlive()){
            thread.start();
        }

    }
}
