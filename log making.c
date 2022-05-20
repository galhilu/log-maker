#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct buyInfo
{
    char chem_name[41];
    char chem_code[21];
    long quantity;
    long cost;
} buyInfo;
typedef struct invInfo
{
    char chem_name[41];
    char chem_code[21];
    long quantity;
} invInfo;
typedef struct chemNode
{
    invInfo data;
    struct chemNode *left;
    struct chemNode *right;
} chemNode;
typedef struct chemTree
{
    double balance;
    chemNode  *root;
} chemTree;

chemNode* make_node_a(char chem_name[], char chem_code[], long quantity) {
    chemNode *new_node;
    new_node = (chemNode*)malloc(sizeof(chemNode));
    new_node->right = new_node->left = NULL;
    strcpy(new_node->data.chem_name, chem_name);
    strcpy(new_node->data.chem_code, chem_code);
    new_node->data.quantity = quantity;
    return new_node;
}

chemNode* make_node(FILE* inv) {
    char chem_name[41];
    char chem_code[21];
    double quantity;
    fscanf(inv, "%s", chem_name);
    fscanf(inv, "%s", chem_code);
    fscanf(inv, "%lf ", &quantity);
    return make_node_a(chem_name, chem_code, quantity);
}

void insert(chemNode** root, chemNode* node) {
    if (*root == NULL) {
        *root = node; //get new node for tree
        return;
    }
    if (strcmp((*root)->data.chem_code, node->data.chem_code) == 0)
        return; //no duplicates
    if (strcmp((*root)->data.chem_code, node->data.chem_code) > 0)
        insert(&(*root)->left, node);
    else
        insert(&(*root)->right, node);
    return;
}
void deleteTree(chemNode* node) {
    if (node == NULL)
        return;
    deleteTree(node->left);
    deleteTree(node->right);
    free(node);
    return;
}

void printmenu(chemTree *invTree) {
    printf("Welcome to Chem-R-Us LTD database. Current company balance is %lf.\n", invTree->balance);
    printf("Please select an option from the following menu\n");
    printf("Please enter choice.\n");
    printf("0) Quit.\n");
    printf("1) Initialize company balance and inventory and error report files.\n");
    printf("2) Save current Inventory and error status to files.\n");
    printf("3) Record a sale.\n");
    printf("4) Record a purchase.\n\n\n");
    return;
}

int emptyInv(chemTree *inv) {
    if (inv->root == NULL && inv->balance == 0)
        return 1;
    return 0;
}

void initInventory(char *invFileName, double bal, chemTree *invTree) {
    FILE *inv;
    inv = fopen(invFileName, "r");
    if (inv == NULL) {
        printf("Error in opening inventory file. Inventory tree initialized as empty.\n");
        return;
    }
    if (invTree->root != NULL)
        deleteTree(invTree->root);
    chemNode *r = make_node(inv);
    invTree->root = r;
    invTree->balance = bal;
    chemNode * node;
    while (!feof(inv)) {
        node = make_node(inv);
        insert(&r, node);
    }
    fclose(inv);
}

void recinv(FILE *file, chemNode * root) {
    if (root == NULL)
        return;
    recinv(file, root->left);
    fprintf(file, "%s ", root->data.chem_name);
    fprintf(file, "%s ", root->data.chem_code);
    fprintf(file, "%ld\n", root->data.quantity);
    recinv(file, root->right);
    return;
}
int saveInventory(char* invFileName, char* errFileName, chemTree* currInv, FILE *curErr) {
    if (curErr == NULL) {
        printf("Failed to open sale file for reading.\n");
        return 0;
    }
    FILE* savefile;
    savefile = fopen(invFileName, "w");
    recinv(savefile, currInv->root);
    fclose(savefile);
    FILE *errfile;
    errfile = fopen(errFileName, "w");
    char c;
    while ((c = fgetc(curErr)) != EOF)
        fputc(c, errfile);

    fclose(errfile);
    return 1;
}
chemNode* search(char code[], chemNode ** root) {
    if (*root == NULL)
        return NULL;
    if (strcmp(code, (*root)->data.chem_code) == 0)
        return *root;
    if (strcmp(code, (*root)->data.chem_code) > 0) {
        if ((*root)->right != NULL)
            return search(code, &(*root)->right);
        else
            return NULL;
    }
    if (strcmp(code, (*root)->data.chem_code) < 0) {
        if ((*root)->left != NULL)
            return search(code, &(*root)->left);
        else
            return NULL;
    }
    return NULL;
}

