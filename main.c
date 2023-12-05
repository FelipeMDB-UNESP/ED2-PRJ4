#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#define ARQ_INSERCAO "insere.bin"
#define ARQ_BUSCA "busca.bin"

#define TAM_INSERCAO 13
#define TAM_BUSCA 5
#define TAM_HASH 13

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
    fseek(database,0,SEEK_END);
    int retorno = ftell(database)/sizeof(DADO);
    fclose(database);
    return retorno;
}

//inicializa o hash e, se nao houver, cria
void criar_hash(){
    INDICE indice[TAM_HASH];
    for (int i=0; i<TAM_HASH;i++) {
        strcpy(indice[i].chave.codC,"##");
        strcpy(indice[i].chave.codV,"##");
        indice[i].endereco=-1;
    }

    char c;
    FILE* index = fopen("hash.bin","a+b");
    rewind(index);
    if ((fread(&c,sizeof(char),1,index))==0) {
        fwrite(indice, sizeof(indice),1,index);
    }
    fclose(index);
}

//extrai os dados do arquivo binario insere.bin
void carregar_dados(DADO* buffer) {
    FILE* insere = fopen(ARQ_INSERCAO,"r");
    if (insere != NULL)
        fread(buffer,sizeof(DADO),TAM_INSERCAO,insere);
    fclose(insere);
}

//extrai os dados do arquivo binario busca.bin
void carregar_buscas(CHAVE* buffer) {
    FILE* busca = fopen(ARQ_BUSCA,"r");
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
    return numero%TAM_HASH;
}

//insere o indice no Hash, seguindo o Overflow Progressivo, sem buckets
void inserir_chave(CHAVE* chave) {
    INDICE indice;
    strcpy(indice.chave.codC,chave->codC);
    strcpy(indice.chave.codV,chave->codV);
    indice.endereco=tam_database();

    INDICE aux;
    int cont=0;

    FILE* index = fopen("hash.bin","r+b");

    printf("\n Endereco: %d\n",converter_chave(chave));
    for (int i = converter_chave(chave); true;) {

        fseek(index,(i*sizeof(INDICE)),SEEK_SET);
        fread(&aux,sizeof(INDICE),1,index);
        
        if (aux.endereco == -1) {
            fseek(index,(i*sizeof(INDICE)),SEEK_SET);
            fwrite(&indice,sizeof(INDICE),1,index);
            printf("\n Chave Inserida no Endereco: %d\n",i);
            break;
        } else {
            printf("\n  Colisao\n");
            i++;
            if (i==TAM_HASH)
                i=0;
            printf("\n  Tentativa: %d -> %d\n",++cont,i);
        }
        if (i==converter_chave(chave)) {
            printf("\n  Hash cheio!\n");
            break;
        }
    }
    fclose(index);
}

//procura pela chave no arquivo de hash; se nao existir, o addr vai ser TAM_HASH,
//se existir, o addr sera a posicao real no arquivo
int pesquisar_chave(CHAVE* chave) {

    INDICE indice;
    int addr=TAM_HASH;
    int acessos=0;

    FILE* index = fopen("hash.bin","a+b");
    
    for (int i = converter_chave(chave);;) {
        fseek(index, sizeof(INDICE)*i,SEEK_SET);
        fread(&indice,sizeof(INDICE),1,index);
        if ((strcmp(chave->codC,indice.chave.codC)==0) && (strcmp(chave->codV,indice.chave.codV)==0)) {
            addr=i;
            acessos++;
            break;
        }
        acessos++;
        i++;
        if (i==TAM_HASH)
            i=0;
        if (i==converter_chave(chave))
            break;
    }
    fclose(index);
    return criar_retorno(addr, acessos);
}

//realiza a verificacao e insercao do dado e, juntamente, sua chave
void insercao(DADO* dado) {

    CHAVE chave;
    strcpy(chave.codC,dado->codC);
    strcpy(chave.codV,dado->codV);

    if (pesquisar_chave(&chave)/100==TAM_HASH){
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
                printf("\nQual dado deseja inserir? (1-%d)\n",TAM_INSERCAO);
                scanf(" %d",&escolha_insercao);
                insercao(&pasta[escolha_insercao-1]);
                break;
            case 2:
                printf("\nQual chave deseja buscar? (1-%d)\n", TAM_BUSCA);
                scanf(" %d",&escolha_busca);
                compactado = pesquisar_chave(&chave[escolha_busca-1]);
                if (compactado/100==TAM_HASH) {
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
            case 5:
                for(int i = 0; i<TAM_INSERCAO; i++) {
                    imprimir_registro(&pasta[i]);
                }
                break;
            default:
                printf("\n   [Alerta: Valor Invalido]\n");
        }
    }
}