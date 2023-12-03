#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#define TAM_INSERCAO 8
#define TAM_BUSCA 5

typedef struct {
    char codC[3];
    char codV[3];
    char nomeC[50];
    char nomeV[50];
    char qtdDias[4];
} DADO;

typedef struct {
    char codC[3];
    char codV[3];
} CHAVE;

typedef struct {
    CHAVE chave;
    int endereco;
} INDICE;

//quant de registros no arquivo de dados
int tam_database() {
    FILE* database = fopen("database.bin","a+b");
    int retorno = ftell(database)/sizeof(DADO);
    fclose(database);
    return retorno;
}

//inicializa o hash e, se nao houver, cria
void criar_hash(){
    INDICE indice[TAM_INSERCAO];
    for (int i=0; i<TAM_INSERCAO;i++) {
        strcpy(indice[i].chave.codC,"##");
        strcpy(indice[i].chave.codV,"##");
        indice[i].endereco=-1;
    }

    char c;
    FILE* index = fopen("hash.bin","a+b");
    if ((fread(&c,sizeof(char),1,index))==0) {
        fwrite(indice, sizeof(indice),1,index);
    }
    fclose(index);
}

//extrai os dados do arquivo binario insere.bin
void carregar_dados(DADO* buffer) {
    FILE* insere = fopen("insere.bin","r");
    if (insere != NULL)
        fread(buffer,sizeof(DADO),TAM_INSERCAO,insere);
    fclose(insere);
}

//extrai os dados do arquivo binario busca.bin
void carregar_buscas(CHAVE* buffer) {
    FILE* busca = fopen("busca.bin","r");
    if (busca != NULL)
        fread(buffer,sizeof(CHAVE),TAM_BUSCA,busca);
    fclose(busca);
}

//carrega os arquivos dados pelo professor
void carrega_arquivos(DADO* pasta,CHAVE* chave) {
    carregar_dados(pasta);
    carregar_buscas(chave);
}

//insere o registro no arquivo de dados
void inserir_registro(DADO* dado) {
    FILE* database = fopen("database.bin","a+b");
    fwrite(dado,sizeof(DADO),1,database);
    fclose(database);
}

//impressao de um registro
void imprimir_registro(DADO* dado) {
    printf("\n+ Dados do Registro:\n|-Codigo do Cliente: %s\n|-Codigo do Veiculo: %s\n|-Nome do Cliente: %s\n|-Nome do Veiculo: %s\n|-Quantidade de Dias: %s\n#\n",dado->codC,dado->codV,dado->nomeC,dado->nomeV,dado->qtdDias);
}

//usa o addr do registro no arquivo de hash e imprime o registro que esta la
void achar_registro(int addr) {
    INDICE indice;
    DADO dado;

    //resgata o indice no arquivo hash
    FILE* index = fopen("hash.bin","a+b");
    fseek(index,addr*sizeof(INDICE),SEEK_SET);
    fread(&indice,sizeof(INDICE),1,index);
    fclose(index);

    //usa o endereco do indice para extrair o registro
    FILE* database = fopen("database.bin","a+b");
    fseek(database,(indice.endereco)*sizeof(DADO),SEEK_SET);
    fread(&dado,sizeof(DADO),1,database);
    fclose(database);

    //imprime o registro
    imprimir_registro(&dado);
}

//cria o codigo de retorno numerico, que compacta addr e acessos
int criar_retorno(int addr, int acessos) {
    return addr*100+acessos;
}

//converte a chave na posicao do indice na tabela hash
int converter_chave(CHAVE* chave) {
    char res[5];
    strcpy(res,chave->codC);
    strcat(res,chave->codV);
    int numero = atoi(res);
    return numero%13;
}

//insere o indice no Hash, seguindo o Overflow Progressivo, sem buckets
void inserir_chave(CHAVE* chave) {
    INDICE indice = {chave->codC, chave->codV,tam_database()};
    FILE* index = fopen("hash.bin","a+b");
    rewind(index);






    fclose(index);
}

//procura pela chave no arquivo de hash; se nao existir, o addr vai ser TAM_INSERCAO,
//se existir, o addr sera a posicao real no arquivo
int pesquisar_chave(CHAVE* chave) {

    int pos_estimada = converter_chave(chave);
    int addr=TAM_INSERCAO;
    int acessos=0;

    FILE* index = fopen("hash.bin","a+b");
    
    fseek(index, sizeof(INDICE)*pos_estimada,SEEK_SET);




    fclose(index);
    return criar_retorno(addr, acessos);
}

//realiza a verificacao e insercao do dado e, juntamente, sua chave
void insercao(DADO* dado) {

    CHAVE chave = {dado->codC, dado->codV};

    if (pesquisar_chave(&chave)/100==TAM_INSERCAO){
        inserir_chave(&chave);
        inserir_registro(dado);
    } else {
        printf("\nDado ja existente!\n");
    }
}

int main() {

    DADO pasta[TAM_INSERCAO];
    CHAVE chave[TAM_BUSCA];
    carrega_arquivos(&pasta,&chave);
    criar_hash();

    int escolha_insercao;
    int escolha_busca;
    int compactado;
    int opcao;

    //menu
    while (true) {
        printf("\n++===========Menu:==========++\n||1-Inserir                 ||\n||2-Pesquisar Chave Primaria||\n||3-Carregar Arquivos       ||\n||4-Sair                    ||\n++==========================++\n");
        scanf(" %d",&opcao);
        switch (opcao) {
            case 1:
                printf("\nQual dado deseja inserir? (1-%d)",TAM_INSERCAO);
                scanf(" %d",&escolha_insercao);
                insercao(&pasta[escolha_insercao-1]);
                break;
            case 2:
                printf("\nQual chave deseja buscar? (1-%d)", TAM_BUSCA);
                scanf(" %d",&escolha_busca);
                compactado = pesquisar_chave(&chave[escolha_busca-1]);
                if (compactado/100==TAM_INSERCAO) {
                    printf("Chave nao encontrada: h(x): %d | acessos: %d", converter_chave(&chave[escolha_busca-1]), compactado%100);
                } else {
                    printf("Chave encontrada: h(x): %d | addr: %d | acessos: %d", converter_chave(&chave[escolha_busca-1]),compactado/100,compactado%100);
                    achar_registro(compactado/100);
                }
                break;
            case 3:
                carrega_arquivos(&pasta,&chave);
                break;
            case 4:
                exit(0);
            default:
                printf("\n   [Alerta: Valor Invalido]\n");
        }
    }
}