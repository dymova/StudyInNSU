public class UnknownProtocolException extends Exception {
    private final String moreInfo;

    public UnknownProtocolException(String moreInfo) {
        this.moreInfo = moreInfo;
    }

    @Override
    public String toString() {
        return "UnknownProtocolException: " + moreInfo + " .\n";
    }
}