import os

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

csv_path = os.path.join("results", "cutting_energy.csv")
out_path = os.path.join("results", "cutting_energy_diagnostics.png")

df = pd.read_csv(csv_path)


def has_cols(frame, names):
    return all(name in frame.columns for name in names)


# Drop the initialization row for power calculations if dt is zero.
df = df.replace([np.inf, -np.inf], np.nan)
valid_dt = df["step_dt"] > 0
d = df.loc[valid_dt].copy()

eps = 1.0e-300
t = d["time"].to_numpy()

# Derived powers.
d["P_contact_fric_scaled"] = d["step_contact_E_fric_scaled"] / d["step_dt"]
d["P_contact_cond_scaled"] = d["step_contact_E_cond_scaled"] / d["step_dt"]
if has_cols(
    d,
    [
        "step_contact_P_cond_pos_raw",
        "step_contact_P_cond_neg_raw",
        "step_contact_P_cond_net_raw",
    ],
):
    d["P_contact_cond_pos_raw"] = d["step_contact_P_cond_pos_raw"]
    d["P_contact_cond_neg_raw"] = d["step_contact_P_cond_neg_raw"]
    d["P_contact_cond_net_raw_diag"] = d["step_contact_P_cond_net_raw"]
d["P_contact_workpiece"] = d["step_contact_E_workpiece"] / d["step_dt"]
d["P_contact_tool"] = d["step_contact_E_tool"] / d["step_dt"]
d["P_tool_sources"] = d["step_tool_E_sources"] / d["step_dt"]
d["P_tool_convection"] = d["step_tool_E_convection"] / d["step_dt"]
d["P_tool_dirichlet"] = d["step_tool_E_dirichlet"] / d["step_dt"]
d["P_tool_conduction"] = d["step_tool_E_conduction"] / d["step_dt"]

# Normalized residuals.
d["rel_step_interface_residual"] = np.abs(
    d["step_interface_balance_residual"]
) / np.maximum(np.abs(d["step_contact_E_fric_scaled"]), eps)
d["rel_step_tool_source_residual"] = np.abs(
    d["step_tool_source_residual"]
) / np.maximum(np.abs(d["step_contact_E_tool"]), eps)

raw_contact_scale = np.abs(d["step_contact_E_cond_raw"]) + np.abs(
    d["step_contact_E_fric_raw"]
)
d["limiter_fraction"] = d["step_contact_E_limiter_suppressed"] / np.maximum(
    raw_contact_scale, eps
)

# Internal energy changes.
d["wp_internal_E_delta"] = d["wp_internal_E"] - df["wp_internal_E"].iloc[0]
d["tool_internal_E_delta"] = d["tool_internal_E"] - df["tool_internal_E"].iloc[0]

print("Rows:", len(df))
print("Rows with dt > 0:", len(d))
print("Final time:", df["time"].iloc[-1])
print()
print("Final sampled cumulative energies:")
for col in [
    "cum_contact_E_fric_scaled",
    "cum_contact_E_workpiece",
    "cum_contact_E_tool",
    "cum_tool_E_sources",
    "cum_tool_E_convection",
    "cum_interface_balance_residual",
    "cum_tool_source_residual",
]:
    print(f"  {col}: {df[col].iloc[-1]:.15e}")

print()
print("Max absolute residuals:")
for col in [
    "step_interface_balance_residual",
    "step_tool_source_residual",
    "cum_interface_balance_residual",
    "cum_tool_source_residual",
]:
    print(f"  {col}: {np.nanmax(np.abs(df[col])):.15e}")

print()
print("Max normalized step residuals:")
print("  rel_step_interface_residual:", np.nanmax(d["rel_step_interface_residual"]))
print("  rel_step_tool_source_residual:", np.nanmax(d["rel_step_tool_source_residual"]))
print()
print("Max limiter fraction:", np.nanmax(d["limiter_fraction"]))

if has_cols(
    d,
    [
        "step_contact_event_count",
        "step_contact_area_eff",
        "step_contact_hA",
        "step_contact_max_pred_dT",
    ],
):
    print()
    print("Contact thermal diagnostics:")
    print("  max step_contact_event_count:", np.nanmax(d["step_contact_event_count"]))
    print("  max step_contact_area_eff:", np.nanmax(d["step_contact_area_eff"]))
    print("  max step_contact_hA:", np.nanmax(d["step_contact_hA"]))
    print("  max step_contact_max_pred_dT:", np.nanmax(d["step_contact_max_pred_dT"]))

fig, axes = plt.subplots(4, 2, figsize=(14, 14), constrained_layout=True)

