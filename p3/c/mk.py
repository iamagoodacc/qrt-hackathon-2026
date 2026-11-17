"""Write a paste-ready p3 probe (stdout). Kinds: const v1,v2,..  | zeros k | affine a b | table "t:v,..;t:v,.."
(table = return the row whose t is nearest the input t; use it to test several candidates per submission)."""
import sys
kind = sys.argv[1]
head = "#include <bits/stdc++.h>\nusing namespace std;\n\n"
def fl(xs): return "{" + ", ".join(f"{float(x)!r}f" for x in xs) + "}"
if kind == "const":
    body = f"    return vector<float>{fl(sys.argv[2].split(','))};"
elif kind == "zeros":
    body = f"    return vector<float>({int(sys.argv[2])}, 0.0f);"
elif kind == "affine":
    a, b = float(sys.argv[2]), float(sys.argv[3])
    body = f"    return vector<float>{{{a!r}f * t + {b!r}f}};"
elif kind == "table":
    rows = [r.split(":") for r in sys.argv[2].split(";") if r]
    ts = ", ".join(f"{float(t)!r}f" for t, _ in rows)
    vs = ",\n        ".join("vector<float>" + fl(v.split(",")) for _, v in rows)
    body = (f"    static const vector<float> ts = {{{ts}}};\n"
            f"    static const vector<vector<float>> vs = {{\n        {vs}}};\n"
            "    size_t best = 0;\n"
            "    for (size_t i = 1; i < ts.size(); i++) if (fabs(ts[i] - t) < fabs(ts[best] - t)) best = i;\n"
            "    return vs[best];")
else:
    sys.exit(__doc__)
print(head + "vector<float> probe(float t) {\n" + body + "\n}")
