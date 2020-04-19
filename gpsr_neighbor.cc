/* -*- Mode:C++; c-basic-offset: 2; tab-width:2, indent-tabs-width:t -*- 
 * Copyright (C) 2005 State University of New York, at Binghamton
 * All rights reserved.
 *
 * NOTICE: This software is provided "as is", without any warranty,
 * including any implied warranty for merchantability or fitness for a
 * particular purpose.  Under no circumstances shall SUNY Binghamton
 * or its faculty, staff, students or agents be liable for any use of,
 * misuse of, or inability to use this software, including incidental
 * and consequential damages.

 * License is hereby given to use, modify, and redistribute this
 * software, in whole or in part, for any commercial or non-commercial
 * purpose, provided that the user agrees to the terms of this
 * copyright notice, including disclaimer of warranty, and provided
 * that this copyright notice, including disclaimer of warranty, is
 * preserved in the source code and documentation of anything derived
 * from this software.  Any redistributor of this software or anything
 * derived from this software assumes responsibility for ensuring that
 * any parties to whom such a redistribution is made are fully aware of
 * the terms of this license and disclaimer.
 *
 * Author: Ke Liu, CS Dept., State University of New York, at Binghamton 
 * October, 2005
 *
 * GPSR code for NS2 version 2.26 or later
 * Note: this implementation of GPSR is different from its original 
 *       version wich implemented by Brad Karp, Harvard Univ. 1999
 *       It is not guaranteed precise implementation of the GPSR design
 */

#include "gpsr_neighbor.h"

#define PI 3.141593

#define MAX(a, b) (a>=b?a:b)
#define MIN(a, b) (a>=b?b:a)

GPSRNeighbors::GPSRNeighbors(){
  my_id_ = -1;
  my_x_ = 0.0;
  my_y_ = 0.0;

  head_ = tail_ = NULL;
  nbSize_ = 0;

}

GPSRNeighbors::~GPSRNeighbors(){
  struct gpsr_neighbor *temp = head_;
  while(temp){
    temp = temp->next_;
    free(head_);
    head_ = temp;
  }
}

double 
GPSRNeighbors::getdis(double ax, double ay, double bx, double by){
  double tempx = ax - bx;
  double tempy = ay - by;

  tempx = tempx * tempx;
  tempy = tempy * tempy;

  double result = sqrt(tempx + tempy);
  return result;
}

int
GPSRNeighbors::nbsize(){
  return nbSize_;
}

void
GPSRNeighbors::myinfo(nsaddr_t mid, double mx, double my){
  my_id_ = mid;
  my_x_ = mx;
  my_y_ = my;
}

struct gpsr_neighbor*
GPSRNeighbors::getnb(nsaddr_t nid){
  struct gpsr_neighbor *temp = head_;
  while(temp){
    if(temp->id_ == nid){
      if((GPSR_CURRENT - temp->ts_) < DEFAULT_GPSR_TIMEOUT)
	return temp;
      else {
	delnb(temp); //if this entry expire, delete it and return NULL
	return NULL;
      }
      return temp;
    }
    temp = temp->next_;
  }
  return NULL;
}


void 
GPSRNeighbors::newNB(nsaddr_t nid, double nx, double ny){
  struct gpsr_neighbor *temp = getnb(nid);

  if(temp==NULL){ //it is a new neighbor
    temp=(struct gpsr_neighbor*)malloc(sizeof(struct gpsr_neighbor));
    temp->id_ = nid;
    temp->x_ = nx;
    temp->y_ = ny;
    temp->ts_ = GPSR_CURRENT;
    temp->next_ = temp->prev_ = NULL;

    if(tail_ == NULL){ //the list now is empty
      head_ = tail_  = temp;
      nbSize_ = 1;
    }
    else { //now the neighbors list is not empty
      tail_->next_ = temp;
      temp->prev_ = tail_;
      tail_ = temp;
      nbSize_++;
    }
  }
  else { //it is a already known neighbor
    temp->ts_ = GPSR_CURRENT;
    temp->x_ = nx; //the updating of location is allowed
    temp->y_ = ny;
  }
}

