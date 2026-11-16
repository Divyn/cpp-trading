import java.util.function.DoubleUnaryOperator;

public class Round {
    static double drift(DoubleUnaryOperator f, int sign) {
        double r = 0, e = 0;
        for (int k = 0; k < 1000; k++) { double p = sign * (100.5 + k); r += f.applyAsDouble(p); e += p; }
        return r - e;
    }

    public static void main(String[] args) {
        double[] cases = {0.5, 1.5, 2.5, -0.5, -1.5, -2.5};
        String[] names = {"Math.round", "Math.rint"};
        DoubleUnaryOperator[] fns = {x -> Math.round(x), Math::rint};
        for (int i = 0; i < 2; i++) {
            StringBuilder sb = new StringBuilder("java," + names[i]);
            for (double x : cases) sb.append(",").append((long) fns[i].applyAsDouble(x));
            sb.append(",").append(drift(fns[i], 1)).append(",").append(drift(fns[i], -1));
            System.out.println(sb);
        }
    }
}
