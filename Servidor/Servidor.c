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
    int opcao = 0; //escolha do tipo de cliente que se conectará, 1=semear; 2=baixar

    struct sockaddr_in servaddr;
    struct sockaddr_in clienteaddr;

    Cliente cliente_semear;
    Cliente cliente_baixar;

    cliente_baixar.online = FALSE;
    cliente_semear.online = FALSE;

    memset(&(cliente_baixar.addr), 0, sizeof(cliente_baixar.addr));
    memset(&(cliente_semear.addr), 0, sizeof(cliente_semear.addr));
    memset(&(clienteaddr), 0, sizeof(clienteaddr));

    socklen_t l = sizeof(struct sockaddr_in);

    // Iniciando a dll de sockets windows
    WSAStartup(MAKEWORD(2, 1), &wsaData);

    //criando socket do servidor
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

    //bindando com a porta
    if (bind(socks, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
        printf("Erro ao bindar\n");

    printf("Servidor inicializado com sucesso!\n\n");
    printf("Esperando clientes se conectarem...\n\n");

    //o servidor permite apenas 2 clientes executando ao mesmo tempo e pressupõe que quem vai semear já possui o arquivo
    while (cliente_baixar.online == FALSE || cliente_semear.online == FALSE)
    {

        //recebe pedido de conexão do cliente
        if (recvfrom(socks, &opcao, sizeof(opcao), 0, (struct sockaddr *)&clienteaddr, &l) < 0)
        {
            error("Erro ao receber conexão do cliente");
        }

        // verifica qual tipo de cliente conectou

        if (opcao == 2 && cliente_baixar.online == FALSE)
        {
            cliente_baixar.addr = clienteaddr;
            cliente_baixar.online = TRUE;
            memset(&(clienteaddr), 0, sizeof(clienteaddr));
            printf("Cliente que deseja baixar conectado\n\n");
        }

        else if (opcao == 1 && cliente_semear.online == FALSE)
        {
            cliente_semear.addr = clienteaddr;
            cliente_semear.online = TRUE;
            memset(&(clienteaddr), 0, sizeof(clienteaddr));
            printf("Cliente seeder conectado\n");
        }
        else if (opcao == 2)
        {
            printf("Ja existe um cliente leecher conectado\n");
        }
        else if (opcao == 1)
        {
            printf("já existe um cliente seeder conectado\n");
        }

        if (cliente_baixar.online == TRUE && cliente_semear.online == TRUE)
        {
            // envia informacoes do seeder para leecher
            if (sendto(socks, &(cliente_semear.addr), sizeof(cliente_semear.addr), 0, (struct sockaddr *)&(cliente_baixar.addr), sizeof(cliente_baixar.addr)) < 0)
            {
                error("Erro ao enviar informações do seeder\n");
            }
            printf("Endereco do seeder enviado com sucesso!\n");

            // envia informacoes do leecher para seeder
            if (sendto(socks, &(cliente_baixar.addr), sizeof(cliente_baixar.addr), 0, (struct sockaddr *)&(cliente_semear.addr), sizeof(cliente_semear.addr)) < 0)
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