"""
Gera o gráfico de speedup x número de threads para o trabalho P1 (LP2).
Executa: python scripts/plot_speedup.py
Saída:   results/speedup.png
"""

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import os

# Dados medidos na máquina de teste (Intel i7-1255U, n=1000, média de 5 runs)
threads   = [1,     2,     4,     8,     10   ]
t_par     = [2.293, 0.985, 0.794, 0.524, 0.581]  # segundos
speedup   = [1.026, 1.501, 2.163, 3.192, 3.156]
eficiencia = [s / t * 100 for s, t in zip(speedup, threads)]

ideal = [float(t) for t in threads]

fig, axes = plt.subplots(1, 2, figsize=(11, 4.5))

# --- Speedup ---
ax = axes[0]
ax.plot(threads, ideal,   "k--",  linewidth=1.2, label="Ideal (linear)")
ax.plot(threads, speedup, "o-",   color="#1f77b4", linewidth=2,
        markersize=7, label="Medido")
ax.set_xlabel("Número de threads")
ax.set_ylabel("Speedup  (T_seq / T_par)")
ax.set_title("Speedup × Threads  —  P1 Multiplicação de Matrizes")
ax.set_xticks(threads)
ax.legend()
ax.grid(True, linestyle="--", alpha=0.5)

for x, y in zip(threads, speedup):
    ax.annotate(f"{y:.2f}×", xy=(x, y), xytext=(4, 6),
                textcoords="offset points", fontsize=8.5)

# --- Eficiência ---
ax2 = axes[1]
ax2.plot(threads, eficiencia, "s-", color="#d62728", linewidth=2,
         markersize=7, label="Eficiência")
ax2.axhline(100, color="k", linestyle="--", linewidth=1.2, label="100 %")
ax2.set_xlabel("Número de threads")
ax2.set_ylabel("Eficiência  (%)")
ax2.set_title("Eficiência × Threads  —  P1 Multiplicação de Matrizes")
ax2.set_xticks(threads)
ax2.legend()
ax2.grid(True, linestyle="--", alpha=0.5)

for x, y in zip(threads, eficiencia):
    ax2.annotate(f"{y:.0f}%", xy=(x, y), xytext=(4, 6),
                 textcoords="offset points", fontsize=8.5)

fig.tight_layout()

out_path = os.path.join(os.path.dirname(__file__), "..", "results", "speedup.png")
out_path = os.path.normpath(out_path)
fig.savefig(out_path, dpi=150)
print(f"Gráfico salvo em: {out_path}")
