#include <stdio.h>
#include "tree_3cell.h"
#include "prioqueue.h"
#include "bitset.h"
#include <stdlib.h>
#include <string.h>

/* Node of the huffman tree */
typedef struct {
  int value;
  unsigned char character;
} freqChar;

typedef struct node Node;

void getFrequency(int *frequency, FILE* file);
int compareTrees(VALUE tree1, VALUE tree2);
binary_tree *buildHuffmanTree (int *frequency, int (*compare)(VALUE, VALUE));
void traverseTree(binaryTree_pos pos, binary_tree* huffmanTree, bitset * navPath, bitset *pathArray[]);
void decodeFile(FILE* decodeThis, FILE* output, binary_tree* huffmanTree);

int main(int argc, char **argv){


	/*
	 * Settings Variables
	 */

	int frequency[256];
	int character;
	if(argc <= 2){
		fprintf(stderr, "Usage: input file, output file\n");
		return 0;
	}


    /*
     * validating input args
     */

	FILE* infilep = fopen(argv[1], "rt");
	if(infilep == NULL){
		fprintf(stderr, "Couldn't open input file %s\n", argv[1]);
	}
	
	FILE* outfilep = fopen(argv[2], "w");
	if(outfilep == NULL){
		fprintf(stderr, "Couldn't open output file %s\n", argv[2]);
	}



    /*
     * calculate frequency table
     * build huffman tree
     * build code table
     */


    getFrequency(frequency, infilep);
	binary_tree *tree4 = buildHuffmanTree(frequency, compareTrees);

	bitset *navPath = bitset_empty();
	bitset *pathArray[256];

	traverseTree(binaryTree_root(tree4), tree4, navPath, pathArray );

    // test whether we can access the bitset stored in a bitset array
    for (int iii = 0; iii < 256; iii++) {
        printf("%d\n", pathArray[iii]->length);
    }



	/*
	 * cleaning up
	 */

	fclose(infilep);
	fclose(outfilep);

	return 0;
}

/*
 * getFrequency - calculates a frequency table on an text imput file
 *                using the 256 characters of the extended ASCII table.
 *
 * Parameter:   frequency - pointer to an int array of length 256. Here the
 *                          frequencies will be summed and stored
 *              file      - pointer of type FILE. The input file has to be
 *                          a standard text file.
 */
void getFrequency(int* frequency, FILE* file){
	int finished=0;
	int ch;
	for (ch = 0; ch < 256; ch++){
		frequency[ch] = 0;
	}
	
	while (finished!=1){
		ch = fgetc(file);
		//printf("%d ", ch);
		/* end of file or read error.  EOF is typically -1 */
		if (ch == EOF){
			finished=1;
		}
		/* assuming ASCII; "letters" means "a to z" */
		else{
			frequency[ch]++;
		}
	}
}


/*
 * compareTrees - is the compare function used in the priorityQueue datatype
 *
 * Paramter:    tree1   - pointer to a binary tree datatype
 *              tree2   - pointer to a binary tree dataype
 *
 * Comments:    This function assumes a freqChar struct to be stored as
 *              the label of the binary tree. The actual comparison is done
 *              between the 'value' of each tree's root.
 */
int compareTrees(VALUE tree1, VALUE tree2){
	freqChar tmp1;
	freqChar tmp2;
	tmp1 = *(freqChar*)binaryTree_inspectLabel(tree1, binaryTree_root(tree1));
	tmp2 = *(freqChar*)binaryTree_inspectLabel(tree2, binaryTree_root(tree2));
	if (tmp1.value > tmp2.value){
		return 0;
	}
	else{
		return 1;
	}
	
}


/*
 * buildHuffmanTree:    - This function builds a huffman tree from a frequency table
 *
 * Parameter:           frequency   - a pointer to an int array of length 256 that represents an
 *                                    extended ASCII character frequency table generated by the
 *                                    function getFrequency
 *                      compare     - pointer to a function that compares the root label of the
 *                                    two binary trees. This function will be used as argument
 *                                    for the priority queue datatype.
 *
 *  The function first makes root/leafs for all 256 characters in the extended ASCII table and puts
 *  them in a priority queue (datatype pqueue from prioqueue.c /.h). Then in a while loop, two
 *  elements at a time are removed from the priority queue. And linked into a new binary tree root.
 *  The label of the new tree root contains as value the combined values of the two children. This
 *  is repeated until just one element is left in the priority queue.
 *
 */
