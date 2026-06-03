import pandas as pd
import matplotlib.pyplot as plt
import time
import os

# Caminho do CSV do teste
CSV_PATH = "apocalypse_log.csv"

# Limites Críticos
LIMIT_RAM_MB = 1000   # Ex: 1 GB
LIMIT_VRAM_MB = 23000 # Ex: 23 GB
LIMIT_HANDLES = 500

def plot_apocalypse(csv_path):
    plt.ion()  # modo interativo
    fig, ax = plt.subplots(3, 1, figsize=(12, 8), sharex=True)
    
    print("Iniciando Monitoramento em Tempo Real do Apocalipse...")
    print("Aguardando atualizacoes no CSV (Ctrl+C para sair)")
    
    while True:
        try:
            if not os.path.exists(csv_path):
                time.sleep(2)
                continue
                
            # Lê o CSV (pula o header na leitura)
            df = pd.read_csv(csv_path, header=0)
            
            if df.empty:
                time.sleep(2)
                continue
                
            # Renomear colunas caso necessario ou garantir nomes esperados
            # O CSV atual tem: Minute,RAM_MB,Handles,VRAM_MB,PendingTasks
            
            step = df['Minute']
            ram = df['RAM_MB']
            vram = df['VRAM_MB']
            handles = df['Handles']
            
            # Limpa os gráficos anteriores
            for a in ax:
                a.clear()
            
            # Plot RAM
            ax[0].plot(step, ram, color='blue', label='RAM Usage')
            ax[0].axhline(y=LIMIT_RAM_MB, color='red', linestyle='--', label='CRITICAL LIMIT')
            ax[0].set_ylabel("RAM (MB)")
            ax[0].set_title("Memory Leak Apocalypse - Realtime Monitoring")
            if ram.iloc[-1] > LIMIT_RAM_MB:
                ax[0].text(step.iloc[-1], ram.iloc[-1], ' ALERTA!', color='red', fontsize=12, fontweight='bold')
            ax[0].legend(loc='upper left')
            
            # Plot VRAM
            ax[1].plot(step, vram, color='green', label='VRAM Usage')
            ax[1].axhline(y=LIMIT_VRAM_MB, color='red', linestyle='--', label='CRITICAL LIMIT')
            ax[1].set_ylabel("VRAM (MB)")
            if vram.iloc[-1] > LIMIT_VRAM_MB:
                ax[1].text(step.iloc[-1], vram.iloc[-1], ' ALERTA!', color='red', fontsize=12, fontweight='bold')
            ax[1].legend(loc='upper left')
            
            # Plot Handles
            ax[2].plot(step, handles, color='orange', label='Handles')
            ax[2].axhline(y=LIMIT_HANDLES, color='red', linestyle='--', label='CRITICAL LIMIT')
            ax[2].set_ylabel("Handles")
            ax[2].set_xlabel("Time (Minutes/Steps)")
            if handles.iloc[-1] > LIMIT_HANDLES:
                ax[2].text(step.iloc[-1], handles.iloc[-1], ' ALERTA!', color='red', fontsize=12, fontweight='bold')
            ax[2].legend(loc='upper left')
            
            plt.pause(2)  # Atualiza a cada 2 segundos
        except KeyboardInterrupt:
            print("\nMonitoramento interrompido pelo usuario.")
            break
        except Exception as e:
            # Silence temporary errors during write locking
            time.sleep(1)

if __name__ == "__main__":
    plot_apocalypse(CSV_PATH)