void makeSale(char* saleFileName, chemTree* currInv, FILE* currErr) {
    char seller[200];
    char code[21];
    chemNode *node;
    long quantity;
    double sellprice, tempbal=0;
    int a;
    printf("Please enter name of sales representative (no spaces):\n");
    fscanf(stdin, "%s", seller);
    FILE * sale;
    sale = fopen(saleFileName, "r");
    if (sale == NULL) {
        printf("Failed to open sale file for reading.");
        return;
    }
    while (!feof(sale)) {
        fscanf(sale, "%s", code);
        fscanf(sale, "%ld ", &quantity);
        fscanf(sale, "%lf ", &sellprice);
        node = search(code, &currInv->root);
        if (node == NULL || node->data.quantity < quantity) {
            a = fprintf(currErr, "%s %s %ld\n", code, seller, quantity);
            continue;
        }
        node->data.quantity -= quantity;
        tempbal += sellprice;
    }
    currInv->balance += tempbal;
    return;
}

void makePurchase(struct buyInfo *buy, chemTree *currInv) {
    chemNode **inv = &currInv->root;
    chemNode *newChem = search(buy->chem_code, inv);
    chemNode *newNode;
    currInv->balance -= buy->cost;
    if (newChem == NULL) {
        newNode = make_node_a(buy->chem_name, buy->chem_code, buy->quantity);
        insert(&currInv->root, newNode);
    }
    else
        newChem->data.quantity += buy->quantity;
    return;
}



int main() {
    int x = 0;
    chemTree invTree;
    double bal=0.0;
    invTree.root = NULL;
    invTree.balance = 0;
    char invFileName[200];
    char errFileName[200];
    char saleFileName[200];
    char newerrFileName[200];
    buyInfo newBuy;
    FILE *currerr;
    FILE *copyerr;
    while (1) {
        printmenu(&invTree);
        fscanf(stdin, "%d", &x);
        getchar();

        if (x == 0) {
            deleteTree(invTree.root);
            break;
        }

        if (x == 1) {
            printf("Please enter inventory file name:");
            fscanf(stdin, "%s", invFileName);
            printf("\nPlease enter balance value:");
            fscanf(stdin, "%lf", &bal);
            printf("\nPlease enter error file name:\n");
            fscanf(stdin, "%s", errFileName);
            initInventory(invFileName, bal, &invTree);
            continue;
        }

        if (x == 2) {
            if (emptyInv(&invTree))
                printf("inventory not initialized yet. Initialize before choosing any other option.\n");
            else {
                printf("Please enter name of file for saving current inventory status:");
                fscanf(stdin, "%s", invFileName);
                printf("\nPlease enter name of file for saving error status:\n");
                fscanf(stdin, "%s", newerrFileName);
                currerr = fopen(errFileName, "r");
                saveInventory(invFileName, newerrFileName, &invTree, currerr);
                fclose(currerr);
                continue;
            }
        }

        if (x == 3) {
            if (emptyInv(&invTree))
                printf("inventory not initialized yet. Initialize before choosing any other option.\n");
            else {
                printf("Please enter name of sale file:\n");
                fscanf(stdin, "%s", saleFileName);
                currerr = fopen(errFileName, "a");
                makeSale(saleFileName, &invTree, currerr);
                fclose(currerr);
                continue;
            }
        }

        if (x == 4) {
            if (emptyInv(&invTree))
                printf("inventory not initialized yet. Initialize before choosing any other option.\n");
            else {
                buyInfo *bp = &newBuy;
                printf("Please insert chemical name:");
                scanf("%s", bp->chem_name);
                printf("\nPlease insert chemical code:");
                scanf("%s", bp->chem_code);
                printf("\nPlease insert chemical quantity:");
                scanf("%ld", &bp->quantity);
                printf("\nPlease insert chemical cost:\n");
                scanf("%ld", &bp->cost);
                makePurchase(bp, &invTree);
                continue;
            }
        }
        printf("Command not recognized. Try again.\n");
    }
    return 0;
}