ax = axes[0, 0]
ax.plot(t, d["P_contact_fric_scaled"], label="friction contact power")
ax.plot(t, d["P_contact_cond_scaled"], label="conductive contact power")
ax.set_title("Interface contact powers")
ax.set_xlabel("time")
ax.set_ylabel("power")
ax.grid(True)
ax.legend()

ax = axes[0, 1]
ax.plot(t, d["P_contact_workpiece"], label="to workpiece")
ax.plot(t, d["P_contact_tool"], label="to FE tool")
ax.set_title("Partitioned contact power")
ax.set_xlabel("time")
ax.set_ylabel("power")
ax.grid(True)
ax.legend()

ax = axes[1, 0]
ax.semilogy(
    t, np.maximum(d["rel_step_interface_residual"], eps), label="interface residual"
)
ax.semilogy(
    t, np.maximum(d["rel_step_tool_source_residual"], eps), label="tool source residual"
)
ax.set_title("Normalized per-step balance residuals")
ax.set_xlabel("time")
ax.set_ylabel("relative residual")
ax.grid(True, which="both")
ax.legend()

ax = axes[1, 1]
ax.plot(t, d["limiter_fraction"], label="limiter fraction")
ax.set_title("Limiter activity")
ax.set_xlabel("time")
ax.set_ylabel("suppressed / raw contact energy")
ax.grid(True)
ax.legend()

ax = axes[2, 0]
ax.plot(t, d["P_tool_sources"], label="tool contact sources")
ax.plot(t, d["P_tool_convection"], label="tool convection")
ax.plot(t, d["P_tool_dirichlet"], label="tool Dirichlet")
ax.plot(t, d["P_tool_conduction"], label="tool conduction")
ax.set_title("FE tool thermal budget powers")
ax.set_xlabel("time")
ax.set_ylabel("power")
ax.grid(True)
ax.legend()

ax = axes[2, 1]
ax.plot(t, d["wp_internal_E_delta"], label="workpiece internal energy change")
ax.plot(t, d["tool_internal_E_delta"], label="tool internal energy change")
ax.set_title("Internal energy changes")
ax.set_xlabel("time")
ax.set_ylabel("energy change")
ax.grid(True)
ax.legend()

ax = axes[3, 0]
if has_cols(
    d, ["step_contact_event_count", "step_contact_area_eff", "step_contact_hA"]
):
    ax.plot(t, d["step_contact_event_count"], label="contact events")
    ax2 = ax.twinx()
    ax2.plot(t, d["step_contact_area_eff"], color="tab:orange", label="sum A_eff")
    ax2.plot(t, d["step_contact_hA"], color="tab:green", label="sum h_c A_eff")
    ax2.set_ylabel("area / conductance")
    lines, labels = ax.get_legend_handles_labels()
    lines2, labels2 = ax2.get_legend_handles_labels()
    ax.legend(lines + lines2, labels + labels2)
else:
    ax.text(0.5, 0.5, "contact area diagnostics unavailable", ha="center", va="center")
ax.set_title("Contact area and conductance diagnostics")
ax.set_xlabel("time")
ax.set_ylabel("count")
ax.grid(True)

ax = axes[3, 1]
if has_cols(
    d,
    [
        "step_contact_P_cond_pos_raw",
        "step_contact_P_cond_neg_raw",
        "step_contact_deltaT_mean",
        "step_contact_h_c_mean",
    ],
):
    ax.plot(t, d["step_contact_P_cond_pos_raw"], label="raw P_cond into tool")
    ax.plot(t, d["step_contact_P_cond_neg_raw"], label="raw P_cond out of tool")
    ax.plot(t, d["step_contact_P_cond_net_raw"], label="raw P_cond net")
    ax2 = ax.twinx()
    ax2.plot(
        t,
        d["step_contact_deltaT_mean"],
        color="tab:purple",
        linestyle="--",
        label="mean ΔT",
    )
    ax2.plot(
        t,
        d["step_contact_h_c_mean"],
        color="tab:brown",
        linestyle="--",
        label="mean h_c",
    )
    ax2.set_ylabel("mean ΔT / h_c")
    lines, labels = ax.get_legend_handles_labels()
    lines2, labels2 = ax2.get_legend_handles_labels()
    ax.legend(lines + lines2, labels + labels2)
else:
    ax.text(
        0.5, 0.5, "conductive sign diagnostics unavailable", ha="center", va="center"
    )
ax.set_title("Conductive sign, ΔT, and h_c diagnostics")
ax.set_xlabel("time")
ax.set_ylabel("raw conductive power")
ax.grid(True)

fig.suptitle("cutting_energy.csv diagnostics")
fig.savefig(out_path, dpi=180)
print()
print("Wrote:", out_path)
