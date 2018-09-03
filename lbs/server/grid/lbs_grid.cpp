#include "server/grid/lbs_grid.h"

#include <stdio.h>
#include <stdlib.h>

//scp -r sx3@123.57.237.171:~/lbs .
//scp lbs_grid.cpp sx3@123.57.237.171:~/lbs/server/grid
int lbs_grid_init(lbs_grid_t* lbs_grid, double lon1, double lon2, double lat1, double lat2, int row_num, int col_num){
  //lbs_grid_t * lbs_grid = (lbs_grid_t *) malloc(sizeof(lbs_grid_t));
  lbs_grid->row_num = row_num;
  lbs_grid->col_num = col_num;

  lbs_grid->cell_width = (lon2 - lon1)/col_num;
  lbs_grid->cell_height = (lat2 - lat1)/row_num;

  lbs_grid->lon_min = lon1;
  lbs_grid->lat_min = lat1;

  lbs_hashtable_init(&lbs_grid->hash_table);
  lbs_grid->cell = (lbs_cell_t *)malloc(sizeof(lbs_cell_t)*col_num*row_num);
  for (int i = 0; i<col_num*row_num; i++){
    lbs_queue_init(&(lbs_grid->cell[i].dammy_node.queue));
  }
  return 0;
}


//网格的删除
int lbs_grid_destroy(lbs_grid_t * lbs_grid) {
    lbs_hashtable_destroy(&(lbs_grid->hash_table));
    free(lbs_grid->cell);
}


//更新移动位置
int lbs_grid_update(lbs_grid_t* lbs_grid, double lon, double lat, uint64_t timestamp, uint32_t id){
  //先搜找hashtable
  lbs_hashnode_t * hashnode = lbs_hashtable_get(&(lbs_grid->hash_table), id);
  int new_cell_row = lbs_grid_cell_row(lbs_grid, lat);
  int new_cell_col = lbs_grid_cell_col(lbs_grid, lon);
  int new_cellId = lbs_grid_cell_id(lbs_grid, new_cell_row, new_cell_col);

  //case1:如果空就加个新的
  if(hashnode == NULL){
    //加新的 lbs_mov_node_t
    lbs_mov_node_t * new_lbs_mov_node = (lbs_mov_node_t*) (malloc(sizeof(lbs_mov_node_t)));
    lbs_queue_init(&(new_lbs_mov_node->queue));
    new_lbs_mov_node->lon = lon;
    new_lbs_mov_node->lat = lat;
    new_lbs_mov_node->id = id;
    new_lbs_mov_node->timestamp = timestamp;
    //开新的hashnode
    int ret = lbs_hashtable_set(&(lbs_grid->hash_table), id, new_lbs_mov_node, new_cellId);
    hashnode = lbs_hashtable_get(&(lbs_grid->hash_table), id);
    lbs_cell_t* curr_cell = lbs_grid_cell(lbs_grid, new_cellId);
    lbs_queue_insert_head(&(curr_cell->dammy_node.queue), &(new_lbs_mov_node->queue));
    return 0;
  }

  //case2: 如果需要动地方
  else if(hashnode->cell_id != new_cellId){
    hashnode->cell_id = new_cellId;
    hashnode->mov_node->lon = lon;
    hashnode->mov_node->lat = lat;
    hashnode->mov_node->timestamp = timestamp;
    lbs_queue_remove(&(hashnode->mov_node->queue));
    lbs_cell_t* curr_cell = lbs_grid_cell(lbs_grid, new_cellId);
    lbs_queue_insert_head(&(curr_cell->dammy_node.queue), &(hashnode->mov_node->queue));
  }
  //如果不需要动地方
  else{ //hashnode->cell_id == new_cellId
    hashnode->mov_node->lon = lon;
    hashnode->mov_node->lat = lat;
    hashnode->mov_node->timestamp = timestamp;
  }
  return 0;
}
//计算cell row
int lbs_grid_cell_row(lbs_grid_t* lbs_grid, double lat) {
  int row = (lat - (lbs_grid->lat_min)) / (lbs_grid->cell_height);
  return row;
}

//计算cell col
int lbs_grid_cell_col(lbs_grid_t* lbs_grid, double lon) {
  int col = (lon - (lbs_grid->lon_min)) / (lbs_grid->cell_width);
  return col;
}

//计算cell id
int lbs_grid_cell_id(lbs_grid_t* lbs_grid, int cell_row, int cell_col) {
  int id = (lbs_grid->col_num) * cell_row + cell_col;
  return id;
}

//计算row 和 col
void lbs_grid_cell_row_col(lbs_grid_t* lbs_grid, int cell_id, int* cell_row, int* cell_col) {
  *cell_row = cell_id / lbs_grid->col_num;
  *cell_col = cell_id % lbs_grid->col_num;
}

//获取cell id 里面的 cell
lbs_cell_t* lbs_grid_cell(lbs_grid_t* lbs_grid, int cell_id){
  if (lbs_grid!= NULL){
    return &(lbs_grid->cell[cell_id]);
  }
}