void
GPSRNeighbors::delnb(nsaddr_t nid){
  struct gpsr_neighbor *temp = getnb(nid);
  if(temp==NULL) return;
  else delnb(temp);
}

void
GPSRNeighbors::delnb(struct gpsr_neighbor *nb){
  struct gpsr_neighbor *preffix = nb->prev_;

  if(preffix == NULL){
    head_ = nb->next_;
    nb->next_ = NULL;

    if(head_ == NULL) 
      tail_ = NULL;
    else head_->prev_ = NULL;

    free(nb);
  }
  else {
    preffix->next_ = nb->next_;
    nb->prev_ = NULL;
    if(preffix->next_ == NULL)
      tail_ = preffix;
    else (preffix->next_)->prev_ = preffix;
    free(nb);
  }
  
  nbSize_--;
  
}

void
GPSRNeighbors::delalltimeout(){
  struct gpsr_neighbor *temp = head_;
  struct gpsr_neighbor *dd;
  while(temp){
    if((GPSR_CURRENT - temp->ts_) >= DEFAULT_GPSR_TIMEOUT){
      dd = temp;
      temp = temp->next_;
      delnb(dd);
    }
    else temp = temp->next_;
  }
  
}

nsaddr_t
GPSRNeighbors::gf_nexthop(double dx, double dy){
  struct gpsr_neighbor *temp = head_;
  //initializing the minimal distance as my distance to sink
  double mindis =getdis(my_x_, my_y_, dx, dy); 
  nsaddr_t nexthop = -1; //the nexthop result 

  while(temp){
    double tempdis = getdis(temp->x_, temp->y_, dx, dy);
    if(tempdis < mindis){
      mindis = tempdis;
      nexthop = temp->id_;
    }
    temp = temp->next_;
  }

  return nexthop;
}


struct gpsr_neighbor *
GPSRNeighbors::gg_planarize(){
    struct gpsr_neighbor *temp, *result, *index;
    index = head_;
    result = NULL;
    
    while(index){
      double midpx = my_x_ + (index->x_ - my_x_)/2.0;
      double midpy = my_y_ + (index->y_ - my_y_)/2.0;
      double mdis = getdis(my_x_, my_y_, midpx, midpy);
      
      temp = head_;
      while(temp){
	if(temp->id_ != index->id_){
	  double tempdis = getdis(midpx, midpy, temp->x_, temp->y_);
	  if(tempdis < mdis) break;
	}
	temp=temp->next_;
      }
      
      if(temp==NULL){
	temp = (struct gpsr_neighbor*)malloc(sizeof(struct gpsr_neighbor));
	temp->id_ = index->id_;
	temp->x_ = index->x_;
	temp->y_ = index->y_;
	temp->next_ = result;
	temp->prev_ = NULL;
	if(result) result->prev_ = temp;
	result = temp;
      }
      
      index=index->next_;
    }
  
    return result;

}

struct gpsr_neighbor *
GPSRNeighbors::rng_planarize(){
    struct gpsr_neighbor *temp, *result, *index;
    index = head_;
    result = NULL;
    
    while(index){
      double mdis = getdis(my_x_, my_y_, index->x_, index->y_);
      
      temp = head_;
      while(temp){
	if(temp->id_ != index->id_){
	  double tempdis1 = getdis(my_x_, my_y_, temp->x_, temp->y_);
	  double tempdis2 = getdis(index->x_, index->y_, temp->x_, temp->y_);
	  if(tempdis1 < mdis && tempdis2 < mdis) break;
	}
	temp=temp->next_;
      }
      
      if(temp==NULL){
	temp = (struct gpsr_neighbor*)malloc(sizeof(struct gpsr_neighbor));
	temp->id_ = index->id_;
	temp->x_ = index->x_;
	temp->y_ = index->y_;
	temp->next_ = result;
	temp->prev_ = NULL;
	if(result) result->prev_ = temp;
	result = temp;
      }
      
      index=index->next_;
    }
  
    return result;

}


double
GPSRNeighbors::angle(double x1, double y1, double x2, double y2){
  double line_len = sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));

  double sin_theta, cos_theta;
  double theta;

  if(line_len == 0.0){
    printf("2 nodes are the same\n");
    return -1.0;
  }

  sin_theta = (y2-y1)/line_len;
  cos_theta = (x2-x1)/line_len;

  theta = acos(cos_theta);
  
  if(sin_theta<0){
    theta = 2*PI - theta;
  }

  return theta;
}


