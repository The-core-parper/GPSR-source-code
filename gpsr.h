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

/* gpsr.h : The head file for the GPSR routing agent, defining the 
 *          routing agent, methods (behaviors) of the routing 
 *          
 * Note: the routing table (local neighborhood) information is kept 
 *       in another class gpsr_neighbor which is defined in 
 *       gpsr_neighbor{.h, .cc}. So the planarizing and next hop deciding
 *       is made there, not in this file
 *
 */

#ifndef GPSR_ROUTING_H_
#define GPSR_ROUTING_H_

#include "config.h"
#include "agent.h"
#include "ip.h"
#include "address.h"
//#include "scheduler.h"
#include "timer-handler.h"
#include "mobilenode.h"
#include "tools/random.h"
#include "packet.h"
#include "trace.h"
#include "classifier-port.h"
#include "cmu-trace.h"

#include "gpsr_packet.h"
#include "gpsr_neighbor.h"
#include "gpsr_sinklist.h"

class GPSRAgent;

/*
 * Hello timer which is used to fire the hello message periodically
 */
class GPSRHelloTimer : public TimerHandler {
public:
  GPSRHelloTimer(GPSRAgent *a) : TimerHandler() {a_=a;}
protected:
  virtual void expire(Event *e);
  GPSRAgent *a_;
};

/*
 * The Query Timer which is used by the data sink to fire the 
 * data query. It is not a part of the design of the GPSR routing.
 * Since the information of the data sink mostly is not able to be 
 * obtained directly (mostly, by DHT: distributed hash table), I 
 * just let the data sink to trigger the routing, like a common 
 * On-Demond routing.
 * 
 */
class GPSRQueryTimer : public TimerHandler {
public:
  GPSRQueryTimer(GPSRAgent *a) : TimerHandler() {a_=a;}
protected:
  virtual void expire(Event *e);
  GPSRAgent *a_;
};

class GPSRAgent : public Agent {
private:

  friend class GPSRHelloTimer;
  friend class GPSRQueryTimer;
  
  MobileNode *node_;             //the attached mobile node
  PortClassifier *port_dmux_;    //for the higher layer app de-multiplexing
  
  nsaddr_t my_id_;               //node id (address), which is NOT necessary
  double my_x_;                  //node location info, fatal info
  double my_y_;                  //     obtained from the attached node

  Sinks *sink_list_;      //for multiple sinks

  GPSRNeighbors *nblist_; //neighbor list: routing table implemenation 
                          //               and planarizing implementation

  int recv_counter_;           
  u_int8_t query_counter_;

  GPSRHelloTimer hello_timer_;
  GPSRQueryTimer query_timer_;

  int planar_type_; //1=GG planarize, 0=RNG planarize

  double hello_period_;
  double query_period_;

  void turnon();              //set to be alive
  void turnoff();             //set to be dead
  void startSink();          
  void startSink(double);

  void GetLocation(double*, double*); //called at initial phase
  virtual void getLoc();

  void hellomsg();
  void query(nsaddr_t);

  void recvHello(Packet*);
  void recvQuery(Packet*);
  
  void sinkRecv(Packet*);
  void forwardData(Packet*);

  RNG randSend_;

  /**
   * The below variables and functions are used for 
   * localization algorithms
   */
  double localized_x_; 
  double localized_y_;
  void dvhop();

protected:
  Trace *tracetarget;              //for routing agent special trace
  void trace(char *fmt,...);       //   Not necessary 

  void hellotout();                //called by timer::expire(Event*)
  void querytout();

public:
  GPSRAgent();

  int command(int, const char*const*);
  void recv(Packet*, Handler*);         //inherited virtual function

};

#endif
