// Rust order book. Build: rustc -C opt-level=3 orderbook.rs -o orderbook_rust
use std::env;
use std::fs;
use std::time::Instant;

struct Level {
    key: i32, // price for bids, -price for asks, so the best is always the largest
    head: usize,
    tail: usize,
}

struct Book {
    qty: Vec<i32>,
    price: Vec<i32>,
    prev: Vec<usize>,
    next: Vec<usize>,
    buy: Vec<bool>,
    alive: Vec<bool>,
    bids: Vec<Level>, // sorted by key, best at the end
    asks: Vec<Level>,
    fills: i64,
    traded: i64,
    cancelled: i64,
}

fn find(levels: &[Level], key: i32) -> usize {
    levels.partition_point(|l| l.key < key)
}

impl Book {
    fn new(n: usize) -> Book {
        Book {
            qty: vec![0; n],
            price: vec![0; n],
            prev: vec![0; n],
            next: vec![0; n],
            buy: vec![false; n],
            alive: vec![false; n],
            bids: Vec::new(),
            asks: Vec::new(),
            fills: 0,
            traded: 0,
            cancelled: 0,
        }
    }

    fn add(&mut self, id: usize, p: i32, mut q: i32, is_buy: bool) {
        let other = if is_buy { &mut self.asks } else { &mut self.bids };
        while q > 0 {
            let Some(level) = other.last_mut() else { break };
            let best = if is_buy { -level.key } else { level.key };
            if if is_buy { best > p } else { best < p } {
                break;
            }
            let o = level.head;
            let t = q.min(self.qty[o]);
            self.qty[o] -= t;
            q -= t;
            self.fills += 1;
            self.traded += t as i64;
            if self.qty[o] == 0 {
                self.alive[o] = false;
                level.head = self.next[o];
                if level.head == 0 {
                    other.pop();
                } else {
                    self.prev[level.head] = 0;
                }
            }
        }
        if q == 0 {
            return;
        }

        let own = if is_buy { &mut self.bids } else { &mut self.asks };
        let k = if is_buy { p } else { -p };
        let i = find(own, k);
        if i == own.len() || own[i].key != k {
            own.insert(i, Level { key: k, head: 0, tail: 0 });
        }
        let level = &mut own[i];
        self.qty[id] = q;
        self.price[id] = p;
        self.buy[id] = is_buy;
        self.alive[id] = true;
        self.next[id] = 0;
        self.prev[id] = level.tail;
        if level.tail != 0 {
            self.next[level.tail] = id;
        } else {
            level.head = id;
        }
        level.tail = id;
    }

    fn cancel(&mut self, id: usize) {
        if !self.alive[id] {
            return;
        }
        self.alive[id] = false;
        self.cancelled += self.qty[id] as i64;
        let is_buy = self.buy[id];
        let side = if is_buy { &mut self.bids } else { &mut self.asks };
        let k = if is_buy { self.price[id] } else { -self.price[id] };
        let i = find(side, k);
        let (p, n) = (self.prev[id], self.next[id]);
        if p != 0 {
            self.next[p] = n;
        } else {
            side[i].head = n;
        }
        if n != 0 {
            self.prev[n] = p;
        } else {
            side[i].tail = p;
        }
        if side[i].head == 0 {
            side.remove(i);
        }
    }
}

fn main() {
    let args: Vec<String> = env::args().collect();
    let path = args.get(1).map(String::as_str).unwrap_or("ops.bin");
    let runs: usize = args.get(2).map(|s| s.parse().unwrap()).unwrap_or(5);
    let bytes = fs::read(path).unwrap();
    let words: Vec<i32> = bytes
        .chunks_exact(4)
        .map(|c| i32::from_le_bytes([c[0], c[1], c[2], c[3]]))
        .collect();
    let count = words[0] as usize;
    let ops = &words[1..];

    let mut seconds = Vec::new();
    let mut last = None;
    for _ in 0..runs {
        let mut book = Book::new(count + 2);
        let start = Instant::now();
        for op in ops.chunks_exact(4) {
            if op[3] & 2 != 0 {
                book.cancel(op[0] as usize);
            } else {
                book.add(op[0] as usize, op[1], op[2], op[3] & 1 != 0);
            }
        }
        seconds.push(start.elapsed().as_secs_f64());
        last = Some(book);
    }
    let b = last.unwrap();
    let best_bid = b.bids.last().map_or(0, |l| l.key);
    let best_ask = b.asks.last().map_or(0, |l| -l.key);
    let times: Vec<String> = seconds.iter().map(|s| format!("{:.6}", s)).collect();
    println!("rust,{},{},{},{},{},{}", b.fills, b.traded, b.cancelled, best_bid, best_ask, times.join(","));
}
