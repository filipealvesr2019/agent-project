import csv
import re
from collections import defaultdict

print("\n=========================================================================================")
print("                   AgentOS: Relatório Consolidado de Benchmark (Inferência)               ")
print("=========================================================================================")

# Extrair tempos de carregamento do log
load_times = {}
try:
    with open('benchmark_log.txt', 'r', encoding='utf-16', errors='ignore') as f:
        current_model = None
        for line in f:
            if "Carregando modelo:" in line:
                current_model = line.split("Carregando modelo:")[1].split("(")[0].strip()
            elif "Model loaded in" in line and current_model:
                ms = re.search(r'in (\d+) ms', line)
                if ms:
                    load_times[current_model] = int(ms.group(1))
except Exception:
    try:
        with open('benchmark_log.txt', 'r', encoding='utf-8', errors='ignore') as f:
            current_model = None
            for line in f:
                if "Carregando modelo:" in line:
                    current_model = line.split("Carregando modelo:")[1].split("(")[0].strip()
                elif "Model loaded in" in line and current_model:
                    ms = re.search(r'in (\d+) ms', line)
                    if ms:
                        load_times[current_model] = int(ms.group(1))
    except FileNotFoundError:
        pass

# Agregar dados do CSV
stats = defaultdict(lambda: {'tps': [], 'ram': [], 'status': set(), 'latency': []})

try:
    with open('benchmark_results.csv', 'r', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        for row in reader:
            modelo = row['Modelo']
            stats[modelo]['tps'].append(float(row['TPS']))
            stats[modelo]['ram'].append(float(row['RAM_MB']))
            stats[modelo]['latency'].append(float(row['Latencia(ms)']))
            stats[modelo]['status'].add(row['Status'])

    print(f"{'Modelo':<40} | {'TPS Médio':<10} | {'RAM (MB)':<10} | {'Load (ms)':<10} | {'Resp.(ms)':<10}")
    print("-" * 90)

    for modelo, data in stats.items():
        avg_tps = sum(data['tps']) / len(data['tps']) if data['tps'] else 0
        avg_ram = sum(data['ram']) / len(data['ram']) if data['ram'] else 0
        avg_lat = sum(data['latency']) / len(data['latency']) if data['latency'] else 0
        load_ms = load_times.get(modelo, 0)
        
        print(f"{modelo:<40} | {avg_tps:<10.2f} | {avg_ram:<10.0f} | {load_ms:<10} | {avg_lat:<10.0f}")

    print("=========================================================================================\n")
    print("Decisões de Arquitetura Sugeridas com base nos dados:")
    print("- O modelo com maior TPS e menor RAM deve ser o 'Surrogate Router' (L3) permanente.")
    print("- O modelo com maior inteligência suportável (Cenário B/C) será alocado sob demanda para o Cognitive Engine.")
    print("- A soma da RAM dos modelos concorrentes não deve ultrapassar 20GB (reserva de SO).\n")

except Exception as e:
    print(f"Erro ao gerar relatório: {e}")
