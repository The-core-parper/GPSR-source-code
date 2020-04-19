/* -*- Mode:C++; c-basic-offset: 2; tab-width:2; indent-tabs-width:t -*- 
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

/* gpsr_neighbor.h : the head file for the definition of the neighbor 
 *                   list for each node in the network
 * Note: Due to the network nature, the neighbor list of different nodes 
 *       which are neighbors from a view of the transimission radio range
 *       may not consistent, which means for a pair of nodes, one may know
 *       the other is a neighbor, but the other one does not
 */


#ifndef GPSR_NEIGHBOR_H_
#define GPSR_NEIGHBOR_H_

#include "gpsr_packet.h"

#define DEFAULT_GPSR_TIMEOUT   200.0   
        //the default time out period is 200.0 sec
        //If the hello messge was not received during this period
        //the entry in the neighbor list may be deleted

#define INIFINITE_DISTANCE   1000000000.0 //bigger than radio range is enough

/* The structure used for neighbor entry
 */
struct gpsr_neighbor {
  nsaddr_t id_; 
  double x_;    //the geo info
  double y_; 
  
  double ts_;     //the last time stamp of the hello msg from it
  struct gpsr_neighbor *next_;
  struct gpsr_neighbor *prev_;
};

class GPSRNeighbors {
private:
  //the neighbors list
  struct gpsr_neighbor *head_;
  struct gpsr_neighbor *tail_;
  int nbSize_; //number of neighbors

  nsaddr_t my_id_; //my id 
  double my_x_;    //my geo info
  double my_y_;

  //find the entry in neighbor list according to the provided id
  struct gpsr_neighbor *getnb(nsaddr_t);

  //delete the entry in neighbors list according to the provided id
  void delnb(nsaddr_t); 

  //delete the entry directly
  void delnb(struct gpsr_neighbor *);

  //delete all the entries time out
  void delalltimeout();

  //for a given neighbor list (such as planarized neighbors), return size
  int num_of_neighbors(struct gpsr_neighbor*);
  
  /* functions used for perimeter routing calculation */
  struct gpsr_neighbor *gg_planarize();  //GG planarize
  struct gpsr_neighbor *rng_planarize(); //RNG planarize
  void free_neighbors(struct gpsr_neighbor*); //free the given neighbor list

  double angle(double, double, double, double);
                                        //absolute angle of the given line
                                        //relative angle calculation based
  
  int intersect(nsaddr_t, double, double, double, double);
  //check the 2 lines are intersected locally
  
public:
  GPSRNeighbors();
  ~GPSRNeighbors();

  //using to update location information of myself
  void myinfo(nsaddr_t, double, double);
  //return the number of neighbors
  int nbsize();

  //calculate the distance between (x1, y1, x2, y2)
  double getdis(double, double, double, double);

  //add a possible new neighbor
  void newNB(nsaddr_t, double, double); 

  //decide the next hop based on the destination(x, y)
  nsaddr_t gf_nexthop(double, double); //greedy forwarding mode
  nsaddr_t peri_nexthop(int, nsaddr_t, 	double, double, 
			double, double);//perimeter mode
                                       

  //dump the neighbor list 
  void dump();
};

#endif 
