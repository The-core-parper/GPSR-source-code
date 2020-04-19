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

/* gpsr.cc : the definition of the gpsr routing agent class
 *           
 */
#include "gpsr.h"

int hdr_gpsr::offset_;

static class GPSRHeaderClass : public PacketHeaderClass{
public:
  GPSRHeaderClass() : PacketHeaderClass("PacketHeader/GPSR",
					 sizeof(hdr_all_gpsr)){
    bind_offset(&hdr_gpsr::offset_);
  }
}class_gpsrhdr;

static class GPSRAgentClass : public TclClass {
public:
  GPSRAgentClass() : TclClass("Agent/GPSR"){}
  TclObject *create(int, const char*const*){
    return (new GPSRAgent());
  }
}class_gpsr;

void
GPSRHelloTimer::expire(Event *e){
  a_->hellotout();
}

void
GPSRQueryTimer::expire(Event *e){
  a_->querytout();
}

void
GPSRAgent::hellotout(){
  hellomsg();
  hello_timer_.resched(hello_period_);
}

void
GPSRAgent::startSink(){
  if(sink_list_->new_sink(my_id_, my_x_, my_y_, 
			  my_id_, 0, query_counter_))
    querytout();
}

void
GPSRAgent::startSink(double gp){
  query_period_ = gp;
  startSink();
}

void
GPSRAgent::querytout(){
  query(my_id_);
  query_counter_++;
  query_timer_.resched(query_period_);
}

void
GPSRAgent::getLoc(){
  GetLocation(&my_x_, &my_y_);
}

void
GPSRAgent::GetLocation(double *x, double *y){
  double pos_x_, pos_y_, pos_z_;
  node_->getLoc(&pos_x_, &pos_y_, &pos_z_);
  *x=pos_x_;
  *y=pos_y_;
}


GPSRAgent::GPSRAgent() : Agent(PT_GPSR), 
		     hello_timer_(this), query_timer_(this),
		     my_id_(-1), my_x_(0.0), my_y_(0.0),
		     recv_counter_(0), query_counter_(0),
		     query_period_(INFINITE_DELAY)
{
  bind("planar_type_", &planar_type_);  
  bind("hello_period_", &hello_period_);
  
  sink_list_ = new Sinks();
  nblist_ = new GPSRNeighbors();
  
  for(int i=0; i<5; i++)
    randSend_.reset_next_substream();
}

void
GPSRAgent::turnon(){
  getLoc();
  nblist_->myinfo(my_id_, my_x_, my_y_);
  hello_timer_.resched(randSend_.uniform(0.0, 0.5));
}

void
GPSRAgent::turnoff(){
  hello_timer_.resched(INFINITE_DELAY);
  query_timer_.resched(INFINITE_DELAY);
}

void 
GPSRAgent::hellomsg(){
  if(my_id_ < 0) return;

  Packet *p = allocpkt();
  struct hdr_cmn *cmh = HDR_CMN(p);
  struct hdr_ip *iph = HDR_IP(p);
  struct hdr_gpsr_hello *ghh = HDR_GPSR_HELLO(p);

  cmh->next_hop_ = IP_BROADCAST;
  cmh->last_hop_ = my_id_;
  cmh->addr_type_ = NS_AF_INET;
  cmh->ptype() = PT_GPSR;
  cmh->size() = IP_HDR_LEN + ghh->size();

  iph->daddr() = IP_BROADCAST;
  iph->saddr() = my_id_;
  iph->sport() = RT_PORT;
  iph->dport() = RT_PORT;
  iph->ttl_ = IP_DEF_TTL;

  ghh->type_ = GPSRTYPE_HELLO;
  ghh->x_ = (float)my_x_;
  ghh->y_ = (float)my_y_;

  send(p, 0);
}