binary_tree *buildHuffmanTree (int *frequency, int (*compare)(VALUE, VALUE)){

    pqueue *treebuildingQueue = pqueue_empty (compare);
	//pqueue_setMemHandler(treebuildingQueue, free); // CANT USE THIS CAUSE THE FINISHED TREE WILL BE REMOVED
	//PERHAPS THE PRIO QUEUE SHOULD BE BUILT IN A DIFFERENT FUNCTION?
	int chartmp;
	binary_tree *huffmanTree = binaryTree_create();
	
	/* Create 1 tree for each character and put all of them in a priority queue*/
	for (chartmp = 0; chartmp < 256; chartmp++){
		freqChar *tmp=malloc(sizeof(freqChar));
		tmp->character = chartmp;
		tmp->value = frequency[chartmp];
		binary_tree* treetmp = binaryTree_create();
		binaryTree_setLabel(treetmp, tmp, binaryTree_root(treetmp));
		pqueue_insert(treebuildingQueue, treetmp);
	}
	
	/*While priority queue isn't empty take out the two front values and 
	connect these two trees with a new node (tree), put this new combined tree 
	in the queue.*/
	while(!pqueue_isEmpty(treebuildingQueue)){
		
		/*Create new tree with 1 node.*/
		binary_tree *newTree = binaryTree_create();
		
		/*Take out the first tree from the queue and save the values on its label.*/
		binary_tree *tree1 = pqueue_inspect_first(treebuildingQueue);
		freqChar *freqChartmp1 = binaryTree_inspectLabel(pqueue_inspect_first(treebuildingQueue), binaryTree_root(pqueue_inspect_first(treebuildingQueue)));
		pqueue_delete_first(treebuildingQueue);
		
		/*Initiate struct to put on the new node's label.*/
		freqChar *freqChartmp2 = malloc(sizeof(freqChar));
		
		/*When the last tree has been taken out return that tree*/
		if(pqueue_isEmpty(treebuildingQueue)){
			huffmanTree = tree1;
			//pqueue_free(treebuildingQueue); !!!REMOVES THE TREE!!!
			return huffmanTree;
		}
		else{
			/*Take out the second tree from the queue and save the values on its label.*/
			binary_tree *tree2 = pqueue_inspect_first(treebuildingQueue);
			freqChar *freqChartmp3 = binaryTree_inspectLabel(pqueue_inspect_first(treebuildingQueue), binaryTree_root(pqueue_inspect_first(treebuildingQueue)));
			pqueue_delete_first(treebuildingQueue);
			
			/*Give values to the label.*/
			freqChartmp2->value = freqChartmp1->value + freqChartmp3->value;
			freqChartmp2->character = -1;
			
			/*Set label on the new tree and insert a left and right child.*/
			binaryTree_setLabel(newTree, freqChartmp2, binaryTree_root(newTree));
			binaryTree_insertLeft(newTree, binaryTree_root(newTree));
			binaryTree_insertRight(newTree, binaryTree_root(newTree));
			
			/*Set the two trees from the queue as right/left child on the new node.*/
			newTree->root->rightChild = tree1->root;
			newTree->root->leftChild = tree2->root;
			
			/*Insert the new tree in the queue.*/
			pqueue_insert(treebuildingQueue, newTree);
		}
	}
	pqueue_free(treebuildingQueue);
	return 0;
}

/*
 * traverseTree - function that traverses a binary tree
 *
 * Parameter:   pos     - position where to start the traversal
 *              tree    - pointer to binary tree to traverse
 *
 * This function expects the leafs of the tree to have labels
 * of type freqChar. It will print out both charachter and value
 * of each leaf. Traversal is pre-order.
 */
void traverseTree(binaryTree_pos pos, binary_tree *huffmanTree, bitset * navPath, bitset *pathArray[]){

	int length = navPath->length;

	if(binaryTree_hasLeftChild(huffmanTree, pos)){
		bitset *newPath = bitset_empty();
		for (int iii = 0; iii < length; iii++ ){
			bitset_setBitValue(newPath, iii, bitset_memberOf(navPath,iii));
		}
		bitset_setBitValue(newPath, length, 0);
		traverseTree(binaryTree_leftChild(huffmanTree, pos), huffmanTree, newPath, pathArray);
		//bitset_free(newPath);
	}
	if(binaryTree_hasRightChild(huffmanTree, pos)){
		bitset *newPath = bitset_empty();
		for (int iii = 0; iii < length; iii++ ){
			bitset_setBitValue(newPath, iii, bitset_memberOf(navPath,iii));
		}
		bitset_setBitValue(newPath, length, 1);
		traverseTree(binaryTree_rightChild(huffmanTree, pos), huffmanTree, newPath, pathArray);
		//bitset_free(newPath);
	}
	
	/*If current position does not have a left or right child print the character*/
	if(!binaryTree_hasLeftChild(huffmanTree, pos)&&!binaryTree_hasRightChild(huffmanTree, pos)){
		freqChar* tmp = binaryTree_inspectLabel(huffmanTree, pos);
		//printf("%d\n", tmp->character);
		pathArray[(int)tmp->character] = navPath;

        // diagnostic screen output, to be removed later
        printf("%c : %d : ", tmp->character, tmp->value);
		for (int iii = 0; iii < length; iii++){
			printf("%d", bitset_memberOf(navPath, iii) );
		}
		printf("\n");

	}

	
}
void decodeFile(FILE* decodeThis, FILE* output, binary_tree* huffmanTree){
	int finished = 0;
	int failed = 0;
	char tmp;
	freqChar* tmp2;
	binaryTree_pos treePos = binaryTree_root(huffmanTree);
	while (finished!=1){
		while((binaryTree_hasLeftChild(huffmanTree, treePos)||binaryTree_hasRightChild(huffmanTree, treePos))&&finished!=1){
			tmp = fgetc(decodeThis);
			if (tmp == EOF){
				finished = 1;
			}
			else if (tmp == '0'){
				treePos = binaryTree_leftChild(huffmanTree, treePos);
			}
			else if (tmp == '1'){
				treePos = binaryTree_rightChild(huffmanTree, treePos);
			}
			else{
				finished = 1;
				failed = 1;
				fprintf(stderr, "ERROR: Unknown binary sequence, decoding failed.\n");
			}
		}
		if(finished != 1){
			tmp2 = (freqChar*)binaryTree_inspectLabel(huffmanTree, treePos);
			printf("%c\n", tmp2->character);
			fprintf(output, "%c", tmp2->character);
			treePos = binaryTree_root(huffmanTree);
		}
	}
	if(failed == 0){
		printf("File decoded successfully!\n");
	}
}