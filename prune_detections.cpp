#include "prune_detections.h"

// Union-Find (disjoint set) helpers
int uf_parent[MAX_ALL_DETECTIONS];

void uf_init(int n) {
  for (int i = 0; i < n; ++i) uf_parent[i] = i;
}
int uf_find(int a) {
  int p = a;
  while (uf_parent[p] != p) p = uf_parent[p];
  // path compression
  int cur = a;
  while (uf_parent[cur] != p) {
    int next = uf_parent[cur];
    uf_parent[cur] = p;
    cur = next;
  }
  return p;
}
void uf_union(int a, int b) {
  int ra = uf_find(a);
  int rb = uf_find(b);
  if (ra == rb) return;
  uf_parent[rb] = ra;
}

// collect detections from template_scores, cluster, and pick highest-score per cluster
// Inputs:
//   template_scores[] : array of TemplateScore already filled by zncc()
// Outputs:
//   out_labels, out_x, out_y, out_score are arrays sized MAX_ALL_DETECTIONS which will be filled
// Returns: number of final (unique) detections
// int cluster_and_select_max(TemplateScore template_scores[],
//                            char out_labels[], uint8_t out_x[], uint8_t out_y[], float out_score[]) {
//   // 1) collect all raw detections into a flat array
//   Detection dets[MAX_ALL_DETECTIONS];
//   int det_count = 0;

//   for (int t = 0; t < TEMPLATE_COUNT; ++t) {
//     for (int k = 0; k < MAX_DETECTIONS_PER_LABEL; ++k) {
//       float s = template_scores[t].best_scores[k];
//       // keep only scored detections (use ZNCC_THRESHOLD; adjust if you use -1 sentinel)
//       if (s > ZNCC_THRESHOLD) {
//         if (det_count >= MAX_ALL_DETECTIONS) break;
//         Detection &d = dets[det_count];
//         d.label = template_scores[t].label;
//         d.x = template_scores[t].best_x[k];
//         d.y = template_scores[t].best_y[k];
//         d.score = s;
//         det_count++;
//       }
//     }
//   }

//   if (det_count == 0) return 0;

//   // 2) init union-find
//   uf_init(det_count);

//   // 3) build adjacency by distance threshold and union
//   for (int i = 0; i < det_count; ++i) {
//     for (int j = i + 1; j < det_count; ++j) {
//       int dx = (int)dets[i].x - (int)dets[j].x;
//       int dy = (int)dets[i].y - (int)dets[j].y;
//       int dist2 = dx * dx + dy * dy;
//       if (dist2 <= CLUSTER_DISTANCE_SQ) {
//         uf_union(i, j);
//       }
//     }
//   }

//   // 4) For each cluster (component) choose the detection with max score
//   // Map representative -> index of best detection so far
//   // The root of each cluster is compared with all child elements.
//   // At root index of a cluster in best_idx_for_rep, the value is the detection index with highest score. It is -1 everywhere else.
//   // e.g. uf_parent = [0, 0, 0, 3, 3, 5, ...], values at indices 1-2 have root at 0, value at index 4 has root of 3, value at index 5 is a root itself.
//   // best_idx_for_rep = [1, -1, -1, 3, -1, 5, ...], index 1 has best score to represent cluster with a root index of 0, index 3 itself has the best score and is also a root, same with index 5
//   int best_idx_for_rep[MAX_ALL_DETECTIONS];
//   for (int i = 0; i < det_count; ++i) best_idx_for_rep[i] = -1;

//   for (int i = 0; i < det_count; ++i) {
//     int r = uf_find(i);
//     int cur_best = best_idx_for_rep[r];
//     if (cur_best == -1 || dets[i].score > dets[cur_best].score) {
//       best_idx_for_rep[r] = i;
//     }
//   }

//   // 5) Collect unique best detections
//   int out_count = 0;
//   for (int i = 0; i < det_count; ++i) {
//     if (best_idx_for_rep[i] != -1) {
//       int root = uf_find(i);
//       // the root of i must be i
//       if (root == i) {
//         Detection &b = dets[best_idx_for_rep[i]];
//         out_labels[out_count] = b.label;
//         out_x[out_count] = b.x;
//         out_y[out_count] = b.y;
//         out_score[out_count] = b.score;
//         out_count++;
//       }
//     }
//   }

//   return out_count;
// }

