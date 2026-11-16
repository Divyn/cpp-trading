from decimal import Decimal, ROUND_HALF_UP

CASES = [0.5, 1.5, 2.5, -0.5, -1.5, -2.5]


def half_up(x):
    return int(Decimal(str(x)).quantize(Decimal("1"), rounding=ROUND_HALF_UP))


def drift(f, sign):
    prices = [sign * (100.5 + k) for k in range(1000)]
    return sum(f(p) for p in prices) - sum(prices)


for name, f in [("round", round), ("Decimal ROUND_HALF_UP", half_up)]:
    print("python", name, *[int(f(x)) for x in CASES], drift(f, 1), drift(f, -1), sep=",")
