import csv

def find_closest_row(input_file, Find_val, indice): #Encontra no artquivo "input file" o valor de tensão mais próximo da carga "findval" % e escreve na posição "indice" da lista de saída
    min_difference = float('inf')  

    with open(input_file, 'r') as csvfile:
        reader = csv.DictReader(csvfile)
        for row in reader:
            percentage_05A = row.get('Percentage_05A')
            Voltage = row.get('Voltage')
            if percentage_05A:
                value = float(percentage_05A)
                difference = abs(value - Find_val)
                if difference < min_difference:
                    min_difference = difference
                    Voltage_list[indice] = float(Voltage)


# Usage example:
input_file = "Meio_Amper_Carga_Perc.csvt"
Voltage_list = list(range(0,50)) #Cria a lista de saída. Neste caso estão sendo utilizada 50 posições, ou seja, a discretização dos valores será de 2 em 2 %.
i = 0x00
Val = 0x00

while i  < len(Voltage_list):    #Chama a função de busca de valor a quantidade de vezes necessária para preencher a lista de saída.
    find_closest_row(input_file, Val, i )
    i += 1
    Val += 100/(len(Voltage_list))

print("uint16_t V_Bat_mv[] = {", end='')   #Forma preguiçosa de escrever o output no terminal :)
for element in Voltage_list:

    print(str(int(element*1000)), end='')
    print(", ", end='')
print("};", end='') #REMOVER MANUALEMNTE A ÚLTIMA VIRGULA!!!!!!

# ==================================================================================================
# O script gera uma variável tipo vetor em C. Use essa variável como parâmetro no código do controlador para 
# realizar o cálculo do estado de carga da bateria tanto nos sensores quanto na central.