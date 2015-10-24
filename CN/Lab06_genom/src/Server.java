
//aag
public class Server {
    private static final String USAGE = "Usage: java Server <port> <stringHash> <stringLength>";

    public static void main(String[] args) {
        if(args.length != 3) {
            throw new IllegalArgumentException(USAGE);
        }
        int port = Integer.parseInt(args[0]);
        String hash = args[1];
        int stringLength = Integer.parseInt(args[2]);

       Thread server = new Thread(new ServerThread(port, hash, stringLength), "Server");
        server.start();
    }


}
