// Go order book. Build: go build -o orderbook_go orderbook.go
package main

import (
	"encoding/binary"
	"fmt"
	"os"
	"sort"
	"strconv"
	"strings"
	"time"
)

type Level struct {
	key  int32 // price for bids, -price for asks, so the best is always the largest
	head int32
	tail int32
}

type Book struct {
	qty, price, prev, next   []int32
	buy, alive               []bool
	bids, asks               []Level // sorted by key, best at the end
	fills, traded, cancelled int64
}

func newBook(n int) *Book {
	return &Book{
		qty: make([]int32, n), price: make([]int32, n),
		prev: make([]int32, n), next: make([]int32, n),
		buy: make([]bool, n), alive: make([]bool, n),
	}
}

func find(levels []Level, key int32) int {
	return sort.Search(len(levels), func(i int) bool { return levels[i].key >= key })
}

func (b *Book) add(id, p, q int32, isBuy bool) {
	other := &b.bids
	if isBuy {
		other = &b.asks
	}
	for q > 0 && len(*other) > 0 {
		level := &(*other)[len(*other)-1]
		best := level.key
		if isBuy {
			best = -level.key
		}
		if (isBuy && best > p) || (!isBuy && best < p) {
			break
		}
		o := level.head
		t := min(q, b.qty[o])
		b.qty[o] -= t
		q -= t
		b.fills++
		b.traded += int64(t)
		if b.qty[o] == 0 {
			b.alive[o] = false
			level.head = b.next[o]
			if level.head == 0 {
				*other = (*other)[:len(*other)-1]
			} else {
				b.prev[level.head] = 0
			}
		}
	}
	if q == 0 {
		return
	}

	own := &b.asks
	k := -p
	if isBuy {
		own = &b.bids
		k = p
	}
	i := find(*own, k)
	if i == len(*own) || (*own)[i].key != k {
		*own = append(*own, Level{})
		copy((*own)[i+1:], (*own)[i:])
		(*own)[i] = Level{key: k}
	}
	level := &(*own)[i]
	b.qty[id] = q
	b.price[id] = p
	b.buy[id] = isBuy
	b.alive[id] = true
	b.next[id] = 0
	b.prev[id] = level.tail
	if level.tail != 0 {
		b.next[level.tail] = id
	} else {
		level.head = id
	}
	level.tail = id
}

func (b *Book) cancel(id int32) {
	if !b.alive[id] {
		return
	}
	b.alive[id] = false
	b.cancelled += int64(b.qty[id])
	side := &b.asks
	k := -b.price[id]
	if b.buy[id] {
		side = &b.bids
		k = b.price[id]
	}
	i := find(*side, k)
	level := &(*side)[i]
	p, n := b.prev[id], b.next[id]
	if p != 0 {
		b.next[p] = n
	} else {
		level.head = n
	}
	if n != 0 {
		b.prev[n] = p
	} else {
		level.tail = p
	}
	if level.head == 0 {
		*side = append((*side)[:i], (*side)[i+1:]...)
	}
}

func main() {
	path, runs := "ops.bin", 5
	if len(os.Args) > 1 {
		path = os.Args[1]
	}
	if len(os.Args) > 2 {
		runs, _ = strconv.Atoi(os.Args[2])
	}
	data, err := os.ReadFile(path)
	if err != nil {
		panic(err)
	}
	words := make([]int32, len(data)/4)
	for i := range words {
		words[i] = int32(binary.LittleEndian.Uint32(data[4*i:]))
	}
	count := int(words[0])
	ops := words[1:]

	var seconds []string
	var b *Book
	for r := 0; r < runs; r++ {
		b = newBook(count + 2)
		start := time.Now()
		for i := 0; i < count; i++ {
			op := ops[4*i : 4*i+4]
			if op[3]&2 != 0 {
				b.cancel(op[0])
			} else {
				b.add(op[0], op[1], op[2], op[3]&1 != 0)
			}
		}
		seconds = append(seconds, fmt.Sprintf("%.6f", time.Since(start).Seconds()))
	}
	var bid, ask int32
	if len(b.bids) > 0 {
		bid = b.bids[len(b.bids)-1].key
	}
	if len(b.asks) > 0 {
		ask = -b.asks[len(b.asks)-1].key
	}
	fmt.Printf("go,%d,%d,%d,%d,%d,%s\n", b.fills, b.traded, b.cancelled, bid, ask, strings.Join(seconds, ","))
}
