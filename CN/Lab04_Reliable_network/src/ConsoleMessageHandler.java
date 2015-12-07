import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetSocketAddress;
import java.util.Scanner;
import java.util.Set;

public class ConsoleMessageHandler implements Runnable {

    private final Node node;
    private final DatagramSocket socket;

    public ConsoleMessageHandler(DatagramSocket socket, Node node) {
        this.socket = socket;
        this.node = node;
    }

    @Override
    public void run() {
        try {
            Scanner scanner = new Scanner(System.in, "UTF-8");
            System.out.println("Please, enter message:");
            Set<InetSocketAddress> children = node.getChildren();

            for (;;) {
                String message = scanner.nextLine();
                if (message.length() == 0) {
                    continue;
                }
                byte[] sendData = message.getBytes();
                for (InetSocketAddress child : children) {
                    DatagramPacket childPacket = new DatagramPacket(sendData,
                            sendData.length, child.getAddress(), child.getPort());
                    socket.send(childPacket);
                }
                if(!node.isRoot()) {
                    DatagramPacket parentPacket = new DatagramPacket(message.getBytes(),message.getBytes().length,
                            node.getParentAddress(), node.getParentPort());
                    socket.send(parentPacket);
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
