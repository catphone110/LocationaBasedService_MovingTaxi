#include "server/grid/lbs_index.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/log.h"

#include "server/grid/lbs_distance.h"
#include "server/grid/lbs_grid.h"
#include "server/grid/lbs_bitmap.h"
#include "server/grid/lbs_nn_heap.h"

#define LBS_LON_MIN 116
#define LBS_LON_MAX 117
#define LBS_LAT_MIN 39
#define LBS_LAT_MAX 41

#define LBS_ROW_NUM 200
#define LBS_COL_NUM 100

static lbs_grid_t lbs_grid;

// 初始化网格索引
int lbs_grid_index_init() {
  // TODO: by student
  int ret = lbs_grid_init(&lbs_grid, LBS_LON_MIN, LBS_LON_MAX, LBS_LAT_MIN, LBS_LAT_MAX, LBS_ROW_NUM, LBS_COL_NUM);
  return 0;
}

// 更新接口[出租车位置更新]
int lbs_grid_index_update(double lon,
                           double lat,
                           uint64_t timestamp,
                           uint32_t id) {
  // TODO: by student
  int ret = lbs_grid_update(&lbs_grid, lon, lat, timestamp, id);
  return 0;
}

// 范围查询接口[查询某一范围内的所有出租车信息]
int lbs_grid_index_range_query(double lon1,
                               double lon2,
                               double lat1,
                               double lat2,
                               lbs_res_node_t* out) {
  int cell_row_min = lbs_grid_cell_row(&lbs_grid, lat1);
  int cell_row_max = lbs_grid_cell_row(&lbs_grid, lat2);
  int cell_col_min = lbs_grid_cell_col(&lbs_grid, lon1);
  int cell_col_max = lbs_grid_cell_col(&lbs_grid, lon2);
  for (int r = cell_row_min; r <= cell_row_max; r++){
    for (int c = cell_col_min; c<=cell_col_max; c++){
      int curr_cell_id = lbs_grid_cell_id(&lbs_grid, r, c);
        lbs_queue_t* head_queue = &(lbs_grid.cell[curr_cell_id].dammy_node.queue);
        lbs_queue_t* temp_queue = (lbs_grid.cell[curr_cell_id].dammy_node.queue).next;
        while (temp_queue != head_queue){
          lbs_mov_node_t * temp_mov_node = (lbs_mov_node_t *) temp_queue;
          // get info inside 以及判断
          if(temp_mov_node->lon >= lon1 && temp_mov_node->lon <= lon2 &&
             temp_mov_node->lat >= lat1 && temp_mov_node->lat <= lat2){
               // 符合条件，做一个deepcopy然后加进 out list
              lbs_res_node_t * deepcopy_mov_node = (lbs_res_node_t *) malloc(sizeof(lbs_res_node_t));
              memcpy(deepcopy_mov_node, temp_mov_node, sizeof(lbs_res_node_t));
              lbs_queue_insert_head(&(out->queue), &(deepcopy_mov_node->queue));
          }
          temp_queue = (lbs_queue_t *)temp_mov_node->queue.next;
        }//end while
    }
  }
  return 0;
}

double lbs_cell_min_distance(lbs_grid_t* lbs_grid, double src_lon, double src_lat, int dis_cell_id) {
  double grid_distance;
  double des_lon;
  double des_lat;
  int src_cell_row = lbs_grid_cell_row(lbs_grid, src_lat);
  int src_cell_col = lbs_grid_cell_col(lbs_grid, src_lon);
  int des_cell_row;
  int des_cell_col;
  lbs_grid_cell_row_col(lbs_grid, dis_cell_id, &des_cell_row, &des_cell_col);
  //左上的格子
  if ((des_cell_row > src_cell_row) && (des_cell_col < src_cell_col)){
    des_lon = LBS_LON_MIN + (des_cell_col + 1) * lbs_grid->cell_width;
    des_lat = LBS_LAT_MIN + (des_cell_row) * lbs_grid->cell_height;
    grid_distance = lbs_distance(src_lon, src_lat, des_lon, des_lat);
    }
  //直上的格子
  else if ((des_cell_row > src_cell_row) && (des_cell_col == src_cell_col)){
    des_lon = src_lon;
    des_lat = LBS_LAT_MIN + (des_cell_row) * lbs_grid->cell_height;
    grid_distance = lbs_distance(src_lon, src_lat, des_lon, des_lat);
    }
  //右上的格子
  else if ((des_cell_row > src_cell_row) && (des_cell_col > src_cell_col)){
    des_lon = LBS_LON_MIN + (des_cell_col) * lbs_grid->cell_width;
    des_lat = LBS_LAT_MIN + (des_cell_row) * lbs_grid->cell_height; 
    grid_distance = lbs_distance(src_lon, src_lat, des_lon, des_lat);
    }
  //直左的格子
  else if ((des_cell_row == src_cell_row) && (des_cell_col < src_cell_col)){
    des_lon = LBS_LON_MIN + (des_cell_col + 1) * lbs_grid->cell_width;
    des_lat = src_lat;
    grid_distance = lbs_distance(src_lon, src_lat, des_lon, des_lat);
    }
  //直右的格子
  else if ((des_cell_row == src_cell_row) && (des_cell_col > src_cell_col)){
    des_lon = LBS_LON_MIN + (des_cell_col) * lbs_grid->cell_width;
    des_lat = src_lat;
    grid_distance = lbs_distance(src_lon, src_lat, des_lon, des_lat);    
    }
  //本身的格子
  else if ((des_cell_row == src_cell_row) && (des_cell_col == src_cell_col)){
    grid_distance = 0;    
    }
  //左下的格子
  else if ((des_cell_row < src_cell_row) && (des_cell_col < src_cell_col)){
    des_lon = LBS_LON_MIN + (des_cell_col + 1) * lbs_grid->cell_width;
    des_lat = LBS_LAT_MIN + (des_cell_row + 1) * lbs_grid->cell_height;
    grid_distance = lbs_distance(src_lon, src_lat, des_lon, des_lat);  
    }
  //直下的格子
  else if ((des_cell_row < src_cell_row) && (des_cell_col == src_cell_col)){
    des_lon = src_lon;
    des_lat = LBS_LAT_MIN + (des_cell_row + 1) * lbs_grid->cell_height;
    grid_distance = lbs_distance(src_lon, src_lat, des_lon, des_lat);   
    }
  //右下的格子
  else{
    des_lon = LBS_LON_MIN + (des_cell_col) * lbs_grid->cell_width;
    des_lat = LBS_LAT_MIN + (des_cell_row + 1) * lbs_grid->cell_height;
    grid_distance = lbs_distance(src_lon, src_lat, des_lon, des_lat);  
    }
  return grid_distance;
}

