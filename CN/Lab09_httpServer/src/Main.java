public class Main {
    private static final String USAGE = "Usage: java Main <serverPort>";


    public static void main(String[] args) {
        if(args.length != 2) {
            throw new IllegalArgumentException(USAGE);
        }
        int port = Integer.parseInt(args[0]);
        HttpServer server = new HttpServer(port, args[1]);
        server.run();
    }
}