int cluster_and_select_max(Detection detections[], int num_detections, Detection filtered_detections[]) {

  if (num_detections == 0) return 0;

  // 2) init union-find
  uf_init(num_detections);

  // 3) build adjacency by distance threshold and union
  for (int i = 0; i < num_detections; ++i) {
    for (int j = i + 1; j < num_detections; ++j) {
      #ifdef CLUSTER_SAME_LABELS_ONLY
      if(detections[i].label != detections[j].label) continue;
      #endif
      int dx = (int)detections[i].x - (int)detections[j].x;
      int dy = (int)detections[i].y - (int)detections[j].y;
      int dist2 = dx * dx + dy * dy;
      if (dist2 <= CLUSTER_DISTANCE_SQ) {
        uf_union(i, j);
      }
    }
  }

  #ifdef REMOVE_SMALL_CLUSTERS
  // filter out all clusters that have size < 2
  int cluster_size[MAX_ALL_DETECTIONS] = {0};

  for(int i = 0; i < num_detections; ++i){
    int r = uf_find(i);
    cluster_size[r]++;
  }
  #endif


  // 4) For each cluster (component) choose the detection with max score
  // Map representative -> index of best detection so far
  // The root of each cluster is compared with all child elements.
  // At root index of a cluster in best_idx_for_rep, the value is the detection index with highest score. It is -1 everywhere else.
  // e.g. uf_parent = [0, 0, 0, 3, 3, 5, ...], values at indices 1-2 have root at 0, value at index 4 has root of 3, value at index 5 is a root itself.
  // best_idx_for_rep = [1, -1, -1, 3, -1, 5, ...], index 1 has best score to represent cluster with a root index of 0, index 3 itself has the best score and is also a root, same with index 5
  int best_idx_for_rep[MAX_ALL_DETECTIONS];
  for (int i = 0; i < num_detections; ++i) best_idx_for_rep[i] = -1;

  for (int i = 0; i < num_detections; ++i) {
    int r = uf_find(i);
    #ifdef REMOVE_SMALL_CLUSTERS
    if(cluster_size[r] < MIN_DETECTIONS_PER_CLUSTER) continue;
    #endif
    int cur_best = best_idx_for_rep[r];
    if (cur_best == -1) {
      best_idx_for_rep[r] = i;
      continue;
    }
    #ifdef OVERRIDE_DETECTIONS
    // if current label is 'T' and current best is '1', choose 'T' as the new best choice
    // same with '8' and '6', '8' and '3'
    if((detections[i].label == 'T' && detections[cur_best].label == '1') ||
      (detections[i].label == '8' && detections[cur_best].label == '6') ||
      (detections[i].label == '8' && detections[cur_best].label == '3')){
      best_idx_for_rep[r] = i;
      continue;
    }
    // if '1' has higher score than 'T', 'T' still remains the best choice
    // if '6' has higher score than '8', '8' still remains the best choice
    // if '3' has higher score than '8', '8' still remains the best choice
    if((detections[i].label == '1' && detections[cur_best].label == 'T') ||
      (detections[i].label == '6' && detections[cur_best].label == '8') ||
      (detections[i].label == '3' && detections[cur_best].label == '8')){
        continue;
    }
    #endif

    #ifdef PRIORITIZE_LARGE_LABELS
    uint8_t i_w = templates[label_to_idx(detections[i].label)].w;
    uint8_t i_h = templates[label_to_idx(detections[i].label)].h;
    uint8_t cur_best_w = templates[label_to_idx(detections[cur_best].label)].w;
    uint8_t cur_best_h = templates[label_to_idx(detections[cur_best].label)].h;
    if(i_w < cur_best_w && i_h < cur_best_h) continue;
    #endif

    if(detections[i].score > detections[cur_best].score){
      best_idx_for_rep[r] = i;
    }
  }

  // 5) Collect unique best detections
  int out_count = 0;
  for (int i = 0; i < num_detections; ++i) {
    if (uf_find(i) == i && best_idx_for_rep[i] != -1) {
      int best = best_idx_for_rep[i];
      filtered_detections[out_count++] = detections[best];
    }
  }

  return out_count;
}

// int cluster_and_select_max(Detection detections[], int num_detections, Detection filtered_detections[]) {

//   if (num_detections == 0) return 0;

//   // initialize map for CCL as all zeroes
//   uint8_t map[ROI_SIZE] = {0};
//   int* left, top_left, top;

//   int cluster_map[num_detections];
//   float cluster_scores[MAX_CLUSTERS] = {0};
//   int best_cluster_idx[MAX_CLUSTERS];
//   int num_filtered_detections = 0;
//   int current_cluster_idx = 1;

//   // first CCL pass
//   for(int i = 0; i < num_detections; i++){
//     uint8_t x = detections[i].x;
//     uint8_t y = detections[i].y;
//     uint8_t *addr = DIAGONAL(&map, detections[i].x, detections[i].y);
//     if(x == 0){
//       left = addr;
//       top_left = addr;
//     }
//     else if(y == 0){
//       top_left = addr;
//       top = addr;
//     }
//     else{
//       left = LEFT(addr);
//       top_left = TOP_LEFT(addr);
//       top = TOP(addr);
//     }
    
//     if(*left == 0 && *top_left == 0 && *top == 0){
//       *addr = current_cluster_idx;
//       current_cluster_idx++;
//     }
//     else{
//       int min_cluster_idx = min();
//     }
//   }

//   for(int i = 0; i < num_detections; i++){
//     int cluster_idx = cluster_map[i];
//     if(detections[i].score > cluster_scores[cluster_idx]){
//       cluster_scores[cluster_idx] = detections[i].score;
//       best_cluster_idx[cluster_idx] = i;
//     }
//   }

//   for(int i = 0; i < MAX_CLUSTERS; i++){
//     filtered_detections[i] = detections[best_cluster_idx[i]];
//     num_filtered_detections++;
//   }

//   return num_filtered_detections;
// }

void sort_detections_by_x(Detection *fd, int num_filtered_detections) {
  for (int i = 0; i < num_filtered_detections - 1; ++i) {
    int min_idx = i;
    for (int j = i + 1; j < num_filtered_detections; ++j) {
      if (fd[j].x < fd[min_idx].x) {
        min_idx = j;
      }
    }

    if (min_idx != i) {
      // swap x
      uint8_t tx = fd[i].x;
      fd[i].x = fd[min_idx].x;
      fd[min_idx].x = tx;

      // swap y
      uint8_t ty = fd[i].y;
      fd[i].y = fd[min_idx].y;
      fd[min_idx].y = ty;

      // swap label
      char tl = fd[i].label;
      fd[i].label = fd[min_idx].label;
      fd[min_idx].label = tl;

      // swap score
      float ts = fd[i].score;
      fd[i].score = fd[min_idx].score;
      fd[min_idx].score = ts;
    }
  }
}