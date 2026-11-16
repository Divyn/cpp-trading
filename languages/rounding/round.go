package main

import (
	"fmt"
	"math"
	"strings"
)

func drift(f func(float64) float64, sign float64) float64 {
	r, e := 0.0, 0.0
	for k := 0; k < 1000; k++ {
		p := sign * (100.5 + float64(k))
		r += f(p)
		e += p
	}
	return r - e
}

func main() {
	cases := []float64{0.5, 1.5, 2.5, -0.5, -1.5, -2.5}
	names := []string{"math.Round", "math.RoundToEven"}
	fns := []func(float64) float64{math.Round, math.RoundToEven}
	for i, f := range fns {
		vals := []string{}
		for _, x := range cases {
			vals = append(vals, fmt.Sprint(int64(f(x))))
		}
		fmt.Printf("go,%s,%s,%g,%g\n", names[i], strings.Join(vals, ","), drift(f, 1), drift(f, -1))
	}
}
