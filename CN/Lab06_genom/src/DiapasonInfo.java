import java.util.ArrayList;

public class DiapasonInfo {
    private final byte diapason;
    private ArrayList<String> results;

    public DiapasonInfo(byte diapason) {
        this.diapason = diapason;
    }

    public void setResults(ArrayList<String> results) {
        this.results = results;
    }

    public ArrayList<String> getResults() {
        return results;
    }

    public byte getDiapason() {
        return diapason;
    }
}
