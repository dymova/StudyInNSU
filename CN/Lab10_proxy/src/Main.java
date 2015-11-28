public class Main {
    public static final String USAGE = "java Main <proxyPort>";

    public static void main(String[] args) {
        if(args.length != 1) {
            throw new IllegalArgumentException(USAGE);
        }

        int port = Integer.parseInt(args[0]);
        new Proxy(port).start();
    }
}
