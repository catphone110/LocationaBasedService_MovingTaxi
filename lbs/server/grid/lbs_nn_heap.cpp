#include "server/grid/lbs_nn_heap.h"

#include <stdlib.h>
#include <string.h>

int lbs_nnheap_init(lbs_nnheap_t* lbs_nnheap) {
	lbs_nnheap->size = 0;
	lbs_nnheap->capacity = 64;
	lbs_nnheap->heap_nodes = (lbs_heapnode_t *) malloc(lbs_nnheap->capacity * sizeof(lbs_heapnode_t));
	return 0;
}

int lbs_nnheap_destroy(lbs_nnheap_t* lbs_nnheap) {
	lbs_nnheap->size = 0;
	lbs_nnheap->capacity = 0;
	free(lbs_nnheap->heap_nodes);
	return 0;
}

void lbs_nnheap_heapify_up_down(lbs_nnheap_t *lbs_nnheap, int index) {
	int left = 2 * index + 1;
	int right = left + 1;
	int smallest = index;

	if (left < lbs_nnheap->size && lbs_nnheap->heap_nodes[left].distance < lbs_nnheap->heap_nodes[index].distance) {
		smallest = left;
	}

	if (right < lbs_nnheap->size && lbs_nnheap->heap_nodes[right].distance < lbs_nnheap->heap_nodes[smallest].distance) {
		smallest = right;
	}

	if (index != smallest) {
		lbs_heapnode_t temp;
		memcpy(&temp, lbs_nnheap->heap_nodes + index, sizeof(lbs_heapnode_t));
		memcpy(lbs_nnheap->heap_nodes + index, lbs_nnheap->heap_nodes + smallest, sizeof(lbs_heapnode_t));
		memcpy(lbs_nnheap->heap_nodes + smallest, &temp, sizeof(lbs_heapnode_t));

		lbs_nnheap_heapify_up_down(lbs_nnheap, smallest);
	}
}

void lbs_nnheap_heapify_down_up(lbs_nnheap_t *lbs_nnheap, int index) {
	if (index <= 0) return;
	int parent = (index + 1) / 2 - 1;

	if (lbs_nnheap->heap_nodes[index].distance < lbs_nnheap->heap_nodes[parent].distance) {
		lbs_heapnode_t temp;
		memcpy(&temp, lbs_nnheap->heap_nodes + index, sizeof(lbs_heapnode_t));
		memcpy(lbs_nnheap->heap_nodes + index, lbs_nnheap->heap_nodes + parent, sizeof(lbs_heapnode_t));
		memcpy(lbs_nnheap->heap_nodes + parent, &temp, sizeof(lbs_heapnode_t));

		lbs_nnheap_heapify_down_up(lbs_nnheap, parent);
	}
}

int lbs_nnheap_insert(lbs_nnheap_t* lbs_nnheap, lbs_mov_node_t* lbs_mov_node, 
					  int cell_id, uint8_t is_grid, double distance) {
	if (lbs_nnheap->size == lbs_nnheap->capacity) {
		lbs_nnheap->capacity *= 2;
		lbs_nnheap->heap_nodes = (lbs_heapnode_t *) realloc(lbs_nnheap->heap_nodes,
															lbs_nnheap->capacity * sizeof(lbs_heapnode_t));
	}
	
	lbs_heapnode_t *new_node = lbs_nnheap->heap_nodes + lbs_nnheap->size;
	new_node->distance = distance;
	new_node->is_grid = is_grid;
	new_node->cell_id = cell_id;
	new_node->node = lbs_mov_node;

	lbs_nnheap->size++;

	lbs_nnheap_heapify_down_up(lbs_nnheap, lbs_nnheap->size - 1);
}

lbs_heapnode_t *lbs_nnheap_top(lbs_nnheap_t *lbs_nnheap) {
	return lbs_nnheap->heap_nodes;
}

void lbs_nnheap_pop(lbs_nnheap_t *lbs_nnheap) {
	memcpy(lbs_nnheap->heap_nodes, lbs_nnheap->heap_nodes + (lbs_nnheap->size-1), sizeof(lbs_heapnode_t));
	lbs_nnheap->size--;
	lbs_nnheap_heapify_up_down(lbs_nnheap, 0);
}