void
GPSRAgent::query(nsaddr_t id){
  if(my_id_ < 0) return;

  Packet *p = allocpkt();

  struct hdr_cmn *cmh = HDR_CMN(p);
  struct hdr_ip *iph = HDR_IP(p);
  struct hdr_gpsr_query *gqh = HDR_GPSR_QUERY(p);

  cmh->next_hop_ = IP_BROADCAST;
  cmh->last_hop_ = my_id_;
  cmh->addr_type_ = NS_AF_INET;
  cmh->ptype() = PT_GPSR;
  cmh->size() = IP_HDR_LEN + gqh->size();
  
  iph->daddr() = IP_BROADCAST;
  iph->saddr() = id;
  iph->sport() = RT_PORT;
  iph->dport() = RT_PORT;
  iph->ttl_ = IP_DEF_TTL;

  gqh->type_ = GPSRTYPE_QUERY;
  double tempx, tempy;
  int hops; 
  sink_list_->getLocbyID(id, tempx, tempy, hops);
  if(tempx >= 0.0){
    gqh->x_ = (float)tempx;
    gqh->y_ = (float)tempy;
    gqh->hops_ = hops;
  }else {
    Packet::free(p);
    return;
  }
  gqh->ts_ = (float)GPSR_CURRENT;
  gqh->seqno_ = query_counter_;

  send(p, 0);
}

void
GPSRAgent::recvHello(Packet *p){
  struct hdr_cmn *cmh = HDR_CMN(p);
  struct hdr_gpsr_hello *ghh = HDR_GPSR_HELLO(p);

  nblist_->newNB(cmh->last_hop_, (double)ghh->x_, (double)ghh->y_);
  //  trace("%d recv Hello from %d", my_id_, cmh->last_hop_);
}

void
GPSRAgent::recvQuery(Packet *p){
  struct hdr_cmn *cmh = HDR_CMN(p);
  struct hdr_ip *iph = HDR_IP(p);
  struct hdr_gpsr_query *gqh = HDR_GPSR_QUERY(p);
  
  if(sink_list_->new_sink(iph->saddr(), gqh->x_, gqh->y_, 
			  cmh->last_hop_, 1+gqh->hops_, gqh->seqno_))
    query(iph->saddr());
  //  trace("%d recv Query from %d ", my_id_, iph->saddr());  
}

void
GPSRAgent::sinkRecv(Packet *p){
  FILE *fp = fopen(SINK_TRACE_FILE, "a+");
  struct hdr_cmn *cmh = HDR_CMN(p);
  struct hdr_ip *iph = HDR_IP(p);
  //  struct hdr_gpsr_data *gdh = HDR_GPSR_DATA(p);

  fprintf(fp, "%2.f\t%d\t%d\n", GPSR_CURRENT,
	  iph->saddr(), cmh->num_forwards());
  fclose(fp);
}
void
GPSRAgent::forwardData(Packet *p){
  struct hdr_cmn *cmh = HDR_CMN(p);
  struct hdr_ip *iph = HDR_IP(p);

  if(cmh->direction() == hdr_cmn::UP &&
     ((nsaddr_t)iph->daddr() == IP_BROADCAST ||
      iph->daddr() == my_id_)){
    sinkRecv(p);
    printf("receive\n");
    port_dmux_->recv(p, 0);
    return;
  }
  else {
    struct hdr_gpsr_data *gdh=HDR_GPSR_DATA(p);
    
    double dx = gdh->dx_;
    double dy = gdh->dy_;
    
    nsaddr_t nexthop;
    if(gdh->mode_ == GPSR_MODE_GF){
      nexthop = nblist_->gf_nexthop(dx, dy);
      
      if(nexthop == -1){
	nexthop = nblist_->peri_nexthop(planar_type_, -1,
					gdh->sx_, gdh->sy_,
					gdh->dx_, gdh->dy_);
	gdh->sx_ = my_x_;
	gdh->sy_ = my_y_;
	gdh->mode_ = GPSR_MODE_PERI;
      }
    }
    else {
      double sddis = nblist_->getdis(gdh->sx_, gdh->sy_, gdh->dx_, gdh->dy_);
      double mydis = nblist_->getdis(my_x_, my_y_, gdh->dx_, gdh->dy_);
      if(mydis < sddis){
	//switch back to greedy forwarding mode
	nexthop = nblist_->gf_nexthop(dx, dy);
	gdh->mode_ = GPSR_MODE_GF;
	
	if(nexthop == -1){
	  nexthop = 
	    nblist_->peri_nexthop(planar_type_, -1, 
				  gdh->sx_, gdh->sy_,
				  gdh->dx_, gdh->dy_);
	  gdh->sx_ = my_x_;
	  gdh->sy_ = my_y_;
	  gdh->mode_ = GPSR_MODE_PERI;
	}
      }
      else{ //still perimeter routing mode
	nexthop = 
	  nblist_->peri_nexthop(planar_type_, cmh->last_hop_, 
				gdh->sx_, gdh->sy_, gdh->dx_, gdh->dy_);
      }
    }

    cmh->direction() = hdr_cmn::DOWN;
    cmh->addr_type() = NS_AF_INET;
    cmh->last_hop_ = my_id_;
    cmh->next_hop_ = nexthop;
    send(p, 0);
  }
}



