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

#define PACOTE_BUFFER_SIZE 512
#define MAX_NOME_ARQUIVO 200
#define TRUE 1
#define FALSE 0

typedef struct pacote
{
    char buffer[PACOTE_BUFFER_SIZE]; // buffer
    int seq;                         // numero de sequencia
    int checksum;                    // soma de svrificação
} Pacote;

typedef struct pctAck
{
    int seq; // numero de sequencia
    int nack; // 0 = ack e 1 = nack
} Ack;

// dispara mensagem de erro e finaliza execucao
void error(char *msg)
{
    printf("%s\n", msg);
    exit(1);
}

void calc_checksum(Pacote *pct)
{
    unsigned int i, sum = 0;
    for (i = 0; i < PACOTE_BUFFER_SIZE; i++)
    {
        if (pct->buffer[i] == '1')
            sum += 2 * i;
        else
            sum += i;
    }

    pct->checksum = sum;
}

int verifica_checksum(Pacote *pct)
{
    unsigned int i, sum = 0;
    for (i = 0; i < PACOTE_BUFFER_SIZE; i++)
    {
        if (pct->buffer[i] == '1')
            sum += 2 * i;
        else
            sum += i;
    }

    if (sum == pct->checksum)
        return TRUE;
    else
        return FALSE;
}

void leecher(int socks, struct sockaddr_in *servaddr)
{

    char nome_arquivo[MAX_NOME_ARQUIVO];

    Pacote pacote;
    memset(pacote.buffer, 0, PACOTE_BUFFER_SIZE);

    Ack ack; //ACK a ser enviado
    ack.nack = 0;

    int seq = 0; // Verificador de numero de sequencia
    int checksum_ok = 0;
    int aviso = 2; // Tipo de cliente a informar ao servidor

    socklen_t l = sizeof(struct sockaddr);
    FILE *f;

    struct sockaddr_in semeadoraddr; // Informacoes do cliente semeador

    // avisa servidor que deseja baixar
    if (sendto(socks, &aviso, sizeof(aviso), 0, (struct sockaddr *)servaddr, sizeof(struct sockaddr)) < 0)
    {
        error("Erro ao enviar opcao ao servidor\n");
    }

    printf("Esperando servidor encontrar semeador...\n");

    // Espera servidor retornar informações do semeador
    if (recvfrom(socks, &semeadoraddr, l, 0, (struct sockaddr *)servaddr, &l) < 0)
    {
        error("Erro ao receber pacote\n");
    }

    printf("Servidor encontrou cliente seeder");
    printf("ip: %s\n", inet_ntoa(semeadoraddr.sin_addr));


    printf("Digite o nome do arquivo desejado: ");
    scanf(" %s", nome_arquivo);

    // Envia o nome do arquivo desejado para o seeder
    if (sendto(socks, nome_arquivo, MAX_NOME_ARQUIVO, 0, (struct sockaddr *)&semeadoraddr, sizeof(semeadoraddr)) < 0)
    {
        error("Erro ao enviar o nome do arquivo para o seeder\n");
    }

    // Criação do arquivo para armazenar o arquivo baixado
    
    char new[100] = "Copia de ";
    strcat(new, nome_arquivo);
    f = fopen(new, "wb");

    // Recepção dos pacotes

    while (pacote.seq != -1)
    {
        // Recebe pacote
        if (recvfrom(socks, &pacote, sizeof(Pacote), 0, (struct sockaddr *)&semeadoraddr, &l) < 0)
        {
            error("Erro ao receber pacote!\n");
        }

        // Verifica a soma de verificação
        checksum_ok = verifica_checksum(&pacote);

        // Imprime informações referentes ao pacote recebido
        printf("seq %d, checksum = %d\n", seq, pacote.checksum);

        while (checksum_ok != TRUE)
        {

            if (recvfrom(socks, &pacote, sizeof(Pacote), 0, (struct sockaddr *)&semeadoraddr, &l) < 0)
            {
                printf("Erro no download do arquivo\n");
                exit(1);
            }
            checksum_ok = verifica_checksum(&pacote);
        }

        // Verifica numero de sequencia para ver se pacote estao chegando ordenados
        if (seq == pacote.seq || pacote.seq == -1)
        {
            // Preenche o ack a ser enviado
            ack.seq = seq;
            ack.nack = 0;
            seq++;

            // envia ack
            if (sendto(socks, &ack, sizeof(Ack), 0, (struct sockaddr *)&semeadoraddr, sizeof(semeadoraddr)) < 0)
            {
                error("Erro ao enviar pacote ACK\n");
            }

            //escreve arquivo
            if (fwrite(pacote.buffer, 1, PACOTE_BUFFER_SIZE, f) < 0)
            {
                error("Erro ao escrever arquivo no buffer\n");
            }

            // Reseta pacote 
            memset(pacote.buffer, 0, PACOTE_BUFFER_SIZE);

            // Recebe ultimo pacote
            if (pacote.seq == -1)
                break;
        }

    }
    fclose(f);
}

