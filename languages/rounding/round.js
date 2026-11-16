const CASES = [0.5, 1.5, 2.5, -0.5, -1.5, -2.5];
const even = new Intl.NumberFormat("en-US", { maximumFractionDigits: 0, roundingMode: "halfEven", useGrouping: false });
const fns = [["Math.round", Math.round], ["Intl halfEven", (x) => Number(even.format(x))]];
const drift = (f, sign) => {
  let r = 0, e = 0;
  for (let k = 0; k < 1000; k++) { const p = sign * (100.5 + k); r += f(p); e += p; }
  return r - e;
};
for (const [name, f] of fns) console.log(["node", name, ...CASES.map((x) => String(f(x) + 0)), drift(f, 1), drift(f, -1)].join(","));
