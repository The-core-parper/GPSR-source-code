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

/* gpsr_sinklist.cc : the definition file for the sink list class
 *
 */

#include "gpsr_sinklist.h"


Sinks::Sinks(){
  sinklist_ = NULL;
}

bool
Sinks::new_sink(nsaddr_t id, double x, double y, 
		nsaddr_t lasthop, int hops, int seqno){
  struct sink_entry *temp = sinklist_;
  while(temp){
    if(temp->id_ == id){
      if(temp->x_ == x && temp->y_ == y && 
	 ((seqno <= temp->seqno_) || (temp->hops_ <= hops)))
	return false;
      else {
	temp->x_ = x;
	temp->y_ = y;
	temp->lasthop_ = lasthop;
	temp->hops_ = hops;
	temp->seqno_ = seqno;
	return true;
      }
    }
    temp = temp->next_;
  }
  temp = (struct sink_entry*)malloc(sizeof(struct sink_entry));
  temp->id_ = id;
  temp->x_ = x;
  temp->y_ = y;
  temp->lasthop_ = lasthop;
  temp->hops_ = hops;
  temp->seqno_ = seqno;
  temp->next_ = sinklist_;
  sinklist_ = temp;
  return true;
}

bool
Sinks::remove_sink(nsaddr_t id){
  struct sink_entry *temp;
  struct sink_entry *p, *q;

  q = NULL;
  p = sinklist_;
  while(p){
    temp = p;
    if(temp->id_ == id){
      p = temp->next_;
      if(q){
	q->next_ = p;
      }
      else {
	sinklist_ = p;
      }
      free(temp);
      return true;
    }

    q = p;
    p = p->next_;
  }
  return false;
}

void
Sinks::getLocbyID(nsaddr_t id, double &x, double &y, int &hops){
  struct sink_entry *temp = sinklist_;
  while(temp){
    if(temp->id_ == id){
      x = temp->x_;
      y = temp->y_;
      hops = temp->hops_;
      return;
    }
    temp = temp->next_;
  }
  x = -1.0;
  y = -1.0;
}

void 
Sinks::dump(){
  struct sink_entry *temp = sinklist_;
  FILE *fp = fopen("sinklist.tr", "a+");
  while(temp){
    fprintf(fp, "%d\t%f\t%f\t%d\t%d\t%d\n", 
	    temp->id_, temp->x_, temp->y_, 
	    temp->lasthop_, temp->hops_, temp->seqno_);
    temp = temp->next_;
  }
  fclose(fp);
}
