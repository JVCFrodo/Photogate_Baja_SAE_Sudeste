import serial
import openpyxl
import sys
import time
import serial.tools.list_ports
import pandas as pd
from rich.console import Console
from rich.table import Table
from datetime import datetime

def Show_Logo_Image():
    ascii_art = """
                ____   ___        __ ___       _____  ___     ______   _____  __  __ ____   ______ _____ ______ ______
               / __ ) /   |      / //   |     / ___/ /   |   / ____/  / ___/ / / / // __ \ / ____// ___//_  __// ____/
              / __  |/ /| | __  / // /| |     \__ \ / /| |  / __/     \__ \ / / / // / / // __/   \__ \  / /  / __/   
             / /_/ // ___ |/ /_/ // ___ |    ___/ // ___ | / /___    ___/ // /_/ // /_/ // /___  ___/ / / /  / /___   
            /_____//_/  |_|\____//_/  |_|   /____//_/  |_|/_____/   /____/ \____//_____//_____/ /____/ /_/  /_____/                                                                                                                                                                                    
    ____                          __                   __       __             __            __                ___  _    __ ______
   / __ \ ____  _      __ ____   / /____   ____ _ ____/ /  ____/ /___     ____/ /____ _ ____/ /____   _____   /   || |  / // ____/
  / / / // __ \| | /| / // __ \ / // __ \ / __ `// __  /  / __  // _ \   / __  // __ `// __  // __ \ / ___/  / /| || | / // /_    
 / /_/ // /_/ /| |/ |/ // / / // // /_/ // /_/ // /_/ /  / /_/ //  __/  / /_/ // /_/ // /_/ // /_/ /(__  )  / ___ || |/ // __/    
/_____/ \____/ |__/|__//_/ /_//_/ \____/ \__,_/ \__,_/   \__,_/ \___/   \__,_/ \__,_/ \__,_/ \____//____/  /_/  |_||___//_/                                                                                                                                                                                                                                                                                                                                                 
    """
    print(ascii_art)

def Search_Usb_Adapter():
    COM_Port = ''
    myports = [tuple(p) for p in list(serial.tools.list_ports.comports())]
    for port in myports:
        if "USB-SERIAL CH340" in port[1]:
          COM_Port = port[0]
    if COM_Port == '':
        print("Adaptador USB não encontrado!!! ")
        print("Encerrando programa...")
    return COM_Port 

def Show_Results(Data):

    tabela = Table(title="Resultados AVF")

    Colunas = ["Passada", "Carro", "30 Metros", "Velocidade km/h"]
    Linhas = Data

    for Coluna in Colunas:
        tabela.add_column(Coluna)

    for Linha in Linhas:
        tabela.add_row(*Linha, style='bright_green')   

    console = Console()
    print(" ")
    console.print(tabela)




# Function to extract variables from the received string
def extract_variables(data):
    passada = data[0:3]
    carro = data[4:7]
    metros_30 = data[8:14]
    velocidade = data[15:17]
    return passada, carro, metros_30, velocidade

# Function to initialize Excel pythonworkbook and worksheet
def initialize_excel():
    workbook = openpyxl.Workbook()
    sheet = workbook.active
    sheet.title = "Serial Data"
    sheet.append(["Passada", "Carro", "30 Metros", "Velocidade km/h"])
    return workbook, sheet

# Function to append data to Excel worksheet
def append_to_excel(sheet, data):
    sheet.append(data)

# Main function to read from serial and save to Excel
def main(com_port):
    # Configure the serial port (adjust the parameters as needed)
    ser = serial.Serial(com_port, 9600, timeout=1)

    # Initialize Excel workbook and worksheet
    workbook, sheet = initialize_excel()
    line = ""
    
    print("Aguardando conexão...")
    while("Handshake Ok" not in line):
        line = ''
        line = ser.readline().decode('utf-8').strip()
        line_write = ("Handshake Request").encode('UTF-8')
        ser.write(line_write)
        time.sleep(0.1)

    print("Sicronia estabelecida. Para iniciar, presisone o botão de início na tela.") 
    line = ''

    while("Data Size" not in line):
        line = ''
        line = ser.readline().decode('utf-8').strip()
        time.sleep(0.01)
    
    parts = line.split()
    Num_Lines = 0x00
    # Loop through the parts to find and return the first integer
    for part in parts:
        if part.isdigit():
            Num_Lines = int(part)
            line_write = ("Numlines Ok").encode('UTF-8')
            ser.write(line_write)

    i = 0x00
    Results = []
    while i < Num_Lines:
        # Read a line from the serial port
        line = ser.readline().decode('utf-8')
        if line != '':
            # Extract variables from the received line
            passada, carro, metros_30, velocidade = extract_variables(line)
            # Append the data to the Excel worksheet
            append_to_excel(sheet, [passada, carro, metros_30, velocidade])
            Results.append([passada, carro, metros_30, velocidade])


            line_write = ("Line Ok").encode('UTF-8')
            ser.write(line_write)
            i = i+1
            line = ""
    Show_Results(Results)

    
    current_datetime = datetime.now()
    formatted_datetime = current_datetime.strftime("%Y-%m-%d_%H-%M-%S")
    filename = f"Resultados_AVF_{formatted_datetime}.xlsx"
    workbook.save(filename)
    print(f"Download Finalizado. Verifique o arquivo xlsx {filename}.")
    ser.close()

if __name__ == "__main__":
    # if len(sys.argv) != 2:
    #     print("Usage: python script.py <COM_PORT>")
    #     sys.exit(1)

    # com_port = sys.argv[1]
    Show_Logo_Image()
    com_port = Search_Usb_Adapter()
    
    if com_port == '':
        time.sleep(5)
        sys.exit(1)
    else:
        print(f"Adaptador USB encontrado na porta: {com_port}")
        print("Para interromper o programa pressione CTRL+C")
        main(com_port)

    