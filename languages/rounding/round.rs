fn drift(f: fn(f64) -> f64, sign: f64) -> f64 {
    let (mut r, mut e) = (0.0, 0.0);
    for k in 0..1000 { let p = sign * (100.5 + k as f64); r += f(p); e += p; }
    r - e
}

fn main() {
    let cases = [0.5, 1.5, 2.5, -0.5, -1.5, -2.5];
    let fns: [(&str, fn(f64) -> f64); 2] = [("f64::round", f64::round), ("f64::round_ties_even", f64::round_ties_even)];
    for (name, f) in fns {
        let vals: Vec<String> = cases.iter().map(|&x| (f(x) as i64).to_string()).collect();
        println!("rust,{},{},{},{}", name, vals.join(","), drift(f, 1.0), drift(f, -1.0));
    }
}
