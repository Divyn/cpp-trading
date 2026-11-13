"""Python order book. Run: python3 orderbook.py ops.bin 5"""
import sys
import time
from array import array
from bisect import bisect_left
from operator import itemgetter

KEY = itemgetter(0)


def find(levels, key):
    return bisect_left(levels, key, key=KEY)


class Book:
    def __init__(self, n):
        self.qty = [0] * n
        self.price = [0] * n
        self.prev = [0] * n
        self.next = [0] * n
        self.buy = [False] * n
        self.alive = [False] * n
        # each level is [key, head, tail]; key is price for bids, -price for asks,
        # sorted by key with the best at the end
        self.bids = []
        self.asks = []
        self.fills = self.traded = self.cancelled = 0

    def add(self, id, p, q, is_buy):
        other = self.asks if is_buy else self.bids
        while q > 0 and other:
            level = other[-1]
            best = -level[0] if is_buy else level[0]
            if (best > p) if is_buy else (best < p):
                break
            o = level[1]
            t = min(q, self.qty[o])
            self.qty[o] -= t
            q -= t
            self.fills += 1
            self.traded += t
            if self.qty[o] == 0:
                self.alive[o] = False
                level[1] = self.next[o]
                if level[1] == 0:
                    other.pop()
                else:
                    self.prev[level[1]] = 0
        if q == 0:
            return

        own = self.bids if is_buy else self.asks
        k = p if is_buy else -p
        i = find(own, k)
        if i == len(own) or own[i][0] != k:
            own.insert(i, [k, 0, 0])
        level = own[i]
        self.qty[id] = q
        self.price[id] = p
        self.buy[id] = is_buy
        self.alive[id] = True
        self.next[id] = 0
        self.prev[id] = level[2]
        if level[2]:
            self.next[level[2]] = id
        else:
            level[1] = id
        level[2] = id

    def cancel(self, id):
        if not self.alive[id]:
            return
        self.alive[id] = False
        self.cancelled += self.qty[id]
        side = self.bids if self.buy[id] else self.asks
        i = find(side, self.price[id] if self.buy[id] else -self.price[id])
        level = side[i]
        p, n = self.prev[id], self.next[id]
        if p:
            self.next[p] = n
        else:
            level[1] = n
        if n:
            self.prev[n] = p
        else:
            level[2] = p
        if level[1] == 0:
            del side[i]


def main():
    path = sys.argv[1] if len(sys.argv) > 1 else "ops.bin"
    runs = int(sys.argv[2]) if len(sys.argv) > 2 else 5
    words = array("i")
    with open(path, "rb") as f:
        words.frombytes(f.read())
    if sys.byteorder != "little":
        words.byteswap()
    count = words[0]
    flat = words[1:].tolist()
    ops = list(zip(flat[0::4], flat[1::4], flat[2::4], flat[3::4]))

    seconds = []
    for _ in range(runs):
        b = Book(count + 2)
        start = time.perf_counter()
        for id, p, q, flags in ops:
            if flags & 2:
                b.cancel(id)
            else:
                b.add(id, p, q, flags & 1 == 1)
        seconds.append(f"{time.perf_counter() - start:.6f}")
    bid = b.bids[-1][0] if b.bids else 0
    ask = -b.asks[-1][0] if b.asks else 0
    print(f"python,{b.fills},{b.traded},{b.cancelled},{bid},{ask},{','.join(seconds)}")


if __name__ == "__main__":
    main()
