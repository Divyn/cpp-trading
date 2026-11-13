// Java order book. Build: javac OrderBook.java   Run: java OrderBook ops.bin 5
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.StringJoiner;

public class OrderBook {
    static final class Level {
        int key; // price for bids, -price for asks, so the best is always the largest
        int head;
        int tail;

        Level(int key) { this.key = key; }
    }

    final int[] qty, price, prev, next;
    final boolean[] buy, alive;
    final ArrayList<Level> bids = new ArrayList<>(); // sorted by key, best at the end
    final ArrayList<Level> asks = new ArrayList<>();
    long fills, traded, cancelled;

    OrderBook(int n) {
        qty = new int[n];
        price = new int[n];
        prev = new int[n];
        next = new int[n];
        buy = new boolean[n];
        alive = new boolean[n];
    }

    static int find(ArrayList<Level> levels, int key) {
        int lo = 0, hi = levels.size();
        while (lo < hi) {
            int mid = (lo + hi) >>> 1;
            if (levels.get(mid).key < key) lo = mid + 1;
            else hi = mid;
        }
        return lo;
    }

    void add(int id, int p, int q, boolean isBuy) {
        ArrayList<Level> other = isBuy ? asks : bids;
        while (q > 0 && !other.isEmpty()) {
            Level level = other.get(other.size() - 1);
            int best = isBuy ? -level.key : level.key;
            if (isBuy ? best > p : best < p) break;
            int o = level.head;
            int t = Math.min(q, qty[o]);
            qty[o] -= t;
            q -= t;
            fills++;
            traded += t;
            if (qty[o] == 0) {
                alive[o] = false;
                level.head = next[o];
                if (level.head == 0) other.remove(other.size() - 1);
                else prev[level.head] = 0;
            }
        }
        if (q == 0) return;

        ArrayList<Level> own = isBuy ? bids : asks;
        int k = isBuy ? p : -p;
        int i = find(own, k);
        if (i == own.size() || own.get(i).key != k) own.add(i, new Level(k));
        Level level = own.get(i);
        qty[id] = q;
        price[id] = p;
        buy[id] = isBuy;
        alive[id] = true;
        next[id] = 0;
        prev[id] = level.tail;
        if (level.tail != 0) next[level.tail] = id;
        else level.head = id;
        level.tail = id;
    }

    void cancel(int id) {
        if (!alive[id]) return;
        alive[id] = false;
        cancelled += qty[id];
        ArrayList<Level> side = buy[id] ? bids : asks;
        int i = find(side, buy[id] ? price[id] : -price[id]);
        Level level = side.get(i);
        int p = prev[id], n = next[id];
        if (p != 0) next[p] = n;
        else level.head = n;
        if (n != 0) prev[n] = p;
        else level.tail = p;
        if (level.head == 0) side.remove(i);
    }

    public static void main(String[] args) throws IOException {
        String path = args.length > 0 ? args[0] : "ops.bin";
        int runs = args.length > 1 ? Integer.parseInt(args[1]) : 5;
        ByteBuffer buf = ByteBuffer.wrap(Files.readAllBytes(Path.of(path))).order(ByteOrder.LITTLE_ENDIAN);
        int count = buf.getInt();
        int[] ops = new int[4 * count];
        buf.asIntBuffer().get(ops);

        StringJoiner seconds = new StringJoiner(",");
        OrderBook b = null;
        for (int r = 0; r < runs; r++) {
            b = new OrderBook(count + 2);
            long start = System.nanoTime();
            for (int i = 0; i < count; i++) {
                int j = 4 * i;
                if ((ops[j + 3] & 2) != 0) b.cancel(ops[j]);
                else b.add(ops[j], ops[j + 1], ops[j + 2], (ops[j + 3] & 1) != 0);
            }
            seconds.add(String.format("%.6f", (System.nanoTime() - start) / 1e9));
        }
        int bid = b.bids.isEmpty() ? 0 : b.bids.get(b.bids.size() - 1).key;
        int ask = b.asks.isEmpty() ? 0 : -b.asks.get(b.asks.size() - 1).key;
        System.out.printf("java,%d,%d,%d,%d,%d,%s%n", b.fills, b.traded, b.cancelled, bid, ask, seconds);
    }
}
