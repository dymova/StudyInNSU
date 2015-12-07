import java.io.*;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Date;

public class Connection implements Runnable {
    public static final double VERSION = 1.1;
    public static final int OK_CODE = 200;
    public static final int NOT_FOUNT_CODE = 404;
    public static final int BUF_SIZE = 1024;
    public static final int BAD_REQUEST_CODE = 400;
    private final InputStream inputStream;
    private final OutputStream outputStream;

    private final String defaultDir;
    private static final String ERROR_NOT_FOUND = "<html><body><center><h1>404 Not found</h1></center></body></html>\n";
    private static final String ERROR_BAD_REQUEST = "<html><body><center><h1>400 Bad request</h1></center>" +
            "<br> <center></body>Server works only with GET requests<center></body></html>\n";

    public Connection(InputStream inputStream, OutputStream outputStream, String defaultDir) {
        this.inputStream = inputStream;
        this.outputStream = outputStream;
        this.defaultDir = defaultDir;
    }

    @Override
    public void run() {
        try (BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream))) {
            String header = reader.readLine();
            System.out.println("header: "+ header);

            if(header.contains("GET")) {
                String uri = getURIFromHeader(header);
                handleRequest(uri);
            } else {
                outputStream.write(createAnswerHeader(BAD_REQUEST_CODE).getBytes());
                outputStream.write(ERROR_BAD_REQUEST.getBytes());
            }


        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void handleRequest(String uri) throws IOException {
        Path path = Paths.get(uri);
        String header;

        if(Files.isDirectory(path)) {
            path = Paths.get(defaultDir + "/index.html");
        }

        if(Files.isRegularFile(path)) {
            header = createAnswerHeader(OK_CODE);
            outputStream.write(header.getBytes());
            sendFile(path);
        } else {
            header = createAnswerHeader(NOT_FOUNT_CODE);
            System.out.println("answer:" + header);
            outputStream.write(header.getBytes());
            outputStream.write(ERROR_NOT_FOUND.getBytes());
        }

    }

    private void sendFile(Path path) {
        try(FileInputStream reader = new FileInputStream(path.toString())) {
            int bytesReadNumber;
            byte[] buffer = new byte[BUF_SIZE];
            while (-1 != (bytesReadNumber = reader.read(buffer))) {
                outputStream.write(buffer, 0, bytesReadNumber);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private String createAnswerHeader(int code) {
        StringBuilder builder = new StringBuilder();
        builder.append("HTTP/")
                .append(VERSION)
                .append(" ")
                .append(code)
                .append("\n");

        builder.append("Date: ")
                .append(new Date().toString())
                .append("\n");

        builder.append("\n\n");

        return builder.toString();
    }

    private String getURIFromHeader(String header) {
        String[] substring = header.split(" ");
        String uri = substring[1];
        int paramIndex = uri.indexOf("?");
        if(paramIndex != -1) {
            uri = uri.substring(0, paramIndex);
        }

        return defaultDir + uri;
    }
}
