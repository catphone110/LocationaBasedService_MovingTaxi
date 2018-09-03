#include "server/grid/lbs_hashtable.h"

#include <stdio.h>
#include <stdlib.h>

//scp -r sx3@123.57.237.171:~/lbs .


//初始化
int lbs_hashtable_init(lbs_hashtable_t* lbs_hashtable){
  int capacity = 19997;
  lbs_hashtable->hash_nodes = (lbs_hashnode_t *)malloc(capacity*sizeof(lbs_hashnode_s));
  lbs_hashtable->size = 0;
  lbs_hashtable->capacity = capacity;
  for(int i = 0; i<lbs_hashtable->capacity; i++){
      lbs_queue_init(&(lbs_hashtable->hash_nodes[i].queue));
  }
  return 0;
}
//
int lbs_hashtable_destroy(lbs_hashtable_t* lbs_hashtable){
  return 0;
}

int lbs_hashtable_set(lbs_hashtable_t * lbs_hashtable,
                      uint32_t id, lbs_mov_node_t * lbs_mov_node, int cell_id){

  lbs_hashnode_t * hashnode = (lbs_hashnode_t *) malloc(sizeof(lbs_hashnode_t));
  lbs_queue_init(&(hashnode->queue));
  hashnode->mov_node = lbs_mov_node;
  hashnode->cell_id = cell_id;

  int index = id%(lbs_hashtable->capacity);
  lbs_queue_insert_head(&(lbs_hashtable->hash_nodes[index].queue), &(hashnode->queue));

  return 0;
}

lbs_hashnode_t * lbs_hashtable_get(lbs_hashtable_t * lbs_hashtable, uint32_t id){
  int index = id%(lbs_hashtable->capacity);
  lbs_hashnode_t * temp = (lbs_hashnode_t*) (lbs_hashtable->hash_nodes[index].queue.next);
  while(temp != (lbs_hashtable->hash_nodes + index)){
      if (temp->mov_node->id == id){
         return temp;
      }
      temp = (lbs_hashnode_t*) (temp->queue.next);
  }
  return NULL;
  //lbs_queue_insert_head(lbs_hashtable->hash_nodes[index]->queue, hash_nodes->queue);

}
