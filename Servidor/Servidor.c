#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#pragma comment(lib, "Ws2_32.lib")

#define PACOTE_BUFFER_SIZE 100
#define MAX_NOME_ARQUIVO 100
#define TRUE 1
#define FALSE 0

typedef struct cliente
{
    struct sockaddr_in addr; // informaões dos clientes
    int online;              // status do cliente

} Cliente;

//função para printar erros e encerrar o programa
void error(char *msg)
{
    printf("%s\n", msg);
    exit(1);
}

int main()
{

    WSADATA wsaData;

    int socks;
    int opcao = 0; // Escolha do tipo de cliente que se conectará, 1 - Seeder ; 2 - Leecher

    struct sockaddr_in servaddr;
    struct sockaddr_in clienteaddr;

    Cliente cliente_seeder;
    Cliente cliente_leecher;

    cliente_leecher.online = FALSE;
    cliente_seeder.online = FALSE;

    memset(&(cliente_leecher.addr), 0, sizeof(cliente_leecher.addr));
    memset(&(cliente_seeder.addr), 0, sizeof(cliente_seeder.addr));
    memset(&(clienteaddr), 0, sizeof(clienteaddr));

    socklen_t l = sizeof(struct sockaddr_in);

    // Iniciando a DLL de sockets windows
    WSAStartup(MAKEWORD(2, 1), &wsaData);

    // Criando socket do servidor
    socks = socket(AF_INET, SOCK_DGRAM, 0);
    if (socks < 0)
    {
        printf("Erro ao criar socket\n");
        exit(0);
    }
    
    memset((char *)&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(7802);

    // Bindando com a porta
    if (bind(socks, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
        printf("Erro ao bindar\n");

    printf("Servidor inicializado com sucesso!\n\n");
    printf("Esperando clientes se conectarem...\n\n");

    // O servidor permite apenas 2 clientes simultaneos

    while (cliente_leecher.online == FALSE || cliente_seeder.online == FALSE)
    {

        // Eecebe pedido de conexão do cliente
        if (recvfrom(socks, &opcao, sizeof(opcao), 0, (struct sockaddr *)&clienteaddr, &l) < 0)
        {
            error("Erro ao receber conexão do cliente");
        }

        // Verifica qual tipo de cliente conectou

        if (opcao == 2 && cliente_leecher.online == FALSE) // Conectando cliente leecher
        {
            cliente_leecher.addr = clienteaddr;
            cliente_leecher.online = TRUE;
            memset(&(clienteaddr), 0, sizeof(clienteaddr));
            printf("Cliente que deseja leecher conectado\n\n");
        }

        else if (opcao == 1 && cliente_seeder.online == FALSE) // Conectando cliente leecher
        {
            cliente_seeder.addr = clienteaddr;
            cliente_seeder.online = TRUE;
            memset(&(clienteaddr), 0, sizeof(clienteaddr));
            printf("Cliente seeder conectado\n");
        }
        else if (opcao == 2) // Caso já exista um cliente leecher conectado, nada acontece
            printf("Ja existe um cliente leecher conectado\n");
        else if (opcao == 1) // Caso já exista um cliente seeder conectado, nada acontece
            printf("Ja existe um cliente seeder conectado\n");

        // Servidor so ira conectar clientes em papeis diferentes, nao podendo ter dois clientes seeder e leecher ao mesmo tempo

        if (cliente_leecher.online == TRUE && cliente_seeder.online == TRUE)
        {
            // Envia informacoes do seeder para leecher
            if (sendto(socks, &(cliente_seeder.addr), sizeof(cliente_seeder.addr), 0, (struct sockaddr *)&(cliente_leecher.addr), sizeof(cliente_leecher.addr)) < 0)
            {
                error("Erro ao enviar informações do seeder\n");
            }
            printf("Endereco do seeder enviado com sucesso!\n");

            // Envia informacoes do leecher para seeder
            if (sendto(socks, &(cliente_leecher.addr), sizeof(cliente_leecher.addr), 0, (struct sockaddr *)&(cliente_seeder.addr), sizeof(cliente_seeder.addr)) < 0)
            {
                error("Erro ao enviar informações do leecher\n");
            }
            printf("Endereço do leecher enviado com sucesso!\n");
        }
    }

    closesocket(socks);
    WSACleanup();
    return 0;
}