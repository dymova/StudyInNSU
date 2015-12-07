import java.io.IOException;
import java.net.*;
import java.util.*;

public class Node implements Runnable {
    private static final String STARTING_MESSAGE = "IBORN";
    private static final String FINAL_MESSAGE = "IEXIT";
    private static final String ROOT_MESSAGE = "YOUROOT";
    private static final String NEW_PARENT_MESSAGE = "NEWPARENT";
    private static final String DELIMITER = ":";
    private static final int BUFSIZE = 512;

    private final Set<InetSocketAddress> children =
            Collections.synchronizedSet(new HashSet<>());
    private boolean isRoot = false;
    private int parentPort;
    private InetAddress parentAddress;
    private int port;

    public Node(String[] args) throws UnknownHostException {
        if (args.length == 1) {
            System.out.println("I'm root.");
            isRoot = true;
        } else {
            parentAddress = InetAddress.getByName(args[1]);
            parentPort = Integer.parseInt(args[2]);
        }
        port = Integer.parseInt(args[0]);

    }

    @Override
    public void run() {
        try (DatagramSocket socket = new DatagramSocket(port)) {
            System.out.println("Start: " + port);
            if (!isRoot) {
                sendStartingMessage(socket);
            }

            Runtime.getRuntime().addShutdownHook(new Thread() {
                public void run() {
                    try {
                        Iterator iterator =  children.iterator();
                        if (!isRoot) {
                            byte[] exitMessage = FINAL_MESSAGE.getBytes();
                            DatagramPacket exitPacket = new DatagramPacket(exitMessage, exitMessage.length, parentAddress, parentPort);
                            socket.send(exitPacket);
                        } else {
                            byte[] rootDate = ROOT_MESSAGE.getBytes();
                            if (children.size() == 0) {
                                return;
                            }
                            InetSocketAddress newRoot = (InetSocketAddress) iterator.next();
                            DatagramPacket rootPacket = new DatagramPacket(rootDate, rootDate.length, newRoot.getAddress(), newRoot.getPort());
                            socket.send(rootPacket);
                            System.out.println("new root:" + newRoot.getPort());
                            parentPort = newRoot.getPort();
                            parentAddress = newRoot.getAddress();
                        }
                        while(iterator.hasNext()) {
                            String newParentMessage = NEW_PARENT_MESSAGE + DELIMITER + parentAddress + DELIMITER + parentPort;
                            byte[] newParentData = newParentMessage.getBytes();
                            InetSocketAddress child = (InetSocketAddress) iterator.next();
                            System.out.println("child: "+ child.getPort());
                            DatagramPacket newParentPacket = new DatagramPacket(newParentData,
                                    newParentData.length, child.getAddress(), child.getPort());
                            socket.send(newParentPacket);
                        }
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            });

            Thread consoleThread = new Thread(new ConsoleMessageHandler(socket, this), "ConsoleMessageHandler");
            consoleThread.start();


            byte[] receiveData = new byte[BUFSIZE];
            DatagramPacket receivePacket = new DatagramPacket(receiveData, receiveData.length);
            for (; ; ) {
                socket.receive(receivePacket);
                String receiveMessage = new String(receivePacket.getData(), 0, receivePacket.getLength());
                String[] splittedMessage = receiveMessage.split(DELIMITER);
                switch (splittedMessage[0]) {
                    case STARTING_MESSAGE:
                        children.add(new InetSocketAddress(receivePacket.getAddress(), receivePacket.getPort()));
                        System.out.println(receiveMessage + receivePacket.getPort());
                        printChild();
                        break;
                    case FINAL_MESSAGE:
                        children.remove(new InetSocketAddress(receivePacket.getAddress(), receivePacket.getPort()));
                        System.out.println(receiveMessage);
                        printChild();
                        break;
                    case NEW_PARENT_MESSAGE:
                        parentAddress = InetAddress.getByName(splittedMessage[1].substring(1,splittedMessage[1].length()));
                        parentPort = Integer.parseInt(splittedMessage[2]);
                        sendStartingMessage(socket);
                        System.out.println(receiveMessage);
                        printChild();
                        break;
                    case ROOT_MESSAGE:
                        isRoot = true;
                        System.out.println(receiveMessage);
                        printChild();
                        break;
                    default:
                        System.out.println(">" + receiveMessage);

                        InetSocketAddress sourceMessage = new InetSocketAddress(receivePacket.getAddress(), receivePacket.getPort());
                        if(children.contains(sourceMessage) && !isRoot) {
                            receivePacket.setAddress(parentAddress);
                            receivePacket.setPort(parentPort);
                            socket.send(receivePacket);
                        }
                        for (InetSocketAddress child : children) {
                            if(child.equals(sourceMessage)) {
                                continue;
                            }
                            receivePacket.setAddress(child.getAddress());
                            receivePacket.setPort(child.getPort());
                            socket.send(receivePacket);
                        }
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void sendStartingMessage(DatagramSocket socket) throws IOException {
        byte[] startingMessageData = STARTING_MESSAGE.getBytes();
        DatagramPacket startingPacket = new DatagramPacket(startingMessageData,
                startingMessageData.length, parentAddress, parentPort);
        socket.send(startingPacket);
    }

    private void printChild() {
        System.out.println("-----------------------");
        for (InetSocketAddress child : children) {
            System.out.println(child.getAddress()+ " " + child.getPort());
        }
        System.out.println("-----------------------");

    }

    public Set<InetSocketAddress> getChildren() {
        return children;
    }

    public boolean isRoot() {
        return isRoot;
    }

    public int getParentPort() {
        return parentPort;
    }

    public InetAddress getParentAddress() {
        return parentAddress;
    }

}
