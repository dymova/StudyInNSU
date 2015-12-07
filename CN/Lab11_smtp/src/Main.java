import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;

public class Main {
    public static final String USAGE = "java Main <smtpServerAddress> <serverPort>";

    public static void main(String[] args) throws IOException {
        if(args.length != 2) {
            throw new IllegalArgumentException(USAGE);
        }
        InetAddress address = InetAddress.getByName(args[0]);
        int port = Integer.parseInt(args[1]);


        SmtpSender smtpSender = new SmtpSender(address, port);
        smtpSender.start();
    }
}
