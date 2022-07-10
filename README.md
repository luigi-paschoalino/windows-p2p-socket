# Windows P2P Socket

Grupo: 
Caio Duque Gorgulho Araújo - 2020014150
Juan Pablo Ribeiro - 2021001881
Leonardo Henrique Pereira - 2020028156
Luigi Paschoalino - 2020011865

Os clientes e servidores foram desenvolvidos para serem utilizados dentro do Windows.
Funcionamento apenas dentro da máquina. Não testado se funcional dentro da rede local.

Para compilar os arquivos .c, os seguintes comandos devem ser usados:

	Cliente: gcc cliente.c -o cliente -lws2_32
	Servidor: gcc servidor.c -o servidor -lws2_32

Para a execução correta dos arquivos, a ordem de abertura e os respectivos comandos no cmd devem ser:

	Servidor: apenas executar o arquivo "servidor.exe" ou digitar "servidor" dentro do cmd
	Clientes: "cliente 127.0.0.1 <opção>"
		As opções são:
			1 - seeder (cliente irá enviar um arquivo contido na mesma pasta)
			2 - leecher (cleinte irá solicitar e receber um arquivo contido pelo cliente seeder)

Ao término da execução dos clientes, o servidor irá efetuar a conexão direta entre os dois e irá encerrar seu funcionamento, pois presume que o arquivo a ser solicitado está em posse do seeder.

Durante a transferência, ambos clientes irão informar o número de sequência dos pacotes transferidos.

Após o término da transferência, os clientes se encerrarão.