void
GPSRAgent::recv(Packet *p, Handler *h){
  struct hdr_cmn *cmh = HDR_CMN(p);
  struct hdr_ip *iph = HDR_IP(p);

  if(iph->saddr() == my_id_){//a packet generated by myself
    if(cmh->num_forwards() == 0){
      struct hdr_gpsr_data *gdh = HDR_GPSR_DATA(p);
      cmh->size() += IP_HDR_LEN + gdh->size();

      gdh->type_ = GPSRTYPE_DATA;
      gdh->mode_ = GPSR_MODE_GF;
      gdh->sx_ = (float)my_x_;
      gdh->sy_ = (float)my_y_;
      double tempx, tempy;
      int hops;
      sink_list_->getLocbyID(iph->daddr(), tempx, tempy, hops);
      if(tempx >= 0.0){
	gdh->dx_ = (float)tempx;
	gdh->dy_ = (float)tempy;
      }
      else {
	drop(p, "NoSink");
	return;
      }
      gdh->ts_ = (float)GPSR_CURRENT;
    }
    else if(cmh->num_forwards() > 0){ //routing loop
      if(cmh->ptype() != PT_GPSR)
	drop(p, DROP_RTR_ROUTE_LOOP);
      else Packet::free(p);
      return;
    }
  }

  if(cmh->ptype() == PT_GPSR){
    struct hdr_gpsr *gh = HDR_GPSR(p);
    switch(gh->type_){
    case GPSRTYPE_HELLO:
      recvHello(p);
      break;
    case GPSRTYPE_QUERY:
      recvQuery(p);
      break;
    default:
      printf("Error with gf packet type.\n");
      exit(1);
    }
  } else {
    iph->ttl_--;
    if(iph->ttl_ == 0){
      drop(p, DROP_RTR_TTL);
      return;
    }
    forwardData(p);
  }

}

void 
GPSRAgent::trace(char *fmt, ...){
  va_list ap;
  if(!tracetarget)
    return;
  va_start(ap, fmt);
  vsprintf(tracetarget->pt_->buffer(), fmt, ap);
  tracetarget->pt_->dump();
  va_end(ap);
}

int
GPSRAgent::command(int argc, const char*const* argv){
  if(argc==2){
    if(strcasecmp(argv[1], "getloc")==0){
      getLoc();
      return TCL_OK;
    }

    if(strcasecmp(argv[1], "turnon")==0){
      turnon();
      return TCL_OK;
    }
    
    if(strcasecmp(argv[1], "turnoff")==0){
      turnoff();
      return TCL_OK;
    }

    if(strcasecmp(argv[1], "startSink")==0){
      startSink();
      return TCL_OK;
    }

    if(strcasecmp(argv[1], "neighborlist")==0){
      nblist_->dump();
      return TCL_OK;
    }
    if(strcasecmp(argv[1], "sinklist")==0){
      sink_list_->dump();
      return TCL_OK;
    }
  }


  if(argc==3){
    if(strcasecmp(argv[1], "startSink")==0){
      startSink(atof(argv[2]));
      return TCL_OK;
    }

    if(strcasecmp(argv[1], "addr")==0){
      my_id_ = Address::instance().str2addr(argv[2]);
      return TCL_OK;
    } 

    TclObject *obj;
    if ((obj = TclObject::lookup (argv[2])) == 0){
      fprintf (stderr, "%s: %s lookup of %s failed\n", __FILE__, argv[1],
	       argv[2]);
      return (TCL_ERROR);
    }
    if (strcasecmp (argv[1], "node") == 0) {
      node_ = (MobileNode*) obj;
      return (TCL_OK);
    }
    else if (strcasecmp (argv[1], "port-dmux") == 0) {
      port_dmux_ = (PortClassifier*) obj; //(NsObject *) obj;
      return (TCL_OK);
    } else if(strcasecmp(argv[1], "tracetarget")==0){
      tracetarget = (Trace *)obj;
      return TCL_OK;
    }

  }// if argc == 3

  return (Agent::command(argc, argv));
}
