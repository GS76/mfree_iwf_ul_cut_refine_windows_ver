import argparse
import csv
import os
from typing import List, Dict

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt


def read_csv(path: str) -> List[Dict[str, str]]:
	with open(path, "r", newline="", encoding="utf-8") as f:
		r = csv.DictReader(f)
		return list(r)


def col(rows: List[Dict[str, str]], key: str) -> List[float]:
	out = []
	for row in rows:
		v = row.get(key, "")
		try:
			out.append(float(v))
		except Exception:
			out.append(float("nan"))
	return out


def main() -> int:
	ap = argparse.ArgumentParser()
	ap.add_argument("--thermal-csv", required=True)
	ap.add_argument("--out-dir", required=True)
	ap.add_argument("--prefix", default="fe_tool")
	args = ap.parse_args()

	os.makedirs(args.out_dir, exist_ok=True)
	rows = read_csv(args.thermal_csv)
	t = col(rows, "time")
	tool_Tmin = col(rows, "tool_Tmin")
	tool_Tmax = col(rows, "tool_Tmax")
	P_cond = col(rows, "P_cond_W")
	P_fric = col(rows, "P_fric_W")

	plt.figure(figsize=(12, 6), dpi=160)
	plt.plot(t, tool_Tmin, label="tool_Tmin (K)")
	plt.plot(t, tool_Tmax, label="tool_Tmax (K)")
	plt.xlabel("time (s)")
	plt.ylabel("temperature (K)")
	plt.grid(True, alpha=0.3)
	plt.legend()
	plt.tight_layout()
	plt.savefig(os.path.join(args.out_dir, f"{args.prefix}_temperature_time_history.png"))
	plt.close()

	plt.figure(figsize=(12, 6), dpi=160)
	plt.plot(t, P_cond, label="P_cond (W)")
	plt.plot(t, P_fric, label="P_fric (W)")
	plt.xlabel("time (s)")
	plt.ylabel("power (W)")
	plt.grid(True, alpha=0.3)
	plt.legend()
	plt.tight_layout()
	plt.savefig(os.path.join(args.out_dir, f"{args.prefix}_power_time_history.png"))
	plt.close()

	out_csv = os.path.join(args.out_dir, f"{args.prefix}_thermal_history_extracted.csv")
	with open(out_csv, "w", newline="", encoding="utf-8") as f:
		w = csv.writer(f)
		w.writerow(["time_s", "tool_Tmin_K", "tool_Tmax_K", "P_cond_W", "P_fric_W"])
		for i in range(len(t)):
			w.writerow([t[i], tool_Tmin[i], tool_Tmax[i], P_cond[i], P_fric[i]])

	return 0


if __name__ == "__main__":
	raise SystemExit(main())