/* To check the line from me to theother, and the line from source
 * and destination is intersecting each other or not
 * Note: 2 line segments intersects each other if they have a common 
 *       point, BUT here, if the common point is the end point, 
 *       we don't count it.
 */
int
GPSRNeighbors::intersect(nsaddr_t theother, double sx, double sy,
			 double dx, double dy){
  //line 1 (x1,y1)--(x2,y2) is the segment
  //line 2 (x3,y3)--(x4,y4) is the xD 
  struct gpsr_neighbor *other = getnb(theother);

  if(other==NULL){
    printf("Wrong the other node\n");
    exit(1);
  }
  
  double x1 = my_x_; 
  double y1 = my_y_;
  double x2 = other->x_;
  double y2 = other->y_;
  double x3 = sx;
  double y3 = sy;
  double x4 = dx;
  double y4 = dy;

  double a1 = y2 - y1;
  double b1 = x1 - x2;
  double c1 = x2*y1 - x1*y2;

  double a2 = y4 - y3;
  double b2 = x3 - x4;
  double c2 = x4*y3 - x3*y4;

  double denom = a1*b2 - a2*b1;

  double x, y; //the result;

  if(denom == 0) {
    return 0; //parallel lines;
  }

  x = (b1*c2 - b2*c1)/denom;
  y = (a2*c1 - a1*c2)/denom;

  if(x > MIN(x1, x2) && x < MAX(x1, x2) &&
     x > MIN(x3, x4) && x < MAX(x3, x4))
    return 1;       
  else return 0;
}


int
GPSRNeighbors::num_of_neighbors(struct gpsr_neighbor *nblist){
  struct gpsr_neighbor *temp = nblist;
  int counter = 0;
  while(temp){
    counter++;
    temp = temp->next_;
  }
  return counter;
}

void
GPSRNeighbors::free_neighbors(struct gpsr_neighbor *nblist){
  struct gpsr_neighbor *temp, *head;
  head = nblist;
  while(head){
    temp = head;
    head = head->next_;
    free(temp);
  }
}

nsaddr_t
GPSRNeighbors::peri_nexthop(int type_, nsaddr_t last,
			    double sx, double sy,
			    double dx, double dy){
  struct gpsr_neighbor *planar_neighbors, *temp;
  double alpha, minangle;
  nsaddr_t nexthop=-1;
  
  if(type_){//GG planarizing
    planar_neighbors = gg_planarize();
  }else {//RNG planarizing
    planar_neighbors = rng_planarize();
  }

  if(last>-1){
    struct gpsr_neighbor *lastnb = getnb(last);
    if(lastnb == NULL) {
      printf("Wrong last nb %d->%d\n", last, my_id_);
      exit(1);
    }
    alpha = angle(my_x_, my_y_, lastnb->x_, lastnb->y_);
  }
  else 
    alpha = angle(my_x_, my_y_, dx, dy); 
  temp = planar_neighbors;
  
  while(temp){
    if(temp->id_ != last){
      double delta;
      delta = angle(my_x_, my_y_, temp->x_, temp->y_);
      delta = delta - alpha;
      if(delta < 0.0) {
	delta = 2*PI + delta;
      }
      
      if(delta < minangle){
	minangle = delta;
	nexthop = temp->id_;
      }
    }
    temp = temp->next_;
  }

  if(num_of_neighbors(planar_neighbors) > 1 &&
       intersect(nexthop, sx, sy, dx, dy)){
      free_neighbors(planar_neighbors);
      return peri_nexthop(type_, nexthop, sx, sy, dx, dy);
  }
  return nexthop;
}

void
GPSRNeighbors::dump(){
  delalltimeout();
  
  FILE *fp = fopen(NB_TRACE_FILE, "a+"); 

  struct gpsr_neighbor *temp = head_;
  fprintf(fp, "%d:\t", my_id_);
  while(temp){
    fprintf(fp, "%d ", temp->id_);
    temp = temp->next_;
  }
  fprintf(fp,"\n");
  fclose(fp);
}