// NN查询接口[查询离lon,lat最近的出租车]
int lbs_grid_index_nn_query(double lon, double lat, lbs_res_node_t* out) {
  // TODO: by student
  // 这个out就是list of 符合条件的车
  lbs_nnheap_t * nnheap = (lbs_nnheap_t *) malloc(sizeof(lbs_nnheap_t));
  int ret = lbs_nnheap_init(nnheap);
  lbs_bitmap_t * bitmap = (lbs_bitmap_t *) malloc(sizeof(lbs_bitmap_t));
  ret = lbs_bitmap_init(bitmap, lbs_grid.row_num*lbs_grid.col_num);

  int curr_row = lbs_grid_cell_row(&lbs_grid, lat);
  int curr_col = lbs_grid_cell_col(&lbs_grid, lon);
  int curr_cell_id = lbs_grid_cell_id(&lbs_grid, curr_row, curr_col);

  //push first cells:
  ret = lbs_nnheap_insert(nnheap, &(lbs_grid.cell[curr_cell_id].dammy_node),
                              curr_cell_id, 1, 0);
  ret = lbs_bitmap_setbit(bitmap, curr_cell_id);




  while(lbs_nnheap_top(nnheap)->is_grid){
    lbs_mov_node_t * temp_node = (lbs_mov_node_t *)(lbs_nnheap_top(nnheap)->node->queue.next);
    lbs_mov_node_t * head_node = (lbs_mov_node_t *)(lbs_nnheap_top(nnheap)->node);
    int node_cell_id = lbs_nnheap_top(nnheap)->cell_id;
    //不用lbs_nnheap_top了，直接pop掉， 不然后面insert就乱了
    lbs_nnheap_pop(nnheap);
    while(temp_node!=head_node){
      double dis = lbs_distance(temp_node->lon, temp_node->lat, lon, lat);
      ret = lbs_nnheap_insert(nnheap, temp_node, curr_cell_id, 0, dis);
      temp_node = (lbs_mov_node_t *)(temp_node->queue.next);
    }//end while(temp_node)

    int temp_row = 0;
    int temp_col = 0;
    int temp_cellid = 0;
    lbs_grid_cell_row_col(&lbs_grid, node_cell_id, &temp_row, &temp_col);


    //temp_cellid = lbs_grid_cell_id(&lbs_grid, temp_row-1, temp_col-1);
    double dis = 0.0;
    int row_offset[] = {-1, 0, 1};
    int col_offset[] = {-1, 0, 1};

    for (int r = 0; r < 3; r++){
      for (int c = 0; c < 3; c++){
        //if (r_diff != 0 && c_diff != 0)
        temp_cellid = lbs_grid_cell_id(&lbs_grid, temp_row + row_offset[r], temp_col + col_offset[c]);
        if (!lbs_bitmap_isset(bitmap, temp_cellid)) {
          dis = lbs_cell_min_distance(&lbs_grid, lon, lat, temp_cellid);
          lbs_nnheap_insert(nnheap, &(lbs_grid.cell[temp_cellid].dammy_node), temp_cellid, 1, dis);
          lbs_bitmap_setbit(bitmap, temp_cellid);
        }//end if
      }
    }

  }//end while(lbs_nnheap_top(nnheap)->is_grid)

  // 不用管别人怎么叫的，直接insert head，这样多个nn都可以实现
  lbs_res_node_t * deepcopy_mov_node = (lbs_res_node_t *) malloc(sizeof(lbs_res_node_t));
  memcpy(deepcopy_mov_node, lbs_nnheap_top(nnheap)->node, sizeof(lbs_res_node_t));
  lbs_queue_insert_head(&(out->queue), &(deepcopy_mov_node->queue));

  return 0;
}