void seeder(int socks, struct sockaddr_in *servaddr)
{

    char nome_arquivo[MAX_NOME_ARQUIVO];
    memset(nome_arquivo, 0, MAX_NOME_ARQUIVO);

    int aviso = 1;

    struct sockaddr_in baixadoraddr;
    memset(&baixadoraddr, 0, sizeof(baixadoraddr));

    socklen_t l = sizeof(struct sockaddr);
    socklen_t l2 = sizeof(baixadoraddr);

    // Avisa ao servidor que deseja semear
    if (sendto(socks, &aviso, sizeof(aviso), 0, (struct sockaddr *)servaddr, sizeof(struct sockaddr)) < 0)
    {
        error("Erro ao enviar opcao ao servidor\n");
    }

    printf("Aguardando servidor encontrar cliente leecher...\n");

    // espera servidor retornar informações do baixador (leecher)
    if (recvfrom(socks, &baixadoraddr, l2, 0, (struct sockaddr *)servaddr, &l) < 0)
    {
        error("Erro ao receber informacoes do leecher\n");
    }

    printf("Servidor encontrou cliente leecher - IP: %s\n", inet_ntoa(baixadoraddr.sin_addr));

    // recebe nome do arquivo desejado
    if (recvfrom(socks, nome_arquivo, MAX_NOME_ARQUIVO, 0, (struct sockaddr *)&baixadoraddr, &l2) < 0)
    {
        printf("Nome %s\n", nome_arquivo);
        error("Erro ao receber nome do arquivo\n");
    }

    FILE *fp;
    fp = fopen(nome_arquivo, "rb");
    if (fp == NULL)
    {
        printf("O arquivo %s nao existe\n", nome_arquivo);
    }

    // pega tamanho do arquivo
    fseek(fp, 0, SEEK_END);
    double file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    int fr;
    double num = (file_size / PACOTE_BUFFER_SIZE);
    double numPacotes = ceil(num);
    int numPacotesInt = (int)numPacotes;
    printf("Numero de pacotes: %d\n", numPacotesInt);

    int i;
    int seq = 0;
    int verificaAck;
    int verificaSeq;
    Pacote pacote; // pacote a ser enviado

        memset(pacote.buffer, 0, PACOTE_BUFFER_SIZE);

    printf("Iniciando transferencia...\n\n");
    Ack ack; // ack recebido do cliente que baixa

    // inicia envio de pacote
    for (seq = 0; seq < numPacotesInt; seq++)
    {
        if (seq == numPacotesInt - 1)
            pacote.seq = -1;
        else
            pacote.seq = seq;
        
        if ((fr = fread(pacote.buffer, PACOTE_BUFFER_SIZE, 1, fp)) < 0)
        {
            error("Erro ao ler bytes do arquivo\n");
        }

        calc_checksum(&pacote); 

        // envia pacote
        if (sendto(socks, &pacote, sizeof(Pacote), 0, (struct sockaddr *)&baixadoraddr, sizeof(baixadoraddr)) < 0)
        {
            error("Erro ao enviar pacote\n");
        }

        // verifica se cliente recebeu corretamente atraves do ack
        while (1)
        {
            // recebe ack
            if (recvfrom(socks, &ack, sizeof(Ack), 0, (struct sockaddr *)&baixadoraddr, &l) < 0)
            {
                error("Erro ao receber pacote\n");
            }
            
            verificaAck = ack.nack;
            
            // se ack.nack = 1, então reenvia pacote

            if (verificaAck == 1)
            {
                if (sendto(socks, &pacote, sizeof(Pacote), 0, (struct sockaddr *)&baixadoraddr, sizeof(baixadoraddr)) < 0)
                {
                    error("Erro ao reenviar pacote\n");
                }
            }
            
            // se ack esta correto continua o envio;
            else
            {
                break;
            }
        }
        printf("Cliente recebeu pacote\nNumero de sequencia: %d\n\n", pacote.seq);
        memset(pacote.buffer, 0, PACOTE_BUFFER_SIZE);
    }
    fclose(fp);
}

int main(int argc, char *argv[]){

    // CHECANDO A AUSENCIA DE ARGUMENTOS
    if(argc < 3){
        printf("Digite (exemplo): cliente.exe <ip> <opcao (1 - seeder; 2 - leecher)>\n");
        return(1);
    }

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 1), &wsaData);

    int socks;                            // socket do client
    struct sockaddr_in servaddr, cliAddr; // endereco do servidor e do cliente

    LPHOSTENT hostEntry;

    hostEntry = gethostbyname(argv[1]);
    if (hostEntry == NULL){
        printf("Host desconhecido %s\n", argv[1]);
        return 1;
    }

    //VINCULAR A PORTA DO SERVIDOR REMOTO
    servaddr.sin_family = hostEntry->h_addrtype;
    servaddr.sin_addr = *((LPIN_ADDR)*hostEntry->h_addr_list);
    servaddr.sin_port = htons(7802);

    // CRIANDO SOCKET
    socks = socket(AF_INET, SOCK_DGRAM, 0);
    if (socks < 0){
        printf("Socket nao pode ser aberto\n");
        return 1;
    }

    cliAddr.sin_family = AF_INET;
    cliAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    cliAddr.sin_port = htons(0);

    // VINCULAR O SOCKET AO ENDERECO DO CLIENTE

    if ((bind(socks, (struct sockaddr *)&cliAddr, sizeof(cliAddr))) < 0){
        printf("A porta nao pode ser vinculada\n");
        return 1;
    }

    // ENVIAR O CODIGO DE OPERACAO PARA O SERVIDOR

    if (strcmp(argv[2], "1") == 0)
    {
        seeder(socks, &servaddr);
    }
    else if (strcmp(argv[2], "2") == 0)
    {
        leecher(socks, &servaddr);
    }
    else
    {
        printf("Opcao invalida. Tente executar novamente\n");
        return 1;
    }
    
    closesocket(socks);
    WSACleanup();
    return (0);
}
