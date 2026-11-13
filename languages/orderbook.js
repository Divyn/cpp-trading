// Node order book. Run: node orderbook.js ops.bin 5
"use strict";
const fs = require("fs");

function find(levels, key) {
  let lo = 0, hi = levels.length;
  while (lo < hi) {
    const mid = (lo + hi) >>> 1;
    if (levels[mid].key < key) lo = mid + 1;
    else hi = mid;
  }
  return lo;
}

class Book {
  constructor(n) {
    this.qty = new Int32Array(n);
    this.price = new Int32Array(n);
    this.prev = new Int32Array(n);
    this.next = new Int32Array(n);
    this.buy = new Uint8Array(n);
    this.alive = new Uint8Array(n);
    this.bids = []; // sorted by key (price for bids, -price for asks), best at the end
    this.asks = [];
    this.fills = 0;
    this.traded = 0;
    this.cancelled = 0;
  }

  add(id, p, q, isBuy) {
    const other = isBuy ? this.asks : this.bids;
    while (q > 0 && other.length > 0) {
      const level = other[other.length - 1];
      const best = isBuy ? -level.key : level.key;
      if (isBuy ? best > p : best < p) break;
      const o = level.head;
      const t = Math.min(q, this.qty[o]);
      this.qty[o] -= t;
      q -= t;
      this.fills++;
      this.traded += t;
      if (this.qty[o] === 0) {
        this.alive[o] = 0;
        level.head = this.next[o];
        if (level.head === 0) other.pop();
        else this.prev[level.head] = 0;
      }
    }
    if (q === 0) return;

    const own = isBuy ? this.bids : this.asks;
    const k = isBuy ? p : -p;
    const i = find(own, k);
    if (i === own.length || own[i].key !== k) own.splice(i, 0, { key: k, head: 0, tail: 0 });
    const level = own[i];
    this.qty[id] = q;
    this.price[id] = p;
    this.buy[id] = isBuy ? 1 : 0;
    this.alive[id] = 1;
    this.next[id] = 0;
    this.prev[id] = level.tail;
    if (level.tail !== 0) this.next[level.tail] = id;
    else level.head = id;
    level.tail = id;
  }

  cancel(id) {
    if (!this.alive[id]) return;
    this.alive[id] = 0;
    this.cancelled += this.qty[id];
    const isBuy = this.buy[id] === 1;
    const side = isBuy ? this.bids : this.asks;
    const i = find(side, isBuy ? this.price[id] : -this.price[id]);
    const level = side[i];
    const p = this.prev[id], n = this.next[id];
    if (p !== 0) this.next[p] = n;
    else level.head = n;
    if (n !== 0) this.prev[n] = p;
    else level.tail = p;
    if (level.head === 0) side.splice(i, 1);
  }
}

const path = process.argv[2] || "ops.bin";
const runs = Number(process.argv[3] || 5);
const data = fs.readFileSync(path);
const words = new Int32Array(data.buffer, data.byteOffset, data.length / 4);
const count = words[0];
const ops = words.subarray(1);

const seconds = [];
let b;
for (let r = 0; r < runs; r++) {
  b = new Book(count + 2);
  const start = process.hrtime.bigint();
  for (let i = 0; i < count; i++) {
    const j = 4 * i;
    if (ops[j + 3] & 2) b.cancel(ops[j]);
    else b.add(ops[j], ops[j + 1], ops[j + 2], (ops[j + 3] & 1) !== 0);
  }
  seconds.push((Number(process.hrtime.bigint() - start) / 1e9).toFixed(6));
}
const bid = b.bids.length ? b.bids[b.bids.length - 1].key : 0;
const ask = b.asks.length ? -b.asks[b.asks.length - 1].key : 0;
console.log(`node,${b.fills},${b.traded},${b.cancelled},${bid},${ask},${seconds.join(",")}`);
